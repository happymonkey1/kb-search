//
// Created by happymonkey1 on 5/11/24.
//

#ifndef KB_SEARCH_TIMER_H
#define KB_SEARCH_TIMER_H

#include "kablunk/core/core_types.h"

#include <chrono>

namespace kb {
class timer
{
public:
    // typedef for underlying timer resolution
    using timer_resolution_t = std::chrono::microseconds;
public:
    // default constructor
    timer() { reset(); }
    // default destructor
    ~timer() = default;
    // reset the timer's start point to the current point in time
    void reset() { m_start_time = std::chrono::high_resolution_clock::now(); }
    // get the elapsed time (in seconds) between the recorded start point and current point in time
    f32 get_elapsed()
    {
        return static_cast<f32>(
            std::chrono::duration_cast<timer_resolution_t>(
                std::chrono::high_resolution_clock::now() - m_start_time
            ).count()
        ) / (1000.f * 1000.f);
    }
    // get the elapsed time (in milliseconds) between the recorded start point and current point in time
    f32 get_elapsed_ms()
    {
        return static_cast<f32>(
            std::chrono::duration_cast<timer_resolution_t>(
                std::chrono::high_resolution_clock::now() - m_start_time
            ).count()
        ) / 1000.f;
    }
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time;
};
}

#endif //KB_SEARCH_TIMER_H
