//
// Created by happymonkey1 on 5/10/24.
//

#include "kablunk/core/core.h"
#include "kablunk/crawler/local_crawler.h"

#include <filesystem>
#include <kablunk/core/file_util.h>

// Entrypoint for the cli interface
int main() {
    KB_CORE_INFO("hello from kb-search!");

    // TODO: expose cli method to request a directory to be indexed
    const std::filesystem::path directory = kb::core::get_home_path() / "docs/cppreference/20230810/reference";

    kb::timer crawl_timer{};
    auto crawler = kb::local_crawler::create({ .m_root = directory, .m_max_index_count = 0 });
    crawler->start_crawling();
    KB_CORE_INFO("Crawling took {} ms", crawl_timer.get_elapsed_ms())

    kb::core::log::logger::shutdown();
}
