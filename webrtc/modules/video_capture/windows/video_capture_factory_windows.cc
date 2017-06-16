/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/base/refcount.h"
#include "webrtc/base/scoped_ref_ptr.h"
#ifdef WINRT
#include "webrtc/modules/video_capture/windows/video_capture_winrt.h"
#else // WINRT
#include "webrtc/modules/video_capture/windows/video_capture_ds.h"
#include "webrtc/modules/video_capture/windows/video_capture_mf.h"
#endif // WINRT

namespace webrtc {
namespace videocapturemodule {

// static
VideoCaptureModule::DeviceInfo* VideoCaptureImpl::CreateDeviceInfo() {
  // TODO(tommi): Use the Media Foundation version on Vista and up.
#ifdef WINRT
	return DeviceInfoWinRT::Create().release();
#else
	// TODO(tommi): Use the Media Foundation version on Vista and up.
	return DeviceInfoDS::Create();
#endif
}

rtc::scoped_refptr<VideoCaptureModule> VideoCaptureImpl::Create(
    const char* device_id) {
  if (device_id == nullptr)
    return nullptr;

#ifdef WINRT
	rtc::scoped_refptr<VideoCaptureWinRT> capture(
		new rtc::RefCountedObject<VideoCaptureWinRT>());
	if (capture->Init(device_id) != 0) {
		return nullptr;
	}
	return capture;
#else
	// TODO(tommi): Use Media Foundation implementation for Vista and up.
	rtc::scoped_refptr<VideoCaptureDS> capture(
		new rtc::RefCountedObject<VideoCaptureDS>());
	if (capture->Init(device_id) != 0) {
		return nullptr;
	}

	return capture;
#endif
}

}  // namespace videocapturemodule
}  // namespace webrtc
