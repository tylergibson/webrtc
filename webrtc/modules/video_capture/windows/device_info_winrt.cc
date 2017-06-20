/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#include "webrtc/modules/video_capture/windows/device_info_winrt.h"

#include <windows.media.h>

#include <string>

#include "webrtc/system_wrappers/include/logging.h"
#include "webrtc/base/Win32.h"
#include "webrtc/common_video/video_common_winrt.h"

using Windows::Devices::Enumeration::DeviceClass;
using Windows::Devices::Enumeration::DeviceInformation;
using Windows::Devices::Enumeration::DeviceInformationCollection;
using Windows::Media::Capture::MediaCapture;
using Windows::Media::Capture::MediaCaptureInitializationSettings;
using Windows::Media::Capture::MediaStreamType;
using Windows::Media::MediaProperties::IVideoEncodingProperties;
using Windows::Media::MediaProperties::MediaEncodingSubtypes;
using Windows::UI::Core::CoreDispatcher;
using Windows::UI::Core::CoreDispatcherPriority;
using Windows::UI::Core::DispatchedHandler;

namespace webrtc {
namespace videocapturemodule {

MediaCaptureDevicesWinRT::MediaCaptureDevicesWinRT() :
  critical_section_(CriticalSectionWrapper::CreateCriticalSection()) {
}

MediaCaptureDevicesWinRT::~MediaCaptureDevicesWinRT() {
  if (critical_section_) {
    delete critical_section_;
  }
}

MediaCaptureDevicesWinRT^ MediaCaptureDevicesWinRT::Instance() {
  static MediaCaptureDevicesWinRT^ instance =
    ref new MediaCaptureDevicesWinRT();
  return instance;
}

Platform::Agile<MediaCapture>
MediaCaptureDevicesWinRT::GetMediaCapture(Platform::String^ device_id) {
  CriticalSectionScoped cs(critical_section_);

  Platform::Agile<MediaCapture> media_capture_agile(ref new MediaCapture());

  Concurrency::task<void> initialize_async_task;
  auto handler = ref new DispatchedHandler(
    [this, &initialize_async_task, media_capture_agile, device_id]() {
    auto settings = ref new MediaCaptureInitializationSettings();
    settings->VideoDeviceId = device_id;

    // If Communications media category is configured, the
    // GetAvailableMediaStreamProperties will report only H264 frame format
    // for some devices (ex: Surface Pro 3). Since at the moment, WebRTC does
    // not support receiving H264 frames from capturer, the Communications
    // category is not configured.

    // settings->MediaCategory =
    //  Windows::Media::Capture::MediaCategory::Communications;
    auto initOp = media_capture_agile->InitializeAsync(settings);
    initialize_async_task = Concurrency::create_task(initOp).
      then([this, media_capture_agile](Concurrency::task<void> initTask) {
        try {
          initTask.get();
        } catch (Platform::Exception^ e) {
          LOG(LS_ERROR)
            << "Failed to initialize media capture device. "
            << rtc::ToUtf8(e->Message->Data());
        }
      });
  });

Windows::UI::Core::CoreDispatcher^ _windowDispatcher =	VideoCommonWinRT::GetCoreDispatcher();
  if (_windowDispatcher != nullptr) {
    auto dispatcher_action = _windowDispatcher->RunAsync(
      CoreDispatcherPriority::Normal, handler);
    Concurrency::create_task(dispatcher_action).wait();
  } else {
    handler->Invoke();
  }

  initialize_async_task.wait();

  // Cache the MediaCapture object so we don't recreate it later.
  media_capture_ = media_capture_agile;
  return media_capture_agile;
}

// static
std::unique_ptr<DeviceInfoWinRT> DeviceInfoWinRT::Create() {
	std::unique_ptr<DeviceInfoWinRT> winrt_info(new DeviceInfoWinRT());
  if (winrt_info->Init() != 0) {
    winrt_info.reset();
    LOG(LS_ERROR) << "Failed to initialize device info object.";
  }
  return winrt_info;
}

DeviceInfoWinRT::DeviceInfoWinRT() : DeviceInfoImpl() {
}

DeviceInfoWinRT::~DeviceInfoWinRT() {
}

uint32_t DeviceInfoWinRT::NumberOfDevices() {
  ReadLockScoped cs(_apiLock);
  return GetDeviceInfo(255, 0, 0, 0, 0, 0, 0);
}

int32_t DeviceInfoWinRT::GetDeviceName(uint32_t device_number,
                                       char* device_name_utf8,
                                       uint32_t device_name_utf8_length,
                                       char* device_unique_id_utf8,
                                       uint32_t device_unique_id_utf8_length,
                                       char* product_unique_id_utf8,
                                       uint32_t product_unique_id_utf8_length) {
  ReadLockScoped cs(_apiLock);
  const int32_t result = GetDeviceInfo(device_number,
                                       device_name_utf8,
                                       device_name_utf8_length,
                                       device_unique_id_utf8,
                                       device_unique_id_utf8_length,
                                       product_unique_id_utf8,
                                       product_unique_id_utf8_length);
  return result > (int32_t)device_number ? 0 : -1;
}

int32_t DeviceInfoWinRT::GetDeviceInfo(uint32_t device_number,
                                       char* device_name_utf8,
                                       uint32_t device_name_utf8_length,
                                       char* device_unique_id_utf8,
                                       uint32_t device_unique_id_utf8_length,
                                       char* product_unique_id_utf8,
                                       uint32_t product_unique_id_utf8_length) {
  int device_count = -1;
  Concurrency::create_task(
    DeviceInformation::FindAllAsync(DeviceClass::VideoCapture)).then(
      [this,
      device_number,
      device_name_utf8,
      device_name_utf8_length,
      device_unique_id_utf8,
      device_unique_id_utf8_length,
      product_unique_id_utf8,
      product_unique_id_utf8_length,
      &device_count]
        (Concurrency::task<DeviceInformationCollection^> find_task) {
    try {
      DeviceInformationCollection^ dev_info_collection = find_task.get();
      if (dev_info_collection == nullptr || dev_info_collection->Size == 0) {
        LOG_F(LS_ERROR) << "No video capture device found";
        return;
      }
      device_count = dev_info_collection->Size;
      for (unsigned int i = 0; i < dev_info_collection->Size; i++) {
        if (i != static_cast<int>(device_number))
          continue;  // Continue until the device number is found.
        auto dev_info = dev_info_collection->GetAt(i);
        Platform::String^ device_name = dev_info->Name;
        Platform::String^ device_unique_id = dev_info->Id;
        int convResult = 0;
        convResult = WideCharToMultiByte(CP_UTF8, 0,
          device_name->Data(), -1,
          device_name_utf8, device_name_utf8_length, NULL, NULL);
        if (convResult == 0) {
          LOG(LS_ERROR) << "Failed to convert device name to UTF8. " <<
            GetLastError();
        }
        convResult = WideCharToMultiByte(CP_UTF8, 0,
          device_unique_id->Data(), -1,
          device_unique_id_utf8, device_unique_id_utf8_length, NULL, NULL);
        if (convResult == 0) {
          LOG(LS_ERROR) << "Failed to convert device unique ID to UTF8. " <<
            GetLastError();
        }
        if (product_unique_id_utf8 != NULL)
          product_unique_id_utf8[0] = 0;
      }
    } catch (Platform::Exception^ e) {
      LOG(LS_ERROR) << "Failed to retrieve device info collection. " <<
        rtc::ToUtf8(e->Message->Data());
    }
  }).wait();

  return device_count;
}

int32_t DeviceInfoWinRT::DisplayCaptureSettingsDialogBox(
  const char* device_unique_id_utf8,
  const char* dialog_title_utf8,
  void* parent_window,
  uint32_t position_x,
  uint32_t position_y) {
  LOG_F(LS_ERROR) << "Not supported.";
  return -1;
}

int32_t DeviceInfoWinRT::CreateCapabilityMap(
  const char* device_unique_id_utf8) {
  _captureCapabilities.clear();

  const int32_t device_unique_id_utf8_length =
    (int32_t)strlen(device_unique_id_utf8);
  if (device_unique_id_utf8_length > kVideoCaptureUniqueNameLength) {
    LOG_F(LS_ERROR) << "Device name too long";
    return -1;
  }
  LOG(LS_INFO) << "CreateCapabilityMap called for device " <<
    device_unique_id_utf8;

  Concurrency::create_task(
    DeviceInformation::FindAllAsync(DeviceClass::VideoCapture)).then(
      [this, device_unique_id_utf8, device_unique_id_utf8_length]
        (Concurrency::task<DeviceInformationCollection^> find_task) {
    try {
      DeviceInformationCollection^ dev_info_collection = find_task.get();
      if (dev_info_collection == nullptr || dev_info_collection->Size == 0) {
        LOG_F(LS_ERROR) << "No video capture device found";
        return;
      }
      // Look for the device in the collection.
      DeviceInformation^ chosen_dev_info = nullptr;
      for (unsigned int i = 0; i < dev_info_collection->Size; i++) {
        auto dev_info = dev_info_collection->GetAt(i);
        if (rtc::ToUtf8(dev_info->Id->Data()) == device_unique_id_utf8) {
          chosen_dev_info = dev_info;
          break;
        }
      }

      // If we haven't found the device, return now.
      if (chosen_dev_info == nullptr)
        return;

      // Create a MediaCapture.
      auto media_capture =
        MediaCaptureDevicesWinRT::Instance()->GetMediaCapture(
          chosen_dev_info->Id);
      auto stream_properties = media_capture->VideoDeviceController->
        GetAvailableMediaStreamProperties(MediaStreamType::VideoRecord);
      for (unsigned int i = 0; i < stream_properties->Size; i++) {
        IVideoEncodingProperties^ prop =
          static_cast<IVideoEncodingProperties^>(stream_properties->GetAt(i));
        VideoCaptureCapability capability;
        capability.width = prop->Width;
        capability.height = prop->Height;
        capability.maxFPS =
          static_cast<int>(
            static_cast<float>(prop->FrameRate->Numerator) /
            static_cast<float>(prop->FrameRate->Denominator));
        if (_wcsicmp(prop->Subtype->Data(),
          MediaEncodingSubtypes::Yv12->Data()) == 0)
          capability.rawType = kVideoYV12;
        else if (_wcsicmp(prop->Subtype->Data(),
          MediaEncodingSubtypes::Yuy2->Data()) == 0)
          capability.rawType = kVideoYUY2;
        else if (_wcsicmp(prop->Subtype->Data(),
          MediaEncodingSubtypes::Iyuv->Data()) == 0)
          capability.rawType = kVideoIYUV;
        else if (_wcsicmp(prop->Subtype->Data(),
          MediaEncodingSubtypes::Rgb24->Data()) == 0)
          capability.rawType = kVideoRGB24;
        else if (_wcsicmp(prop->Subtype->Data(),
          MediaEncodingSubtypes::Rgb32->Data()) == 0)
          capability.rawType = kVideoARGB;
        else if (_wcsicmp(prop->Subtype->Data(),
          MediaEncodingSubtypes::Mjpg->Data()) == 0)
          capability.rawType = kVideoMJPEG;
        else if (_wcsicmp(prop->Subtype->Data(),
          MediaEncodingSubtypes::Nv12->Data()) == 0)
          capability.rawType = kVideoNV12;
        else
          capability.rawType = kVideoUnknown;
        _captureCapabilities.push_back(capability);
      }
    } catch (Platform::Exception^ e) {
      LOG(LS_ERROR) << "Failed to find media capture devices. " <<
        rtc::ToUtf8(e->Message->Data());
    }
  }).wait();

  return (int32_t)_captureCapabilities.size();
}

}  // namespace videocapturemodule
}  // namespace webrtc
