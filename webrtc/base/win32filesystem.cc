/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/base/win32filesystem.h"

#include "webrtc/base/win32.h"
#include <shellapi.h>
#include <shlobj.h>
#include <tchar.h>

#include <memory>

#include "webrtc/base/arraysize.h"
#include "webrtc/base/checks.h"
#include "webrtc/base/fileutils.h"
#include "webrtc/base/pathutils.h"
#include "webrtc/base/stream.h"
#include "webrtc/base/stringutils.h"

#if defined(WINRT)
#include <objbase.h>
#endif // defined(WINRT)

// In several places in this file, we test the integrity level of the process
// before calling GetLongPathName. We do this because calling GetLongPathName
// when running under protected mode IE (a low integrity process) can result in
// a virtualized path being returned, which is wrong if you only plan to read.
// TODO: Waiting to hear back from IE team on whether this is the
// best approach; IEIsProtectedModeProcess is another possible solution.

#if defined(WINRT)
#undef GetFileAttributes
DWORD WINAPI GetFileAttributes(LPCTSTR lpFileName) {
  WIN32_FILE_ATTRIBUTE_DATA attributes;
  BOOL ret = GetFileAttributesEx(lpFileName, GetFileExInfoStandard, &attributes);
  if (!ret) {
    return INVALID_FILE_ATTRIBUTES;
  }

  return attributes.dwFileAttributes;
}
#endif // defined(WINRT)

