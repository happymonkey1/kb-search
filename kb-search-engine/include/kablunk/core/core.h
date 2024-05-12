//
// Created by happymonkey1 on 5/10/24.
//

#ifndef KB_SEARCH_CORE_H
#define KB_SEARCH_CORE_H

#include "kablunk/core/core_types.h"
#include "kablunk/core/logger.h"
#include "kablunk/core/timer.h"

#include <vector>
#include <string>
#include <optional>
#include <memory>
#include <filesystem>
#include <string_view>

// Platform detection
#ifdef _WIN32
#	ifdef _WIN64
#		define KB_PLATFORM_WINDOWS
#		define BOOST_INTERPROCESS_WINDOWS
#	else
#		error "x86 is not supported!"
#	endif
#elif TARGET_OS_MAC
//defined(__APPLE__) || defined(__MACH__)
#	error "MacOS is not supported!"
#elif defined(__ANDROID__)
#	error "Android is not supported"
#elif defined(__linux__)
#	define KB_PLATFORM_LINUX
#else
#	error "Unknown platform"
#endif

#ifdef KB_PLATFORM_WINDOWS
#   define KB_DEBUG_BREAK __debugbreak()
#endif

#ifdef KB_PLATFORM_LINUX
#   include <csignal>
#   define KB_DEBUG_BREAK() raise(SIGTRAP)
#endif

#ifndef KB_DEBUG_BREAK
#   define KB_DEBUG_BREAK()
#endif


// TODO: nop in release or 'distribution' mode
#define KB_CORE_ASSERT(x, ...) { if (!(x)) { KB_CORE_ERROR(__VA_ARGS__); KB_DEBUG_BREAK(); } }
#define KB_VERIFY(x) { if (!(x)) { KB_CORE_ERROR("Assumption failed!"); KB_DEBUG_BREAK(); } }


#endif //KB_SEARCH_CORE_H
