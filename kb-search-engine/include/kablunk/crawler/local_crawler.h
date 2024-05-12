//
// Created by happymonkey1 on 5/10/24.
//

#ifndef KB_SEARCH_LOCAL_CRAWLER_H
#define KB_SEARCH_LOCAL_CRAWLER_H

#include "kablunk/crawler/document.h"

#include <filesystem>
#include <stack>

namespace kb {

struct crawler_create_specification {
    std::filesystem::path m_root{};
    u64 m_max_index_count = 0;
};

class local_crawler {
public:
    local_crawler() noexcept = default;
    explicit local_crawler(crawler_create_specification p_specification) noexcept;
    ~local_crawler() noexcept = default;

    static auto create(crawler_create_specification p_specification) noexcept -> std::unique_ptr<local_crawler>;

    auto start_crawling() noexcept -> void;
    auto stop() noexcept -> void { m_running = false; }
private:
    auto crawl_directory(const std::filesystem::path& p_current_root) noexcept -> void;
    [[nodiscard]] auto index_file(const std::filesystem::path& p_file_path) const noexcept -> option<document>;
private:
    crawler_create_specification m_specification{};
    bool m_running = false;
    std::stack<std::filesystem::directory_entry> m_found_directories{};
    u64 m_index_count = 0;
    u64 m_error_count = 0;
};

} // end namespace kb

#endif //KB_SEARCH_LOCAL_CRAWLER_H
