//
// Created by happymonkey1 on 5/12/24.
//

#ifndef KB_SEARCH_OWNING_BUFFER_H
#define KB_SEARCH_OWNING_BUFFER_H

#include "kablunk/core/core.h"

#include <type_traits>
#include <memory>
#include <cstring>

namespace kb {

template <typename T>
class owning_buffer {
public:
    owning_buffer() noexcept = default;
    // allocate a buffer with `p_count` elements
    explicit owning_buffer(size_t p_count)
        : m_buffer{ new T[p_count] }, m_buffer_size{ p_count }, m_count{ 0ull } { }
    ~owning_buffer() noexcept { delete[] m_buffer; }

    // copy construct
    owning_buffer(const owning_buffer& p_other) noexcept
        : m_buffer{ new T[p_other.m_buffer_size] }, m_buffer_size{p_other.m_buffer_size }, m_count{ p_other.m_count }
    {
        if constexpr (std::is_trivial_v<T>) {
            std::memcpy(m_buffer, p_other.m_buffer, m_buffer_size * sizeof(T));
        } else {
            for (size_t i = 0ull; i < m_buffer_size; ++i) {
                m_buffer[i] = p_other.m_buffer[i];
            }
        }
    }

    // move constructor
    owning_buffer(owning_buffer&& p_other) noexcept
        : m_buffer{ p_other.m_buffer }, m_buffer_size{ p_other.m_buffer_size }, m_count{ p_other.m_count }
    {
        p_other.m_buffer = nullptr;
        p_other.m_buffer_size = 0ull;
        p_other.m_count = 0ull;
    }

    // factory that takes ownership of the pointer to create a new owning buffer
    static inline auto from(T * __restrict p_data, size_t p_count) noexcept -> owning_buffer {
        KB_CORE_ASSERT(p_data, "[owning_buffer]: data pointer is null?");

        owning_buffer buffer{};
        buffer.m_buffer = p_data;
        buffer.m_buffer_size = p_count;
        buffer.m_count = p_count;
        return buffer;
    }

    // factory which copy data from the pointer to create a new owning buffer
    static inline auto copy(T const* __restrict p_data, size_t p_count) noexcept -> owning_buffer {
        KB_CORE_ASSERT(p_data, "[owning_buffer]: data pointer is null?");

        owning_buffer buffer{ p_count };
        if constexpr (std::is_trivial_v<T>) {
            std::memcpy(buffer.m_buffer, p_data, p_count * sizeof(T));
        } else {
            for (size_t i = 0; i < p_count; ++i) {
                buffer.m_buffer = p_data[i];
            }
        }
        buffer.m_count = p_count;
        return buffer;
    }

    // copy data from the point into the existing owning buffer
    // if there is not enough space in the current owning buffer, it will resize itself
    inline auto copy_into(T const* __restrict p_data, size_t p_count) noexcept -> void {
        KB_CORE_ASSERT(p_data, "[owning_buffer]: Data pointer is null?");
        KB_CORE_ASSERT(m_buffer, "[owning_buffer]: Internal buffer is null?");

        T* buf_ptr;
        if (m_buffer_size < p_count) {
            buf_ptr = new T[p_count];
        } else {
            buf_ptr = m_buffer;
        }

        if constexpr (std::is_trivial_v<T>) {
            std::memcpy(buf_ptr, p_data, p_count * sizeof(T));
        } else {
            for (size_t i = 0; i < p_count; ++i) {
                buf_ptr[i] = p_data[i];
            }
        }

        if (m_buffer_size < p_count) {
            delete[] m_buffer;
        }

        m_buffer = buf_ptr;
        m_buffer_size = std::max(p_count, m_buffer_size);
        m_count = p_count;
    }

    // reserve space in the buffer
    // does nothing if the requested size is <= the current size
    inline auto reserve(size_t p_count) noexcept -> void {
        if (p_count <= m_buffer_size) {
            return;
        }

        auto new_buffer = new T[p_count];
        if constexpr (std::is_trivial_v<T>) {
            std::memcpy(new_buffer, m_buffer, m_count * sizeof(T));
        } else {
            for (size_t i = 0; i < m_count; ++i) {
                new_buffer[i] = m_buffer[i];
            }
        }

        m_buffer = new_buffer;
        m_buffer_size = p_count;

        delete[] m_buffer;
    }

