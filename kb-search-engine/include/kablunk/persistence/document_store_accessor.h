//
// Created by happymonkey1 on 5/11/24.
//

#ifndef KB_SEARCH_DOCUMENT_STORE_ACCESSOR_H
#define KB_SEARCH_DOCUMENT_STORE_ACCESSOR_H

#include "kablunk/persistence/document_store.h"
#include "kablunk/ingestion/document.h"

#include <string>

namespace kb {

class document_store_accessor{
public:
    document_store_accessor() noexcept = default;
    document_store_accessor(const document_store_accessor&) noexcept = default;
    document_store_accessor(document_store_accessor&&) noexcept = default;
    ~document_store_accessor() noexcept = default;

    auto emplace_document(
        std::string&& p_key,
        document&& p_document
    ) noexcept -> void;

    auto put_document(
        const std::string& p_key,
        const document& p_document
    ) noexcept -> void;

    [[nodiscard]] auto contains_document(const std::string& p_key) const noexcept -> bool;

    [[nodiscard]] auto get_document(const std::string& p_key) const noexcept -> const document&;
    [[nodiscard]] auto get_all_documents(const std::string& p_key) const noexcept -> std::vector<document const*>;
    [[nodiscard]] auto get_document_count() const noexcept -> size_t;

    auto operator=(const document_store_accessor&) noexcept -> document_store_accessor& = default;
    auto operator=(document_store_accessor&&) noexcept -> document_store_accessor& = default;

    [[nodiscard]] static auto create(document_store* p_document_store_ptr) noexcept -> document_store_accessor;

    [[nodiscard]] auto has_relevant_documents_for_term(const std::string& p_term) const noexcept -> bool;
    // return a list of document keys (uris) whose content contains the given term
    [[nodiscard]] auto get_relevant_document_keys_by_term(
        const std::string& p_term
    ) const noexcept -> option<std::reference_wrapper<const std::vector<std::string>>>;
    // returns the number of documents whose content contains the given term
    [[nodiscard]] auto get_relevant_document_count_for_term(
        const std::string &p_term
    ) const noexcept -> size_t;

    // TODO: enum to switch between whether stale data is ok
    [[nodiscard]] auto has_tf_idf_score(
        const std::string &p_term,
        const std::string &p_document_key
    ) const noexcept -> bool;
    [[nodiscard]] auto get_tf_idf_score(
        const std::string &p_term,
        const std::string &p_document_key
    ) const noexcept -> option<f64>;

    // check whether the term frequency cache contains data for the given (term, document) keys
    [[nodiscard]] auto has_term_frequency_data(
        const std::string &p_term,
        const std::string &p_document_key
    ) const noexcept -> bool;

    // try return cached term frequency data for the (term, document) keys
    [[nodiscard]] auto get_term_frequency_data(
        const std::string& p_term,
        const std::string &p_document_key
    ) const noexcept -> option<term_frequency_data const*>;

    // try return cached term frequency data for the term
    // returns tf data for all the relevant documents
    [[nodiscard]] auto get_relevant_term_frequency_data(
        const std::string &p_term
    ) const noexcept -> std::vector<term_frequency_data const*>;

    // emplace term frequency data for the (term, document) keys
    auto emplace_term_frequency_data_for_document(
        const std::string& p_term,
        const std::string& p_document_key,
        term_frequency_data&& p_tf_idf_data
    ) noexcept -> void;

    auto compute_tf_idf(
        const std::string& p_term
    ) noexcept -> void;
private:
    explicit document_store_accessor(document_store* p_document_store_ptr) noexcept;

    auto compute_tf_score_for_document(
        const std::string& p_term,
        const document& p_document,
        const size_t p_document_count
    ) noexcept -> void;
private:
    // non-owning pointer to the store
    document_store *m_document_store_ptr = nullptr;
};

} // end namespace kb

#endif //KB_SEARCH_DOCUMENT_STORE_ACCESSOR_H
