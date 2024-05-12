//
// Created by happymonkey1 on 5/10/24.
//

#ifndef KB_SEARCH_FILE_UTIL_H
#define KB_SEARCH_FILE_UTIL_H

#include <filesystem>

namespace kb::core {

[[nodiscard]] auto get_home_path() noexcept -> std::filesystem::path;

} // end namespace kb::core

#endif //KB_SEARCH_FILE_UTIL_H
