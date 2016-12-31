#pragma once

#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
#ifndef _IOS_SDK_
#define _IOS_SDK_
#endif
#endif

#include <sys/types.h>

#if defined (TARGET_SMARTAR_WINDOWS)
#	include <stdint.h>
#	include <windows.h>
#	if defined (UNITY_EDITOR) || defined (UNITY_STANDALONE_WIN)
#		define CLASS_DECLSPEC
#	else
#		ifdef _EXPORTING
#			define CLASS_DECLSPEC __declspec(dllexport)
#		else
#			define CLASS_DECLSPEC __declspec(dllimport)
#		endif
#	endif
#	define WINAPI __stdcall
#	define SAR_SMARTAR_LOG_FATAL ": fatal: "
#	define SAR_SMARTAR_LOG_ERROR ": error: "
#	define SAR_SMARTAR_LOG_WARN ": warn: "
#	define SAR_SMARTAR_LOG_INFO ": info: "
#	define SAR_SMARTAR_LOG_DEBUG ": debug: "
#	define SAR_SMARTAR_LOG_VERBOSE ": verbose: "

#	define SAR_SMARTAR_LOG_IMPL(level, tag, ...)  // unused

#else
#	define CLASS_DECLSPEC
#	define WINAPI
#endif

#ifdef __ANDROID__
#  include <android/log.h>

#  define SAR_SMARTAR_LOG_FATAL ANDROID_LOG_FATAL
#  define SAR_SMARTAR_LOG_ERROR ANDROID_LOG_ERROR
#  define SAR_SMARTAR_LOG_WARN ANDROID_LOG_WARN
#  define SAR_SMARTAR_LOG_INFO ANDROID_LOG_INFO
#  define SAR_SMARTAR_LOG_DEBUG ANDROID_LOG_DEBUG
#  define SAR_SMARTAR_LOG_VERBOSE ANDROID_LOG_VERBOSE

#  define SAR_SMARTAR_LOG_IMPL __android_log_print
#endif
#ifdef _IOS_SDK_
#  include <stdint.h>
#  define SAR_SMARTAR_LOG_FATAL ": fatal: "
#  define SAR_SMARTAR_LOG_ERROR ": error: "
#  define SAR_SMARTAR_LOG_WARN ": warn: "
#  define SAR_SMARTAR_LOG_INFO ": info: "
#  define SAR_SMARTAR_LOG_DEBUG ": debug: "
#  define SAR_SMARTAR_LOG_VERBOSE ": verbose: "

#  define SAR_SMARTAR_LOG_IMPL(level, tag, ...) sarSmartar::sarIosLog(tag level __VA_ARGS__)

namespace sarSmartar {
    void sarIosLog(const char* format, ...);
}
#endif



namespace sarSmartar {
    // * Android: Note that the constants below must match the java-code constants. 
    enum SarSensorType {
        SAR_SENSOR_TYPE_ACCELEROMETER,
        SAR_SENSOR_TYPE_GYROSCOPE,
    };

    struct SarSensorStateImpl
    {
    public:
        static const int32_t NUM_VALUES = 3;

        SarSensorType type_;
        float values_[NUM_VALUES];
#ifdef __ANDROID__
        uint64_t timestampNsec_;
#endif
#ifdef _IOS_SDK_
        double timestampSec_;
#endif
#if defined (TARGET_SMARTAR_WINDOWS) || defined (TARGET_SMARTAR_LINUX)
        uint64_t timestampNsec_;
#endif
    };
} // end of namespace smartar
