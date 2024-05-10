//
// Created by happymonkey1 on 5/10/24.
//

#ifndef KB_SEARCH_LOGGER_H
#define KB_SEARCH_LOGGER_H

#include <spdlog/spdlog.h>
#include <memory>

namespace kb::core::log {

class logger
{
public:
    inline static const char* k_default_log_file_name = "kb-search.log";
public:
    logger() noexcept = default;
    ~logger() noexcept = default;

    logger(const logger&) = delete;
    logger(logger&&) = delete;

    // singleton getter
    static auto get() noexcept -> logger& {
        if (!s_instance) {
            s_instance = new logger();
            s_instance->init();
        }

        return *s_instance;
    }
    // cleanup
    static auto shutdown() noexcept -> void { delete s_instance; }

    auto get_core_logger() noexcept -> std::shared_ptr<spdlog::logger>& { return m_core_logger; }

    auto operator=(const logger&) noexcept -> logger& = delete;
    auto operator=(logger&) noexcept -> logger& = delete;
private:
    // initialize underlying spdlog implementation
    auto init() noexcept -> void;

private:
    inline static logger* s_instance = nullptr;
    std::shared_ptr<spdlog::logger> m_core_logger{};
};

} // end namespace kb::core::log

// TODO: remove macros and use template functions
#define KB_CORE_TRACE(...) ::kb::core::log::logger::get().get_core_logger()->trace(__VA_ARGS__);
#define KB_CORE_INFO(...) ::kb::core::log::logger::get().get_core_logger()->info(__VA_ARGS__);
#define KB_CORE_WARN(...) ::kb::core::log::logger::get().get_core_logger()->warn(__VA_ARGS__);
#define KB_CORE_ERROR(...) ::kb::core::log::logger::get().get_core_logger()->error(__VA_ARGS__);
#define KB_CORE_FATAL(...) ::kb::core::log::logger::get().get_core_logger()->critical(__VA_ARGS__);

#endif //KB_SEARCH_LOGGER_H
