//
// Created by happymonkey1 on 5/10/24.
//

#ifndef KB_SEARCH_CORE_TYPES_H
#define KB_SEARCH_CORE_TYPES_H

#include <cstdint>
#include <optional>

namespace kb
{ // start namespace kb

using u8 = std::uint8_t;
using i8 = std::int8_t;
using u32 = std::uint32_t;
using i32 = std::int32_t;
using u64 = std::uint64_t;
using i64 = std::int64_t;
using f32 = float;
using f64 = double;


template <typename T>
using option = std::optional<T>;

} // end namespace kb

#endif //KB_SEARCH_CORE_TYPES_H
