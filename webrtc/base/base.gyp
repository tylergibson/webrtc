# Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

{
  'includes': [ '../build/common.gypi', ],
  'conditions': [
    ['os_posix == 1 and OS != "mac" and OS != "ios"', {
      'conditions': [
        ['sysroot!=""', {
          'variables': {
            'pkg-config': '../../../build/linux/pkg-config-wrapper "<(sysroot)" "<(target_arch)"',
          },
        }, {
          'variables': {
            'pkg-config': 'pkg-config'
          },
        }],
      ],
    }],
    ['OS=="ios" or (OS=="mac" and mac_deployment_target=="10.7")', {
      'targets': [
        {
          'target_name': 'rtc_base_objc',
          'type': 'static_library',
          'includes': [ '../build/objc_common.gypi' ],
          'dependencies': [
            'rtc_base',
          ],
          'sources': [
            'objc/NSString+StdString.h',
            'objc/NSString+StdString.mm',
            'objc/RTCDispatcher.h',
            'objc/RTCDispatcher.m',
            'objc/RTCFieldTrials.h',
            'objc/RTCFieldTrials.mm',
            'objc/RTCFileLogger.h',
            'objc/RTCFileLogger.mm',
            'objc/RTCLogging.h',
            'objc/RTCLogging.mm',
            'objc/RTCMacros.h',
            'objc/RTCSSLAdapter.h',
            'objc/RTCSSLAdapter.mm',
            'objc/RTCTracing.h',
            'objc/RTCTracing.mm',
          ],
          'conditions': [
            ['OS=="ios"', {
              'sources': [
                'objc/RTCCameraPreviewView.h',
                'objc/RTCCameraPreviewView.m',
              ],
              'all_dependent_settings': {
                'xcode_settings': {
                  'OTHER_LDFLAGS': [
                    '-framework AVFoundation',
                  ],
                },
              },
            }],
          ],
        }
      ],
    }], # OS=="ios"
  ],
  'targets': [
    {
      # The subset of rtc_base approved for use outside of libjingle.
      'target_name': 'rtc_base_approved',
      'type': 'static_library',
      'sources': [
        'array_view.h',
        'atomicops.h',
        'bitbuffer.cc',
        'bitbuffer.h',
        'buffer.cc',
        'buffer.h',
        'bufferqueue.cc',
        'bufferqueue.h',
        'bytebuffer.cc',
        'bytebuffer.h',
        'byteorder.h',
        'checks.cc',
        'checks.h',
        'constructormagic.h',
        'criticalsection.cc',
        'criticalsection.h',
        'deprecation.h',
        'event.cc',
        'event.h',
        'event_tracer.cc',
        'event_tracer.h',
        'exp_filter.cc',
        'exp_filter.h',
        'logging.cc',
        'logging.h',
        'md5.cc',
        'md5.h',
        'md5digest.cc',
        'md5digest.h',
        'optional.h',
        'platform_file.cc',
        'platform_file.h',
        'platform_thread.cc',
        'platform_thread.h',
        'platform_thread_types.h',
        'random.cc',
        'random.h',
        'rate_statistics.cc',
        'rate_statistics.h',
        'ratetracker.cc',
        'ratetracker.h',
        'refcount.h',
        'safe_conversions.h',
        'safe_conversions_impl.h',
        'scoped_ptr.h',
        'scoped_ref_ptr.h',
        'stringencode.cc',
        'stringencode.h',
        'stringutils.cc',
        'stringutils.h',
        'systeminfo.cc',
        'systeminfo.h',
        'template_util.h',
        'thread_annotations.h',
        'thread_checker.h',
        'thread_checker_impl.cc',
        'thread_checker_impl.h',
        'timeutils.cc',
        'timeutils.h',
        'trace_event.h',
      ],
      'conditions': [
        ['build_with_chromium==1', {
          'dependencies': [
            '<(DEPTH)/base/base.gyp:base',
          ],
          'include_dirs': [
            '../../webrtc_overrides',
          ],
          'sources': [
            '../../webrtc_overrides/webrtc/base/logging.cc',
            '../../webrtc_overrides/webrtc/base/logging.h',
          ],
          'sources!': [
            'logging.cc',
            'logging.h',
          ],
        }],
      ],
    },
    {
      'target_name': 'rtc_base',
      'type': 'static_library',
      'dependencies': [
        '<(webrtc_root)/common.gyp:webrtc_common',
        'rtc_base_approved',
      ],
      'export_dependent_settings': [
        'rtc_base_approved',
      ],
      'defines': [
        'FEATURE_ENABLE_SSL',
        'SSL_USE_OPENSSL',
        'HAVE_OPENSSL_SSL_H',
        'LOGGING=1',
      ],
      'sources': [
        'arraysize.h',
        'asyncfile.cc',
        'asyncfile.h',
        'asyncinvoker.cc',
        'asyncinvoker.h',
        'asyncinvoker-inl.h',
        'asyncpacketsocket.cc',
        'asyncpacketsocket.h',
        'asyncresolverinterface.cc',
        'asyncresolverinterface.h',
        'asyncsocket.cc',
        'asyncsocket.h',
        'asynctcpsocket.cc',
        'asynctcpsocket.h',
        'asyncudpsocket.cc',
        'asyncudpsocket.h',
        'autodetectproxy.cc',
        'autodetectproxy.h',
        'bandwidthsmoother.cc',
        'bandwidthsmoother.h',
        'base64.cc',
        'base64.h',
        'bind.h',
        'callback.h',
        'common.cc',
        'common.h',
        'copyonwritebuffer.cc',
        'copyonwritebuffer.h',
        'crc32.cc',
        'crc32.h',
        'cryptstring.cc',
        'cryptstring.h',
        'dbus.cc',
        'dbus.h',
        'diskcache.cc',
        'diskcache.h',
        'diskcache_win32.cc',
        'diskcache_win32.h',
        'filerotatingstream.cc',
        'filerotatingstream.h',
        'fileutils.cc',
        'fileutils.h',
        'fileutils_mock.h',
        'firewallsocketserver.cc',
        'firewallsocketserver.h',
        'flags.cc',
        'flags.h',
        'format_macros.h',
        'gunit_prod.h',
        'helpers.cc',
        'helpers.h',
        'httpbase.cc',
        'httpbase.h',
        'httpclient.cc',
        'httpclient.h',
        'httpcommon-inl.h',
        'httpcommon.cc',
        'httpcommon.h',
        'httprequest.cc',
        'httprequest.h',
        'httpserver.cc',
        'httpserver.h',
        'ifaddrs-android.cc',
        'ifaddrs-android.h',
        'ifaddrs_converter.cc',
        'ifaddrs_converter.h',
        'macifaddrs_converter.cc',
        'iosfilesystem.mm',
        'ipaddress.cc',
        'ipaddress.h',
        'json.cc',
        'json.h',
        'latebindingsymboltable.cc',
        'latebindingsymboltable.h',
        'libdbusglibsymboltable.cc',
        'libdbusglibsymboltable.h',
        'linux.cc',
        'linux.h',
        'linuxfdwalk.c',
        'linuxfdwalk.h',
        'linked_ptr.h',
        'logsinks.cc',
        'logsinks.h',
        'macasyncsocket.cc',
        'macasyncsocket.h',
        'maccocoasocketserver.h',
        'maccocoasocketserver.mm',
        'maccocoathreadhelper.h',
        'maccocoathreadhelper.mm',
        'macconversion.cc',
        'macconversion.h',
        'macsocketserver.cc',
        'macsocketserver.h',
        'macutils.cc',
        'macutils.h',
        'macwindowpicker.cc',
        'macwindowpicker.h',
        'mathutils.h',
        'messagedigest.cc',
        'messagedigest.h',
        'messagehandler.cc',
        'messagehandler.h',
        'messagequeue.cc',
        'messagequeue.h',
        'multipart.cc',
        'multipart.h',
        'natserver.cc',
        'natserver.h',
        'natsocketfactory.cc',
        'natsocketfactory.h',
        'nattypes.cc',
        'nattypes.h',
        'nethelpers.cc',
        'nethelpers.h',
        'network.cc',
        'network.h',
        'networkmonitor.cc',
        'networkmonitor.h',
        'nullsocketserver.h',
        'openssl.h',
        'openssladapter.cc',
        'openssladapter.h',
        'openssldigest.cc',
        'openssldigest.h',
        'opensslidentity.cc',
        'opensslidentity.h',
        'opensslstreamadapter.cc',
        'opensslstreamadapter.h',
        'optionsfile.cc',
        'optionsfile.h',
        'pathutils.cc',
        'pathutils.h',
        'physicalsocketserver.cc',
        'physicalsocketserver.h',
        'posix.cc',
        'posix.h',
        'profiler.cc',
        'profiler.h',
        'proxydetect.cc',
        'proxydetect.h',
        'proxyinfo.cc',
        'proxyinfo.h',
        'proxyserver.cc',
        'proxyserver.h',
        'ratelimiter.cc',
        'ratelimiter.h',
        'referencecountedsingletonfactory.h',
        'rollingaccumulator.h',
        'rtccertificate.cc',
        'rtccertificate.h',
        'scoped_autorelease_pool.h',
        'scoped_autorelease_pool.mm',
        'scopedptrcollection.h',
        'sec_buffer.h',
        'sha1.cc',
        'sha1.h',
        'sha1digest.cc',
        'sha1digest.h',
        'sharedexclusivelock.cc',
        'sharedexclusivelock.h',
        'signalthread.cc',
        'signalthread.h',
        'sigslot.cc',
        'sigslot.h',
        'sigslotrepeater.h',
        'socket.h',
        'socketadapters.cc',
        'socketadapters.h',
        'socketaddress.cc',
        'socketaddress.h',
        'socketaddresspair.cc',
        'socketaddresspair.h',
        'socketfactory.h',
        'socketpool.cc',
        'socketpool.h',
        'socketserver.h',
        'socketstream.cc',
        'socketstream.h',
        'ssladapter.cc',
        'ssladapter.h',
        'sslconfig.h',
        'sslfingerprint.cc',
        'sslfingerprint.h',
        'sslidentity.cc',
        'sslidentity.h',
        'sslroots.h',
        'sslsocketfactory.cc',
        'sslsocketfactory.h',
        'sslstreamadapter.cc',
        'sslstreamadapter.h',
        'sslstreamadapterhelper.cc',
        'sslstreamadapterhelper.h',
        'stream.cc',
        'stream.h',
        'task.cc',
        'task.h',
        'taskparent.cc',
        'taskparent.h',
        'taskrunner.cc',
        'taskrunner.h',
        'testclient.cc',
        'testclient.h',
        'thread.cc',
        'thread.h',
        'timing.cc',
        'timing.h',
        'transformadapter.cc',
        'transformadapter.h',
        'unixfilesystem.cc',
        'unixfilesystem.h',
        'urlencode.cc',
        'urlencode.h',
        'versionparsing.cc',
        'versionparsing.h',
        'virtualsocketserver.cc',
        'virtualsocketserver.h',
        'win32.cc',
        'win32.h',
        'win32filesystem.cc',
        'win32filesystem.h',
        'win32regkey.cc',
        'win32regkey.h',
        'win32securityerrors.cc',
        'win32socketinit.cc',
        'win32socketinit.h',
        'win32socketserver.cc',
        'win32socketserver.h',
        'win32window.cc',
        'win32window.h',
        'win32windowpicker.cc',
        'win32windowpicker.h',
        'window.h',
        'windowpicker.h',
        'windowpickerfactory.h',
        'winfirewall.cc',
        'winfirewall.h',
        'winping.cc',
        'winping.h',
        'worker.cc',
        'worker.h',
        'x11windowpicker.cc',
        'x11windowpicker.h',
      ],
      # TODO(henrike): issue 3307, make rtc_base build without disabling
      # these flags.
      'cflags!': [
        '-Wextra',
        '-Wall',
      ],
      'cflags_cc!': [
        '-Wnon-virtual-dtor',
      ],
      'direct_dependent_settings': {
        'cflags_cc!': [
          '-Wnon-virtual-dtor',
        ],
        'defines': [
          'FEATURE_ENABLE_SSL',
          'SSL_USE_OPENSSL',
          'HAVE_OPENSSL_SSL_H',
        ],
      },
      'include_dirs': [
        '../../third_party/jsoncpp/overrides/include',
        '../../third_party/jsoncpp/source/include',
      ],
      'conditions': [
        ['build_with_chromium==1', {
          'include_dirs': [
            '../../webrtc_overrides',
            '../../boringssl/src/include',
          ],
          'sources': [
            '../../webrtc_overrides/webrtc/base/win32socketinit.cc',
          ],
          'sources!': [
            'bandwidthsmoother.cc',
            'bandwidthsmoother.h',
            'bind.h',
            'callback.h',
            'dbus.cc',
            'dbus.h',
            'diskcache_win32.cc',
            'diskcache_win32.h',
            'fileutils_mock.h',
            'genericslot.h',
            'httpserver.cc',
            'httpserver.h',
            'json.cc',
            'json.h',
            'latebindingsymboltable.cc',
            'latebindingsymboltable.h',
            'libdbusglibsymboltable.cc',
            'libdbusglibsymboltable.h',
            'linuxfdwalk.c',
            'linuxfdwalk.h',
            'x11windowpicker.cc',
            'x11windowpicker.h',
            'logging.cc',
            'logging.h',
            'logsinks.cc',
            'logsinks.h',
            'macasyncsocket.cc',
            'macasyncsocket.h',
            'maccocoasocketserver.h',
            'maccocoasocketserver.mm',
            'macsocketserver.cc',
            'macsocketserver.h',
            'macwindowpicker.cc',
            'macwindowpicker.h',
            'mathutils.h',
            'multipart.cc',
            'multipart.h',
            'natserver.cc',
            'natserver.h',
            'natsocketfactory.cc',
            'natsocketfactory.h',
            'nattypes.cc',
            'nattypes.h',
            'optionsfile.cc',
            'optionsfile.h',
            'posix.cc',
            'posix.h',
            'profiler.cc',
            'profiler.h',
            'proxyserver.cc',
            'proxyserver.h',
            'referencecountedsingletonfactory.h',
            'rollingaccumulator.h',
            'safe_conversions.h',
            'safe_conversions_impl.h',
            'scopedptrcollection.h',
            'sec_buffer.h',
            'sslconfig.h',
            'sslroots.h',
            'testbase64.h',
            'testclient.cc',
            'testclient.h',
            'transformadapter.cc',
            'transformadapter.h',
            'versionparsing.cc',
            'versionparsing.h',
            'virtualsocketserver.cc',
            'virtualsocketserver.h',
            'win32regkey.cc',
            'win32regkey.h',
            'win32socketinit.cc',
            'win32socketinit.h',
            'win32socketserver.cc',
            'win32socketserver.h',
            'window.h',
            'windowpickerfactory.h',
            'windowpicker.h',
          ],
          'defines': [
            'NO_MAIN_THREAD_WRAPPING',
          ],
          'direct_dependent_settings': {
            'defines': [
              'NO_MAIN_THREAD_WRAPPING',
            ],
          },
        }, {
          'conditions': [
            ['build_json==1', {
              'dependencies': [
                '<(DEPTH)/third_party/jsoncpp/jsoncpp.gyp:jsoncpp',
              ],
            }, {
              'include_dirs': [
                '<(json_root)',
              ],
              'defines': [
                # When defined changes the include path for json.h to where it
                # is expected to be when building json outside of the standalone
                # build.
                'WEBRTC_EXTERNAL_JSON',
              ],
            }],
            ['OS=="win" and clang==1', {
              'msvs_settings': {
                'VCCLCompilerTool': {
                  'AdditionalOptions': [
                    # Disable warnings failing when compiling with Clang on Windows.
                    # https://bugs.chromium.org/p/webrtc/issues/detail?id=5366
                    '-Wno-sign-compare',
                    '-Wno-missing-braces',
                  ],
                },
              },
            }],
          ],
        }],
        ['OS == "android"', {
          'link_settings': {
            'libraries': [
              '-llog',
              '-lGLESv2',
            ],
          },
        }, {
          'sources!': [
            'ifaddrs-android.cc',
            'ifaddrs-android.h',
          ],
        }],
        ['OS=="ios"', {
          'sources/': [
            ['include', 'macconversion.*'],
          ],
          'all_dependent_settings': {
            'xcode_settings': {
              'OTHER_LDFLAGS': [
                '-framework CFNetwork',
                '-framework Foundation',
                '-framework Security',
                '-framework SystemConfiguration',
                '-framework UIKit',
              ],
            },
          },
        }],
        ['use_x11 == 1', {
          'link_settings': {
            'libraries': [
              '-ldl',
              '-lrt',
              '-lXext',
              '-lX11',
              '-lXcomposite',
              '-lXrender',
            ],
          },
        }, {
          'sources!': [
            'x11windowpicker.cc',
            'x11windowpicker.h',
          ],
        }],
        ['OS=="linux"', {
          'link_settings': {
            'libraries': [
              '-ldl',
              '-lrt',
            ],
          },
        }, {
          'sources!': [
            'dbus.cc',
            'dbus.h',
            'libdbusglibsymboltable.cc',
            'libdbusglibsymboltable.h',
            'linuxfdwalk.c',
          ],
        }],
        ['OS=="mac"', {
          'all_dependent_settings': {
            'link_settings': {
              'xcode_settings': {
                'OTHER_LDFLAGS': [
                  '-framework Cocoa',
                  '-framework Foundation',
                  '-framework IOKit',
                  '-framework Security',
                  '-framework SystemConfiguration',
                ],
              },
            },
          },
          'conditions': [
            ['target_arch=="ia32"', {
              'all_dependent_settings': {
                'link_settings': {
                  'xcode_settings': {
                    'OTHER_LDFLAGS': [
                      '-framework Carbon',
                    ],
                  },
                },
              },
            }],
          ],
        }, {
          'sources!': [
            'macasyncsocket.cc',
            'macasyncsocket.h',
            'maccocoasocketserver.h',
            'maccocoasocketserver.mm',
            'macconversion.cc',
            'macconversion.h',
            'macsocketserver.cc',
            'macsocketserver.h',
            'macutils.cc',
            'macutils.h',
            'macwindowpicker.cc',
            'macwindowpicker.h',
          ],
        }],
        ['OS=="win" and OS_RUNTIME=="winrt"', {
          'sources':[
            'loggingserver.cc',
            'loggingserver.h',
            'tracelog.cc',
            'tracelog.h',
          ],
          'sources!': [
            'ifaddrs_converter.cc',
            'win32regkey.cc',
            'win32regkey.h',
            'win32window.cc',
            'win32window.h',
            'win32windowpicker.cc',
            'win32windowpicker.h',
            'winfirewall.cc',
            'winfirewall.h',
            'sec_buffer.h',
            'schanneladapter.cc',
            'schanneladapter.h',
            'win32socketserver.cc',
            'win32socketserver.h',
          ],
        }],
        ['OS=="win" and winrt_platform!="win_phone" and  winrt_platform!="win10_arm"', {
          'sources!': [
            'ifaddrs_converter.cc',
          ],
          'link_settings': {
            'libraries': [
              '-lcrypt32.lib',
              '-liphlpapi.lib',
              '-lsecur32.lib',
            ],
          },
          # Suppress warnings about WIN32_LEAN_AND_MEAN.
          'msvs_disabled_warnings': [4005, 4703],
          'defines': [
            '_CRT_NONSTDC_NO_DEPRECATE',
          ],
        }],
		['OS=="win" and winrt_platform=="win10_arm"', {
          'msvs_disabled_warnings': [4005, 4703],
        }],
				['OS!="win"', {
          'sources/': [
            ['exclude', 'win32[a-z0-9]*\\.(h|cc)$'],
          ],
          'sources!': [
              'winping.cc',
              'winping.h',
              'winfirewall.cc',
              'winfirewall.h',
            ],
        }],
        ['os_posix==0', {
          'sources!': [
            'latebindingsymboltable.cc',
            'latebindingsymboltable.h',
            'posix.cc',
            'posix.h',
            'unixfilesystem.cc',
            'unixfilesystem.h',
          ],
        }, {
          'configurations': {
            'Debug_Base': {
              'defines': [
                # Chromium's build/common.gypi defines this for all posix
                # _except_ for ios & mac.  We want it there as well, e.g.
                # because ASSERT and friends trigger off of it.
                '_DEBUG',
              ],
            },
          }
        }],
        ['OS=="ios" or (OS=="mac" and target_arch!="ia32")', {
          'defines': [
            'CARBON_DEPRECATED=YES',
          ],
        }],
        ['OS!="ios" and OS!="mac"', {
          'sources!': [
            'macifaddrs_converter.cc',
            'scoped_autorelease_pool.mm',
          ],
        }],
        ['OS!="linux" and OS!="android"', {
          'sources!': [
            'linux.cc',
            'linux.h',
          ],
        }],
        ['build_ssl==1', {
          'dependencies': [
            '<(DEPTH)/third_party/boringssl/boringssl.gyp:boringssl',
          ],
        }, {
          'include_dirs': [
            '<(ssl_root)',
          ],
        }],
      ],
    },
    {
     'target_name': 'gtest_prod',
      'type': 'static_library',
      'sources': [
        'gtest_prod_util.h',
      ],
    },
  ],
}
