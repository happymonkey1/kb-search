//
// Created by happymonkey1 on 5/10/24.
//

#ifndef KB_SEARCH_DOCUMENT_H
#define KB_SEARCH_DOCUMENT_H

#include "kablunk/core/core.h"
#include <string>

namespace kb {

enum class document_type_t {
    none,
    html,
    // TODO: extend document types, pdf, txt, etc.

    END, // internal use only
};

struct document_extension {
    inline static constexpr std::string_view k_html = ".html";
    inline static constexpr std::string_view k_xhtml = ".xhtml";
    inline static constexpr std::string_view k_txt = ".txt";
};

inline auto is_document_parsing_implemented(
    const std::filesystem::path& p_file_path
) noexcept -> bool {
    const auto ext_path = p_file_path.extension();
    const auto ext = std::string_view{ ext_path.c_str() };

    if (ext == document_extension::k_html || ext == document_extension::k_xhtml) {
        return true;
    }

    return false;
}

struct document
{
    // type of the document
    document_type_t m_type = document_type_t::none;
    // buffer for the content
    std::string m_content{};
    // uri
    std::string m_uri{};

    // factory function to create a document from a path to a local file
    static auto create(const std::filesystem::path& p_file_path) noexcept -> option<document>;
};

} // end namespace kb

#endif //KB_SEARCH_DOCUMENT_H
