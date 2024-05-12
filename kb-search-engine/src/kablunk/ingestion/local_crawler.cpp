//
// Created by happymonkey1 on 5/10/24.
//

#include "kablunk/ingestion/local_crawler.h"
#include "kablunk/core/core.h"

namespace kb {

local_crawler::local_crawler(crawler_create_specification p_specification) noexcept
    : m_root{ p_specification.m_root }, m_max_index_count{ p_specification.m_max_index_count },
    m_document_store_accessor{ document_store_accessor::create(p_specification.m_document_store_ptr) } {
}

auto local_crawler::create(crawler_create_specification p_specification) noexcept -> local_crawler {
    return local_crawler{ std::move(p_specification) };
}

auto local_crawler::start_crawling() noexcept -> void {
    m_found_directories.emplace(m_root);

    KB_CORE_INFO("[local_crawler]: Starting crawl with root='{}'", m_root.c_str())

    m_running = true;
    while (m_running && !m_found_directories.empty()) {
        const auto current_root = m_found_directories.top();

        crawl_directory(current_root);

        m_found_directories.pop();
    }

    KB_CORE_INFO("[local_crawler]: Finished crawl with root='{}'", m_root.c_str())
    KB_CORE_INFO(
        "[local_crawler]:   indexed {} files, {} errors ({} total files)",
        m_index_count,
        m_error_count,
        m_index_count + m_error_count
    );

    // not explicitly required, but let's release the memory allocated by the internal document read buffer
    details::release_document_read_buffer();
}

auto local_crawler::crawl_directory(const std::filesystem::path& p_current_root) noexcept -> void {
    KB_CORE_INFO("[local_crawler]: Crawling '{}'", p_current_root.c_str())

    for (auto&& dir_entry : std::filesystem::recursive_directory_iterator(p_current_root)) {
        // let's not deal with symlinks for now...
        if (dir_entry.is_symlink()) {
            continue;
        }

        if (dir_entry.is_directory()) {
            continue;
        } else if (dir_entry.is_regular_file()) {
            // skip parsing if not implemented
            const auto ext = dir_entry.path().extension();
            const auto ext_view = std::string_view{ ext.c_str() };
            const auto doc_type = document_extension_str_to_type(ext_view);

            if (!is_document_parsing_implemented(doc_type)) {
                continue;
            }

            if (m_max_index_count != 0 && m_index_count >= m_max_index_count) {
                m_running = false;
                return;
            }

            const auto indexed_file = document::create(dir_entry.path(), doc_type, m_document_store_accessor);
            if (!indexed_file) {
                ++m_error_count;
                continue;
            }

            m_document_store_accessor.put_document(indexed_file->m_uri, *indexed_file);

            ++m_index_count;
        } else {
            KB_CORE_WARN("[local_crawler]: Found unhandled dir entry {}", dir_entry.path().c_str())
        }
    }
}

} // end namespace kb