    // reserve space in the buffer and resize the number of elements
    // does nothing if the requested size is <= the current size
    inline auto resize(size_t p_count) noexcept -> void {
        KB_CORE_ASSERT(m_buffer, "[owning_buffer]: Internal buffer is null?")
        if (p_count > m_buffer_size) {
            const auto min_size = std::min(p_count, m_buffer_size);
            auto new_buffer = new T[p_count];
            if constexpr (std::is_trivial_v<T>) {
                std::memcpy(new_buffer, m_buffer, min_size * sizeof(T));
            } else {
                for (size_t i = 0; i < min_size; ++i) {
                    new_buffer[i] = m_buffer[i];
                }
            }

            m_buffer_size = p_count;
            delete[] m_buffer;
            m_buffer = new_buffer;
        }

        m_count = p_count;
    }

    // reserve space in the buffer and resize the number of elements
    // discards previous elements if the new size > current buffer size
    // does nothing if the requested size is <= the current size
    inline auto resize_discard_ok(size_t p_count) noexcept -> void {
        if (p_count > m_buffer_size) {
            delete[] m_buffer;
            m_buffer = new T[p_count];
            m_buffer_size = p_count;
        }

        m_count = p_count;
    }

    // release memory allocated by the buffer
    inline auto release() noexcept -> void {
        delete[] m_buffer;
        m_buffer = nullptr;
        m_buffer_size = 0ull;
        m_count = 0ull;
    }

    // return a pointer to the beginning of the buffer
    [[nodiscard]] inline auto data() noexcept -> T* { return m_buffer; }
    // return a const pointer to the beginning of the buffer
    [[nodiscard]] inline auto data() const noexcept -> T const* { return m_buffer; }
    // check whether the buffer contains any elements
    [[nodiscard]] inline auto empty() const noexcept -> bool { return m_count == 0; }
    // mutable indexing into the buffer with runtime bounds checking
    [[nodiscard]] inline auto operator[](size_t p_index) noexcept -> T& {
        KB_CORE_ASSERT(p_index < m_count, "[owning_buffer]: Index {} out of bounds {}", p_index, m_count);
        return m_buffer[p_index];
    }
    // immutable indexing into the buffer with runtime bounds checking
    [[nodiscard]] inline auto operator[](size_t p_index) const noexcept -> T const& {
        KB_CORE_ASSERT(p_index < m_count, "[owning_buffer]: Index {} out of bounds {}", p_index, m_count);
        return m_buffer[p_index];
    }
    // number of elements in the buffer
    [[nodiscard]] inline auto size() const noexcept -> size_t { return m_count; }
    // number of elements allocated for the buffer
    [[nodiscard]] inline auto allocated_size() const noexcept -> size_t { return m_buffer_size; }
    // size in bytes of the buffer that is being used
    [[nodiscard]] inline auto size_in_bytes() const noexcept -> size_t { return m_count * sizeof(T); }
    // size in bytes of the buffer that has been allocated
    [[nodiscard]] inline auto allocated_size_in_bytes() const noexcept -> size_t { return m_buffer_size * sizeof(T); }
    // zero out the buffer
    inline auto zero() noexcept -> void {
        if constexpr (std::is_trivial_v<T>) {
            std::memset(m_buffer, 0, m_buffer_size * sizeof(T));
        } else {
            KB_CORE_WARN("[owning_buffer]: Calling zero() on non trivial data!");
            for (size_t i = 0; i < m_buffer_size; ++i) {
                m_buffer[i] = {};
            }
        }
    }

    // copy assign
    auto operator=(const owning_buffer& p_other) noexcept -> owning_buffer& {
        if (this == &p_other) {
            return *this;
        }

        reserve(p_other);

        if constexpr (std::is_trivial_v<T>) {
            std::memcpy(m_buffer, p_other.m_buffer, m_buffer_size * sizeof(T));
        } else {
            for (size_t i = 0ull; i < m_buffer_size; ++i) {
                m_buffer[i] = p_other.m_buffer[i];
            }
        }

        m_buffer_size = p_other.m_buffer_size;
        m_count = p_other.m_count;
        return *this;
    }

    // move assign
    auto operator=(owning_buffer&& p_other) noexcept -> owning_buffer& {
        if (this == &p_other) {
            return *this;
        }

        m_buffer = p_other.m_buffer;
        m_buffer_size = p_other.m_buffer_size;
        m_count = p_other.m_count;

        return *this;
    }
private:
    // Owning pointer to the buffer
    T *m_buffer{};
    // size of the buffer, in terms of elements
    size_t m_buffer_size{};
    // number of elements stored within the buffer
    size_t m_count{};
};

} // end namespace kb

#endif //KB_SEARCH_OWNING_BUFFER_H
