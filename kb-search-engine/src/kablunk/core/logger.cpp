//
// Created by happymonkey1 on 5/10/24.
//

#include "kablunk/core/logger.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace kb::core::log {

auto logger::init() noexcept -> void {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::info);
    console_sink->set_pattern("%^[%T] [%^%l%$] %v");

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(k_default_log_file_name, true);
    file_sink->set_level(spdlog::level::trace);

    m_core_logger = std::make_shared<spdlog::logger>(
        "kb-search",
        std::initializer_list<spdlog::sink_ptr>{ console_sink, file_sink }
    );
    spdlog::register_logger(m_core_logger);
}

} // end namespace kb::core::log