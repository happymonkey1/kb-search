//
// Created by happymonkey1 on 5/10/24.
//

#include "kablunk/crawler/local_crawler.h"
#include "kablunk/core/core.h"

namespace kb {

local_crawler::local_crawler(crawler_create_specification p_specification) noexcept
    : m_specification{ std::move(p_specification) } {
}

auto local_crawler::create(crawler_create_specification p_specification) noexcept -> std::unique_ptr<local_crawler> {
    return std::make_unique<local_crawler>(std::move(p_specification));
}

auto local_crawler::start_crawling() noexcept -> void {
    m_found_directories.emplace(m_specification.m_root);

    KB_CORE_INFO("[local_crawler]: Starting crawl with root='{}'", m_specification.m_root.c_str())

    m_running = true;
    while (m_running && !m_found_directories.empty()) {
        const auto current_root = m_found_directories.top();

        crawl_directory(current_root);

        m_found_directories.pop();
    }

    KB_CORE_INFO("[local_crawler]: Finished crawl with root='{}'", m_specification.m_root.c_str())
    KB_CORE_INFO(
        "[local_crawler]:   indexed {} files, {} errors ({} total files)",
        m_index_count,
        m_error_count,
        m_index_count + m_error_count
    );
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
            if (!is_document_parsing_implemented(dir_entry)) {
                continue;
            }

            if (m_specification.m_max_index_count != 0 && m_index_count >= m_specification.m_max_index_count) {
                m_running = false;
                return;
            }

            KB_CORE_TRACE("[local_crawler]: Indexing '{}'", dir_entry.path().c_str());
            const auto indexed_file = index_file(dir_entry);
            if (!indexed_file) {
                ++m_error_count;
                continue;
            }

            KB_CORE_TRACE("[local_crawler]:   Indexed content '{}'", indexed_file->m_content);
            ++m_index_count;
        } else {
            KB_CORE_WARN("[local_crawler]: Found unhandled dir entry {}", dir_entry.path().c_str())
        }
    }
}

auto local_crawler::index_file(const std::filesystem::path &p_file_path) const noexcept -> option<document> {
    const auto indexed_file = document::create(p_file_path);
    if (!indexed_file) {
        KB_CORE_WARN("[local_crawler]: Failed to index file '{}'", p_file_path.c_str())
        return indexed_file;
    }

    // TODO: add to "db"

    return indexed_file;
}

} // end namespace kb
