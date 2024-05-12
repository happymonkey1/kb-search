//
// Created by happymonkey1 on 5/11/24.
//

#ifndef KB_SEARCH_XML_DOCUMENT_H
#define KB_SEARCH_XML_DOCUMENT_H

#include <string_view>

namespace kb {

enum class xml_element_tag_type_t {
    none,

    head,
    body,
    meta,
    script,

    a,
    p,
    div,
    span,
    code,
    td,

    END, // internal use only
};

struct xml_element_tags {
    inline static constexpr std::string_view k_p_tag = "p";
    inline static constexpr std::string_view k_span_tag = "span";
    inline static constexpr std::string_view k_code_tag = "code";
    inline static constexpr std::string_view k_td_tag = "td";
    inline static constexpr std::string_view k_head_tag = "head";
    inline static constexpr std::string_view k_body_tag = "body";
    inline static constexpr std::string_view k_div_tag = "div";
    inline static constexpr std::string_view k_a_tag = "a";
    inline static constexpr std::string_view k_meta_tag = "meta";
    inline static constexpr std::string_view k_script_tag = "script";
};

inline auto tag_str_to_type(std::string_view p_tag) noexcept -> xml_element_tag_type_t {
    if (p_tag == xml_element_tags::k_head_tag) {
        return xml_element_tag_type_t::head;
    } else if (p_tag == xml_element_tags::k_body_tag) {
        return xml_element_tag_type_t::body;
    } else if (p_tag == xml_element_tags::k_meta_tag) {
        return xml_element_tag_type_t::meta;
    } else if (p_tag == xml_element_tags::k_script_tag) {
        return xml_element_tag_type_t::script;
    } else if (p_tag == xml_element_tags::k_a_tag) {
        return xml_element_tag_type_t::a;
    } else if (p_tag == xml_element_tags::k_p_tag) {
        return xml_element_tag_type_t::p;
    } else if (p_tag == xml_element_tags::k_div_tag) {
        return xml_element_tag_type_t::div;
    } else if (p_tag == xml_element_tags::k_span_tag) {
        return xml_element_tag_type_t::span;
    } else if (p_tag == xml_element_tags::k_code_tag) {
        return xml_element_tag_type_t::code;
    } else if (p_tag == xml_element_tags::k_td_tag) {
        return xml_element_tag_type_t::td;
    } else {
        return xml_element_tag_type_t::none;
    }
}

inline auto should_tag_be_indexed(xml_element_tag_type_t p_tag_type) noexcept -> bool {
    switch (p_tag_type) {
        case xml_element_tag_type_t::td: [[fallthrough]];
        case xml_element_tag_type_t::a: [[fallthrough]];
        case xml_element_tag_type_t::div: [[fallthrough]];
        case xml_element_tag_type_t::code: [[fallthrough]];
        case xml_element_tag_type_t::p: {
            return true;
        }
        default: {
            return false;
        }
    }
}

inline auto should_tag_be_indexed(std::string_view p_tag) noexcept -> bool {
    return should_tag_be_indexed(tag_str_to_type(p_tag));
}

} // end namespace kb

#endif //KB_SEARCH_XML_DOCUMENT_H
