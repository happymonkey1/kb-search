//
// Created by happymonkey1 on 5/11/24.
//

#ifndef KB_SEARCH_DOCUMENT_STORE_H
#define KB_SEARCH_DOCUMENT_STORE_H

#include "kablunk/ingestion/document.h"
#include "kablunk/query/tf_idf_data.h"

// TODO: should be flat map
#include <unordered_map>
#include <tuple>

namespace kb {

struct document_store_specification {
    // Expected document count to reserve space for
    size_t m_expected_document_count = 0;
};

struct document_store {
    inline static constexpr size_t k_expected_term_count = 4192;
    // map uri to document
    std::unordered_map<std::string, document> m_document_map{};
    // cache of (term, document) to term frequency data
    // TODO: can the key be a string view?
    std::unordered_map<std::string, std::unordered_map<std::string, term_frequency_data>> m_term_frequency_cache{};
    // cache of term to relevant document keys (document uri(s))
    // TODO: can the key be a string view?
    std::unordered_map<std::string, std::vector<std::string>> m_term_to_document_cache{};

    static auto create(const document_store_specification& p_specification) noexcept -> document_store;
};

} // end namespace kb

#endif //KB_SEARCH_DOCUMENT_STORE_H
