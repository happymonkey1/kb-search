//
// Created by happymonkey1 on 5/11/24.
//

#include "kablunk/persistence/document_store_accessor.h"

namespace kb {

auto document_store_accessor::emplace_document(std::string &&p_key, document &&p_document) noexcept -> void {
    KB_CORE_ASSERT(m_document_store_ptr, "[document_store_accessor]: document_store is null!");
    m_document_store_ptr->m_document_map.emplace(p_key, p_document);
}

auto document_store_accessor::put_document(const std::string &p_key, const document &p_document) noexcept -> void {
    KB_CORE_ASSERT(m_document_store_ptr, "[document_store_accessor]: document_store is null!");
    m_document_store_ptr->m_document_map.emplace(p_key, p_document);
}

auto document_store_accessor::contains_document(const std::string &p_key) const noexcept -> bool {
    KB_CORE_ASSERT(m_document_store_ptr, "[document_store_accessor]: document_store is null!");
    return m_document_store_ptr->m_document_map.contains(p_key);
}

auto document_store_accessor::get_document(const std::string &p_key) const noexcept -> const document & {
    KB_CORE_ASSERT(m_document_store_ptr, "[document_store_accessor]: document_store is null!");
    return m_document_store_ptr->m_document_map.at(p_key);
}

auto document_store_accessor::create(document_store *p_document_store_ptr) noexcept -> document_store_accessor {
    KB_CORE_ASSERT(p_document_store_ptr, "[document_store_accessor]: document store can not be null!");
    return document_store_accessor{ p_document_store_ptr };
}

document_store_accessor::document_store_accessor(document_store *p_document_store_ptr) noexcept
    : m_document_store_ptr{ p_document_store_ptr } {

}

auto document_store_accessor::has_relevant_documents_for_term(const std::string &p_term) const noexcept -> bool {
    KB_CORE_ASSERT(m_document_store_ptr, "[document_store_accessor]: document_store is null!");
    return m_document_store_ptr->m_term_to_document_cache.contains(p_term);
}

auto document_store_accessor::get_relevant_document_keys_by_term(
    const std::string &p_term
) const noexcept -> option<std::reference_wrapper<const std::vector<std::string>>> {
    KB_CORE_ASSERT(m_document_store_ptr, "[document_store_accessor]: document_store is null!");
    if (!m_document_store_ptr->m_term_to_document_cache.contains(p_term)) {
        return std::nullopt;
    }

    return std::cref(m_document_store_ptr->m_term_to_document_cache.at(p_term));
}

auto document_store_accessor::has_tf_idf_score(
    const std::string &p_term,
    const std::string &p_document_key
) const noexcept -> bool {
    KB_CORE_ASSERT(m_document_store_ptr, "[document_store_accessor]: document_store is null!");
    return m_document_store_ptr->m_term_frequency_cache.contains(p_term) ?
        m_document_store_ptr->m_term_frequency_cache.at(p_term).contains(p_document_key) :
        false;
}

auto document_store_accessor::get_tf_idf_score(
    const std::string &p_term,
    const std::string &p_document_key
) const noexcept -> option<f64> {
    KB_CORE_ASSERT(m_document_store_ptr, "[document_store_accessor]: document_store is null!");
    return has_tf_idf_score(p_term, p_document_key) ?
        std::make_optional(m_document_store_ptr->m_term_frequency_cache.at(p_term).at(p_document_key).m_tf_idf_score) :
        std::nullopt;
}

auto document_store_accessor::get_all_documents(const std::string &p_key) const noexcept -> std::vector<document const*> {
    KB_CORE_ASSERT(m_document_store_ptr, "[document_store_accessor]: document_store is null!");

    // TODO: need to valgrind, can not remember if this results in UB
    //       because memory is not stable in the map
    std::vector<document const*> docs{};
    docs.reserve(m_document_store_ptr->m_document_map.size());
    for (const auto& [_, doc] : m_document_store_ptr->m_document_map) {
        docs.push_back(&doc);
    }

    return docs;
}

auto document_store_accessor::emplace_term_frequency_data_for_document(
    const std::string &p_term,
    const std::string &p_document_key,
    term_frequency_data &&p_tf_idf_data
) noexcept -> void {
    KB_CORE_ASSERT(m_document_store_ptr, "[document_store_accessor]: document_store is null!");

    // if we do not have a document map, reserve some space and emplace the new map with the new key value pair
    // otherwise, just emplace the new key value pair.
    if (m_document_store_ptr->m_term_frequency_cache.contains(p_term)) {
        m_document_store_ptr->m_term_frequency_cache.at(p_term).emplace(
            p_document_key,
            std::move(p_tf_idf_data)
        );
    } else {
        auto new_term_frequency_map = std::unordered_map<std::string, term_frequency_data>{};
        new_term_frequency_map.reserve(document_store::k_expected_term_count);
        m_document_store_ptr->m_term_frequency_cache.emplace(
            p_term,
            new_term_frequency_map
        );
        m_document_store_ptr->m_term_frequency_cache.at(p_term).emplace(p_document_key, std::move(p_tf_idf_data));
    }
}

auto document_store_accessor::has_term_frequency_data(
    const std::string &p_term,
    const std::string &p_document_key
) const noexcept -> bool {
    KB_CORE_ASSERT(m_document_store_ptr, "[document_store_accessor]: document_store is null!");

    return m_document_store_ptr->m_term_frequency_cache.contains(p_term) ?
        m_document_store_ptr->m_term_frequency_cache.at(p_term).contains(p_document_key) :
        false;
}

auto document_store_accessor::get_term_frequency_data(
    const std::string &p_term,
    const std::string &p_document_key
) const noexcept -> option<term_frequency_data const*> {
    KB_CORE_ASSERT(m_document_store_ptr, "[document_store_accessor]: document_store is null!");

    return has_term_frequency_data(p_term, p_document_key) ?
        std::make_optional(&m_document_store_ptr->m_term_frequency_cache.at(p_term).at(p_document_key)) :
        std::nullopt;
}

auto document_store_accessor::get_relevant_term_frequency_data(
    const std::string &p_term) const noexcept -> std::vector<term_frequency_data const *> {
    KB_CORE_ASSERT(m_document_store_ptr, "[document_store_accessor]: document_store is null!");
    if (!m_document_store_ptr->m_term_frequency_cache.contains(p_term)) {
        return {};
    }

    std::vector<term_frequency_data const*> tf_data_list{};
    const auto& tf_data_map = m_document_store_ptr->m_term_frequency_cache.at(p_term);
    tf_data_list.reserve(tf_data_map.size());

    for (const auto& [_, tf_data] : tf_data_map) {
        tf_data_list.emplace_back(&tf_data);
    }

    return tf_data_list;
}

auto document_store_accessor::get_document_count() const noexcept -> size_t {
    KB_CORE_ASSERT(m_document_store_ptr, "[document_store_accessor]: document_store is null!");
    return m_document_store_ptr->m_document_map.size();
}

auto document_store_accessor::get_relevant_document_count_for_term(const std::string &p_term) const noexcept -> size_t {
    KB_CORE_ASSERT(m_document_store_ptr, "[document_store_accessor]: document_store is null!");
    return m_document_store_ptr->m_term_to_document_cache.contains(p_term) ?
        m_document_store_ptr->m_term_to_document_cache.at(p_term).size() :
        0ull;
}

auto document_store_accessor::compute_tf_idf(const std::string &p_term) noexcept -> void {
    const auto document_count = get_document_count();
    const auto doc_keys = get_relevant_document_keys_by_term(p_term);
    if (doc_keys) {
        for (const auto& doc_key : doc_keys->get()) {
            compute_tf_score_for_document(
                p_term,
                get_document(doc_key),
                document_count
            );
        }
    } else {
        KB_CORE_WARN("[compute_tf_idf]: Did not find any docs for term '{}'", p_term)

        const auto all_docs = get_all_documents(p_term);
        KB_CORE_INFO("[compute_tf_idf]: Found {} documents", all_docs.size())
        for (const auto& doc : all_docs) {
            compute_tf_score_for_document(p_term, *doc, document_count);
        }
    }
}

/*
    tf(t, d) = f[t, d] / sum( f[t', d] )
        where t = term
        f[t, d] is the frequency (or raw count) of the term in a document d
        f[t', d] is simply the total number of terms in a document d

    idf(t, D) = log(N / (d in D, t in D)
        where t = term
        N is the total number of documents in the corpus (N = |D|)
        d in D, t in d is the number of documents where the term t appears

    tfidf(t, d, D) = tf(t, d) * idf(t, D)
*/
auto document_store_accessor::compute_tf_score_for_document(
    const std::string &p_term,
    const document &p_document,
    const size_t p_document_count
) noexcept -> void {
    if (const auto tf_data = get_term_frequency_data(p_term, p_document.m_uri);
            tf_data && !(*tf_data)->m_stale) {
        KB_CORE_INFO("[compute_tf_score_for_document]: Found cached tf score for {}, {}", p_term, p_document.m_uri)
        return;
    }

    std::stringstream ss{ p_document.m_content };

    // TODO: this should probably be moved to where we originally parse documents...
    // split document content into tokens
    u64 token_count = 0;
    u64 occurrence = 0;
    std::string token{};
    while (std::getline(ss, token, ' ')) {
        ++token_count;
        if (token == p_term) {
            ++occurrence;
        }
    }

    const f64 tf_score = static_cast<f64>(occurrence) / static_cast<f64>(token_count);
    const f64 idf_score = std::log((1 + static_cast<f64>(p_document_count)) /
        (1. + static_cast<f64>(get_relevant_document_count_for_term(p_term)))
    );
    KB_CORE_INFO(
        "[compute_tf_score_for_document]: token_count={}, occurrence={}, tf_score={}, idf_score={}, tf_idf_score={}",
        token_count,
        occurrence,
        tf_score,
        idf_score,
        tf_score * idf_score
    )

    emplace_term_frequency_data_for_document(
        p_term,
        p_document.m_uri,
        term_frequency_data{
            .m_term = p_term,
            .m_document_key = p_document.m_uri,
            .m_term_occurrence = occurrence,
            .m_total_token_count = token_count,
            .m_tf_score = tf_score,
            .m_idf_score = idf_score,
            .m_tf_idf_score = tf_score * idf_score,
            .m_stale = false,
        }
    );
}


} // end namespace kb