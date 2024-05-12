//
// Created by happymonkey1 on 5/10/24.
//

#ifndef KB_SEARCH_DOCUMENT_H
#define KB_SEARCH_DOCUMENT_H

#include "kablunk/core/core.h"
#include <string>
#include <string_view>

namespace kb {

enum class document_type_t {
    none,
    html,
    xhtml,
    txt,
    // TODO: extend document types, pdf, txt, etc.

    END, // internal use only
};

struct document_extension {
    inline static constexpr std::string_view k_html = ".html";
    inline static constexpr std::string_view k_xhtml = ".xhtml";
    inline static constexpr std::string_view k_txt = ".txt";
};

inline auto document_extension_str_to_type(std::string_view p_extension) noexcept -> document_type_t {
    if (p_extension == document_extension::k_html) {
        return document_type_t::html;
    } else if (p_extension == document_extension::k_xhtml) {
        return document_type_t::xhtml;
    } else  if (p_extension == document_extension::k_txt) {
        return document_type_t::txt;
    } else {
        return document_type_t::none;
    }
}

inline auto is_document_parsing_implemented(
    document_type_t p_document_type
) noexcept -> bool {
    switch (p_document_type) {
        case document_type_t::html: return true;
        case document_type_t::xhtml: return true;
        case document_type_t::txt: return false;
        case document_type_t::none: return false;
        default: {
            KB_CORE_ASSERT(
                false,
                "[document]: Unhandled document_type_t {} in is_document_parsing_implemented!",
                static_cast<std::underlying_type_t<document_type_t>>(p_document_type)
            )
            return false;
        };
    }
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
    static auto create(
        const std::filesystem::path& p_file_path,
        document_type_t p_original_document_type
    ) noexcept -> option<document>;
};

} // end namespace kb

#endif //KB_SEARCH_DOCUMENT_H
