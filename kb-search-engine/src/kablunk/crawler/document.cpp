//
// Created by happymonkey1 on 5/10/24.
//

#include "kablunk/crawler/document.h"

#include <tidy.h>
#include <tidybuffio.h>
#include <tinyxml2.h>

#include <string_view>
#include <fstream>
#include <stack>
#include <kablunk/crawler/xml_document.h>

namespace kb {

// convert html to xhtml, hopefully fixing errors, so we can parse the document
auto convert_html_to_xhtml(
    const std::string& p_content_buffer
) noexcept -> std::string {
    std::string content_output{};
    TidyBuffer tidy_output{};
    TidyBuffer err_buf{};

    TidyDoc tidy_doc = tidyCreate();

    KB_VERIFY(tidyOptSetBool(tidy_doc, TidyXhtmlOut, yes))
    // TODO: does not work
    // ok = tidyOptSetBool(tidy_doc, TidyBodyOnly, yes);

    // remove unneeded stuff
    KB_VERIFY(tidyOptSetBool(tidy_doc, TidyHideComments, yes))
    KB_VERIFY(tidyOptSetBool(tidy_doc, TidyDropEmptyElems, yes))
    // wrap text in blocks or body in p tags
    KB_VERIFY(tidyOptSetBool(tidy_doc, TidyEncloseBlockText, yes))
    KB_VERIFY(tidyOptSetBool(tidy_doc, TidyEncloseBodyText, yes))
    // try add closing tags
    KB_VERIFY(tidyOptSetBool(tidy_doc, TidyCoerceEndTags, yes))
    // add newline before br
    KB_VERIFY(tidyOptSetBool(tidy_doc, TidyBreakBeforeBR, yes))
    // add quotes to special chars
    KB_VERIFY(tidyOptSetBool(tidy_doc, TidyQuoteNbsp, yes))
    KB_VERIFY(tidyOptSetBool(tidy_doc, TidyQuoteAmpersand, yes))
    // replace special characters (smart quotes, em dash, etc.) with ascii
    KB_VERIFY(tidyOptSetBool(tidy_doc, TidyMakeBare, yes))

    // fix non-compliance
    // TODO: this is not working
    //ok = tidyOptSetBool(tidy_doc, TidyFixComments, yes);
    KB_VERIFY(tidyOptSetBool(tidy_doc, TidyFixBackslash, yes))
    KB_VERIFY(tidyOptSetBool(tidy_doc, TidyFixUri, yes))

    // TODO: this is not working
    //ok = tidyOptSetBool(tidy_doc, TidyMergeDivs, yes);
    //ok = tidyOptSetBool(tidy_doc, TidyMergeSpans, yes);

    // Capture diagnostics
    int rc = tidySetErrorBuffer(tidy_doc, &err_buf);

    if (rc >= 0) {
        // Parse the input
        rc = tidyParseString(tidy_doc, p_content_buffer.c_str());
    }
    if (rc >= 0) {
        // Tidy it up!
        rc = tidyCleanAndRepair(tidy_doc);
    }
    if (rc >= 0) {
        // Kvetch
        rc = tidyRunDiagnostics(tidy_doc);
    }
    if ( rc > 1 ) {
        // If error, force tidy_output.
        KB_CORE_ERROR("[document]: Tidy failed! Still forcing tidy_output")
        KB_CORE_ERROR("[document]:   {}", std::string_view{ reinterpret_cast<char*>(err_buf.bp), err_buf.size })
        rc = (tidyOptSetBool(tidy_doc, TidyForceOutput, yes) ? rc : -1);
    }
    if (rc >= 0) {
        // Pretty Print
        rc = tidySaveBuffer(tidy_doc, &tidy_output);

        // Copy from tidy_output buffer back into the local crawler buffer
        if (rc >= 0) {
            KB_CORE_TRACE("[document]: Tidy succeeded.");
            content_output = std::string{ reinterpret_cast<const char*>(tidy_output.bp), tidy_output.size };
        } else {
            KB_CORE_ERROR(
                "[document]: Tidy failed! Error={}",
                std::string_view{ reinterpret_cast<const char*>(err_buf.bp) }
            )
        }
    } else {
        KB_CORE_ERROR("[document]: Tidy failed!");
    }

    tidyBufFree(&tidy_output);
    tidyBufFree(&err_buf);
    tidyRelease(tidy_doc);

    return content_output;
}

auto parse_xml(const std::string& p_content_buffer) noexcept -> option<std::string> {
    tinyxml2::XMLDocument doc{};
    doc.Parse(p_content_buffer.c_str(), p_content_buffer.size());

    if (doc.Error()) {
        KB_CORE_ERROR("[document]: Tinyxml2 parsing error! {}", doc.ErrorStr());
        return std::nullopt;
    }

    std::stringstream ss{};

    // Parse the xml for known nodes which we actually want to index
    {
        std::stack<tinyxml2::XMLNode*> nodes_to_search{};
        nodes_to_search.emplace(doc.RootElement());

        while (!nodes_to_search.empty()) {
            auto cur = nodes_to_search.top();
            nodes_to_search.pop();

            // try to index relevant data
            const auto tag = std::string_view{ cur->Value() };
            const auto tag_type = tag_str_to_type(tag);

            if (cur->ToText()) {
                const auto text = cur->ToText()->Value();
                if (text) {
                    ss << text;
                    ss << "\t";
                }
            }

            // we do not care about head tag or its children
            if (tag_type == xml_element_tag_type_t::head || tag_type == xml_element_tag_type_t::script) {
                continue;
            }

            // add children
            auto it = cur->FirstChild();
            while (it != nullptr) {
                KB_CORE_ASSERT(it, "how is it null?")
                nodes_to_search.emplace(it);
                it = it->NextSibling();
            }
        }
    }

    return ss.str();
}

auto document::create(
    const std::filesystem::path& p_file_path,
    document_type_t p_original_document_type
) noexcept -> option<document> {
    if (!std::filesystem::exists(p_file_path)) {
        return std::nullopt;
    }

    std::ifstream file{ p_file_path, std::ios::in };
    KB_CORE_ASSERT(file, "[document]: Failed to open '{}'", p_file_path.c_str())
    std::string buffer{};

    // parse file by reading entire file content into memory
    {
        file.seekg(0, std::ios::end);
        buffer.resize(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(&buffer[0], static_cast<i64>(buffer.size()));
    }

    switch (p_original_document_type) {
        case document_type_t::html: {
            buffer = convert_html_to_xhtml(buffer);
        } [[fallthrough]];
        case document_type_t::xhtml: {
            if (buffer.empty()) {
                KB_CORE_WARN("[document]: No data available?");
                return std::nullopt;
            }

            const auto content = parse_xml(buffer);
            if (!content) {
                KB_CORE_ERROR("[document]:   Failed to parse '{}'!", p_file_path.c_str())
                return std::nullopt;
            }

            return document{
                .m_type = p_original_document_type,
                .m_content = *content,
                .m_uri = p_file_path.string(),
            };
        }
        case document_type_t::none: [[fallthrough]];
        default: {
            KB_CORE_WARN(
                "[document]: Unhandled document type '{}'",
                static_cast<std::underlying_type_t<document_type_t>>(p_original_document_type)
            );
            return std::nullopt;
        }
    }
}
} // end namespace kb
