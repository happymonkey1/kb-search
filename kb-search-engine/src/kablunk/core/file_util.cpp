//
// Created by happymonkey1 on 5/10/24.
//

#include "kablunk/core/file_util.h"
#include "kablunk/core/core.h"

#include <cstdlib>

namespace kb::core {

auto get_home_path() noexcept -> std::filesystem::path {
#ifdef KB_PLATFORM_LINUX
    return std::getenv("HOME");
#endif
#ifdef KB_PLATFORM_WINDOW
    return std::filesystem::path{ std::getenv("HOMEDRIVE") } / std::filesystem::path{ std::getenv("HOMEPATH") };
#endif

    KB_CORE_ASSERT(false, "Unhandled platform!");
    return {};
}

} // end namespace kb::core