namespace rtc {

bool Win32Filesystem::CreateFolder(const Pathname &pathname) {
  if (pathname.pathname().empty() || !pathname.filename().empty())
    return false;

  std::wstring path16;
  if (!Utf8ToWindowsFilename(pathname.pathname(), &path16))
    return false;

  DWORD res = ::GetFileAttributes(path16.c_str());
  if (res != INVALID_FILE_ATTRIBUTES) {
    // Something exists at this location, check if it is a directory
    return ((res & FILE_ATTRIBUTE_DIRECTORY) != 0);
  } else if ((GetLastError() != ERROR_FILE_NOT_FOUND)
              && (GetLastError() != ERROR_PATH_NOT_FOUND)) {
    // Unexpected error
    return false;
  }

  // Directory doesn't exist, look up one directory level
  if (!pathname.parent_folder().empty()) {
    Pathname parent(pathname);
    parent.SetFolder(pathname.parent_folder());
    if (!CreateFolder(parent)) {
      return false;
    }
  }

  return (::CreateDirectory(path16.c_str(), nullptr) != 0);
}

FileStream *Win32Filesystem::OpenFile(const Pathname &filename,
                                      const std::string &mode) {
  FileStream *fs = new FileStream();
  if (fs && !fs->Open(filename.pathname().c_str(), mode.c_str(), nullptr)) {
    delete fs;
    fs = nullptr;
  }
  return fs;
}

bool Win32Filesystem::DeleteFile(const Pathname &filename) {
  LOG(LS_INFO) << "Deleting file " << filename.pathname();
  if (!IsFile(filename)) {
    RTC_DCHECK(IsFile(filename));
    return false;
  }
  return ::DeleteFile(ToUtf16(filename.pathname()).c_str()) != 0;
}

bool Win32Filesystem::DeleteEmptyFolder(const Pathname &folder) {
  LOG(LS_INFO) << "Deleting folder " << folder.pathname();

  std::string no_slash(folder.pathname(), 0, folder.pathname().length()-1);
  return ::RemoveDirectory(ToUtf16(no_slash).c_str()) != 0;
}

#if defined(WINRT)
bool Win32Filesystem::GetTemporaryFolder(Pathname &pathname, bool create,
                                         const std::string *append) {
  auto folder = Windows::Storage::ApplicationData::Current->TemporaryFolder;
  wchar_t buffer[MAX_PATH + 1];
  wcsncpy_s(buffer, arraysize(buffer), folder->Path->Data(), _TRUNCATE);
  size_t len = strlen(buffer);
  if ((len > 0) && (buffer[len-1] != '\\')) {
    len += strcpyn(buffer + len, arraysize(buffer) - len, L"\\");
  }
  if (len >= arraysize(buffer) - 1)
    return false;
  pathname.clear();
  pathname.SetFolder(ToUtf8(buffer));
  if (append != NULL) {
    RTC_DCHECK(!append->empty());
    pathname.AppendFolder(*append);
  }
  return !create || CreateFolder(pathname);
}
#else // defined(WINRT)
bool Win32Filesystem::GetTemporaryFolder(Pathname &pathname, bool create,
                                         const std::string *append) {
  wchar_t buffer[MAX_PATH + 1];
    return false;
  if (!IsCurrentProcessLowIntegrity() &&
      !::GetLongPathName(buffer, buffer, arraysize(buffer)))
    return false;
  size_t len = strlen(buffer);
  if ((len > 0) && (buffer[len-1] != '\\')) {
    len += strcpyn(buffer + len, arraysize(buffer) - len, L"\\");
  }
  if (len >= arraysize(buffer) - 1)
    return false;
  pathname.clear();
  pathname.SetFolder(ToUtf8(buffer));
  if (append != NULL) {
    RTC_DCHECK(!append->empty());
    pathname.AppendFolder(*append);
  }
  return !create || CreateFolder(pathname);
}
#endif // defined(WINRT)

#if defined(WINRT)
std::string Win32Filesystem::TempFilename(const Pathname &dir,
                                          const std::string &prefix) {
  Pathname fullpath = dir;
  GUID g;
  CoCreateGuid(&g);

  wchar_t filename[MAX_PATH];
  // printf format for the filename, consists of prefix followed by guid.
  wchar_t* maskForFN = L"%s_%08x_%04x_%04x_%02x%02x_%02x%02x%02x%02x%02x%02x";

  swprintf(filename, maskForFN, ToUtf16(prefix).c_str(), g.Data1, g.Data2, g.Data3,
    UINT(g.Data4[0]), UINT(g.Data4[1]), UINT(g.Data4[2]), UINT(g.Data4[3]),
    UINT(g.Data4[4]), UINT(g.Data4[5]), UINT(g.Data4[6]), UINT(g.Data4[7]));

  fullpath.AppendPathname(ToUtf8(filename));
  // Make sure the file exists.
  HANDLE handle = ::CreateFile2(
    ToUtf16(fullpath.pathname()).c_str(),
    GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
    CREATE_NEW,
    NULL);
  if (INVALID_HANDLE_VALUE == handle) {
    LOG_ERR(LS_ERROR) << "CreateFile() failed";
    return false;
  }
  if (!::CloseHandle(handle)) {
    LOG_ERR(LS_ERROR) << "CloseFile() failed";
    // Continue.
  }
  return fullpath.pathname();
}
#else // defined(WINRT)
std::string Win32Filesystem::TempFilename(const Pathname &dir,
                                          const std::string &prefix) {
  wchar_t filename[MAX_PATH];
  if (::GetTempFileName(ToUtf16(dir.pathname()).c_str(),
                        ToUtf16(prefix).c_str(), 0, filename) != 0)
    return ToUtf8(filename);
  RTC_DCHECK(false);
  return "";
}
#endif // defined(WINRT)

bool Win32Filesystem::MoveFile(const Pathname &old_path,
                               const Pathname &new_path) {
  if (!IsFile(old_path)) {
    RTC_DCHECK(IsFile(old_path));
    return false;
  }
  LOG(LS_INFO) << "Moving " << old_path.pathname()
               << " to " << new_path.pathname();
#if defined(WINRT)
  return ::MoveFileEx(ToUtf16(old_path.pathname()).c_str(),
                    ToUtf16(new_path.pathname()).c_str(), 0) != 0;
#else // defined(WINRT)
  return ::MoveFile(ToUtf16(old_path.pathname()).c_str(),
                    ToUtf16(new_path.pathname()).c_str()) != 0;
#endif // defined(WINRT)
}

bool Win32Filesystem::IsFolder(const Pathname &path) {
  WIN32_FILE_ATTRIBUTE_DATA data = {0};
  if (0 == ::GetFileAttributesEx(ToUtf16(path.pathname()).c_str(),
                                 GetFileExInfoStandard, &data))
    return false;
  return (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ==
      FILE_ATTRIBUTE_DIRECTORY;
}

bool Win32Filesystem::IsFile(const Pathname &path) {
  WIN32_FILE_ATTRIBUTE_DATA data = {0};
  if (0 == ::GetFileAttributesEx(ToUtf16(path.pathname()).c_str(),
                                 GetFileExInfoStandard, &data))
    return false;
  return (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool Win32Filesystem::IsAbsent(const Pathname& path) {
  WIN32_FILE_ATTRIBUTE_DATA data = {0};
  if (0 != ::GetFileAttributesEx(ToUtf16(path.pathname()).c_str(),
                                 GetFileExInfoStandard, &data))
    return false;
  DWORD err = ::GetLastError();
  return (ERROR_FILE_NOT_FOUND == err || ERROR_PATH_NOT_FOUND == err);
}

bool Win32Filesystem::CopyFile(const Pathname &old_path,
                               const Pathname &new_path) {
#if defined(WINRT)
  COPYFILE2_EXTENDED_PARAMETERS params = { 0 };
  params.dwSize = sizeof(COPYFILE2_EXTENDED_PARAMETERS);
  params.dwCopyFlags = COPY_FILE_FAIL_IF_EXISTS;
  return ::CopyFile2(ToUtf16(old_path.pathname()).c_str(),
                    ToUtf16(new_path.pathname()).c_str(), &params) != 0;
#else // defined(WINRT)
  return ::CopyFile(ToUtf16(old_path.pathname()).c_str(),
                    ToUtf16(new_path.pathname()).c_str(), TRUE) != 0;
#endif // defined(WINRT)
}

#if defined(WINRT)
bool Win32Filesystem::IsTemporaryPath(const Pathname& pathname) {
  auto folder = Windows::Storage::ApplicationData::Current->TemporaryFolder;
  TCHAR buffer[MAX_PATH + 1];
  wcsncpy_s(buffer, arraysize(buffer), folder->Path->Data(), _TRUNCATE);
  return (::strnicmp(ToUtf16(pathname.pathname()).c_str(),
                     buffer, strlen(buffer)) == 0);
}
#else // defined(WINRT)
bool Win32Filesystem::IsTemporaryPath(const Pathname& pathname) {
  TCHAR buffer[MAX_PATH + 1];
  if (!::GetTempPath(arraysize(buffer), buffer))
    return false;
  if (!IsCurrentProcessLowIntegrity() &&
      !::GetLongPathName(buffer, buffer, arraysize(buffer)))
    return false;
  return (::strnicmp(ToUtf16(pathname.pathname()).c_str(),
                     buffer, strlen(buffer)) == 0);
}
#endif // defined(WINRT)

bool Win32Filesystem::GetFileSize(const Pathname &pathname, size_t *size) {
  WIN32_FILE_ATTRIBUTE_DATA data = {0};
  if (::GetFileAttributesEx(ToUtf16(pathname.pathname()).c_str(),
                            GetFileExInfoStandard, &data) == 0)
  return false;
  *size = data.nFileSizeLow;
  return true;
}

bool Win32Filesystem::GetFileTime(const Pathname& path, FileTimeType which,
                                  time_t* time) {
  WIN32_FILE_ATTRIBUTE_DATA data = {0};
  if (::GetFileAttributesEx(ToUtf16(path.pathname()).c_str(),
                            GetFileExInfoStandard, &data) == 0)
    return false;
  switch (which) {
  case FTT_CREATED:
    FileTimeToUnixTime(data.ftCreationTime, time);
    break;
  case FTT_MODIFIED:
    FileTimeToUnixTime(data.ftLastWriteTime, time);
    break;
  case FTT_ACCESSED:
    FileTimeToUnixTime(data.ftLastAccessTime, time);
    break;
  default:
    return false;
  }
  return true;
}

#if defined(WINRT)
bool Win32Filesystem::GetAppPathname(Pathname* path) {
  auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;
  TCHAR buffer[MAX_PATH + 1];
  wcsncpy_s(buffer, arraysize(buffer), folder->Path->Data(), _TRUNCATE);
  path->SetPathname(ToUtf8(buffer));
  return true;
}
#else // defined(WINRT)
bool Win32Filesystem::GetAppPathname(Pathname* path) {
  TCHAR buffer[MAX_PATH + 1];
  if (0 == ::GetModuleFileName(NULL, buffer, arraysize(buffer)))
    return false;
  path->SetPathname(ToUtf8(buffer));
  return true;
}
#endif // defined(WINRT)

bool Win32Filesystem::GetAppTempFolder(Pathname* path) {
  if (!GetAppPathname(path))
    return false;
  std::string filename(path->filename());
  return GetTemporaryFolder(*path, true, &filename);
}

}  // namespace rtc
