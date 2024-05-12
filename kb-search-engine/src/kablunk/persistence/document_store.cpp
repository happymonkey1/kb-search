//
// Created by happymonkey1 on 5/11/24.
//

#include "kablunk/persistence/document_store.h"

namespace kb {

auto document_store::create(const document_store_specification &p_specification) noexcept -> document_store {
    document_store new_document_store{};
    new_document_store.m_document_map.reserve(p_specification.m_expected_document_count);
    new_document_store.m_term_frequency_cache.reserve(k_expected_term_count);
    new_document_store.m_term_to_document_cache.reserve(k_expected_term_count);

    return new_document_store;
}
} // end namespace kb
