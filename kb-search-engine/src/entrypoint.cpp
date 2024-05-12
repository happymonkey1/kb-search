//
// Created by happymonkey1 on 5/10/24.
//

#include "kablunk/core/core.h"
#include "kablunk/ingestion/local_crawler.h"
#include "kablunk/persistence/document_store.h"
#include "kablunk/persistence/document_store_accessor.h"

#include <filesystem>
#include <kablunk/core/file_util.h>

// Entrypoint for the cli interface
int main() {
    KB_CORE_INFO("hello from kb-search!");

    // TODO: expose cli method to request a directory to be indexed
    constexpr bool k_development = false;
    const std::filesystem::path directory = kb::core::get_home_path() / "docs/cppreference/20230810/reference";

    auto document_store = kb::document_store::create({ .m_expected_document_count = 2048 });

    kb::timer crawl_timer{};
    auto crawler = kb::local_crawler::create({
        .m_root = directory,
        .m_max_index_count = k_development ? 10 : 0,
        .m_document_store_ptr = &document_store,
    });
    crawler.start_crawling();
    KB_CORE_INFO("Crawling took {} ms", crawl_timer.get_elapsed_ms())

    auto document_store_accessor = kb::document_store_accessor::create(&document_store);

    const auto query = "stack";

    KB_CORE_INFO("Starting tf idf computation for '{}'...", query)
    crawl_timer.reset();
    document_store_accessor.compute_tf_idf(query);
    KB_CORE_INFO("Finished tf idf computation");
    KB_CORE_INFO("  Took {} ms", crawl_timer.get_elapsed_ms())

    kb::core::log::logger::shutdown();
}
