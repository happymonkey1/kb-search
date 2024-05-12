//
// Created by happymonkey1 on 5/10/24.
//

#ifndef KB_SEARCH_LOCAL_CRAWLER_H
#define KB_SEARCH_LOCAL_CRAWLER_H

#include "kablunk/ingestion/document.h"

#include <filesystem>
#include <stack>
#include <kablunk/persistence/document_store.h>
#include <kablunk/persistence/document_store_accessor.h>

namespace kb {

struct crawler_create_specification {
    std::filesystem::path m_root{};
    u64 m_max_index_count = 0;
    // non-owning pointer to the document store
    document_store* m_document_store_ptr = nullptr;
};

class local_crawler {
public:
    local_crawler() noexcept = default;
    ~local_crawler() noexcept = default;

    static auto create(crawler_create_specification p_specification) noexcept -> local_crawler;

    auto start_crawling() noexcept -> void;
    auto stop() noexcept -> void { m_running = false; }
private:
    explicit local_crawler(crawler_create_specification p_specification) noexcept;

    auto crawl_directory(const std::filesystem::path& p_current_root) noexcept -> void;
private:
    std::filesystem::path m_root{};
    u64 m_max_index_count = 0;
    document_store_accessor m_document_store_accessor{};
    bool m_running = false;
    std::stack<std::filesystem::directory_entry> m_found_directories{};
    u64 m_index_count = 0;
    u64 m_error_count = 0;
};

} // end namespace kb

#endif //KB_SEARCH_LOCAL_CRAWLER_H
