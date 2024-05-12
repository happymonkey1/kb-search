//
// Created by happymonkey1 on 5/10/24.
//

#include "kablunk/ingestion/document.h"
#include "kablunk/core/owning_buffer.h"
#include "kablunk/persistence/document_store_accessor.h"
#include "kablunk/ingestion/xml_document.h"

#include <tidy.h>
#include <tidybuffio.h>
#include <tinyxml2.h>

#include <string_view>
#include <fstream>
#include <stack>

namespace {
// initially allocate 4KB
constexpr size_t k_initial_file_buffer_size = 4 * 1024;
// statically allocated storage to try and minimize the amount of allocations / destructions
// while reading files
inline kb::owning_buffer<char> s_read_buffer{ k_initial_file_buffer_size };
} // end anonymous namespace

namespace kb {

// convert html to xhtml, hopefully fixing errors, so we can parse the document
auto convert_html_to_xhtml() noexcept -> void {
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
        rc = tidyParseString(tidy_doc, s_read_buffer.data());
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

        // Copy from tidy_output buffer back into the local ingestion buffer
        if (rc >= 0) {
            KB_CORE_TRACE("[document]: Tidy succeeded.");
            s_read_buffer.copy_into(reinterpret_cast<const char*>(tidy_output.bp), tidy_output.size);
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
}

auto parse_xml() noexcept -> option<std::string> {
    tinyxml2::XMLDocument doc{};
    doc.Parse(s_read_buffer.data(), s_read_buffer.size_in_bytes());

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
                    ss << " ";
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
    document_type_t p_original_document_type,
    document_store_accessor &p_document_store_accessor
) noexcept -> option<document> {
    if (!std::filesystem::exists(p_file_path)) {
        return std::nullopt;
    }

    std::ifstream file{ p_file_path, std::ios::in };
    KB_CORE_ASSERT(file, "[document]: Failed to open '{}'", p_file_path.c_str())

    // parse file by reading entire file content into memory
    {
        file.seekg(0, std::ios::end);
        const auto file_size = file.tellg();
        if (static_cast<u64>(file_size) + 1ull > s_read_buffer.allocated_size_in_bytes()) {
            s_read_buffer.resize_discard_ok(static_cast<u64>(file_size) + 1ull);
        }
        file.seekg(0, std::ios::beg);
        s_read_buffer.zero();
        file.read(s_read_buffer.data(), static_cast<i64>(file_size));
    }

    std::string content_buffer{};
    switch (p_original_document_type) {
        case document_type_t::html: {
            convert_html_to_xhtml();
        } [[fallthrough]];
        case document_type_t::xhtml: {
            if (s_read_buffer.empty()) {
                KB_CORE_WARN("[document]: No data available?");
                return std::nullopt;
            }

            const auto content = parse_xml();
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

auto details::release_document_read_buffer() noexcept -> void {
    s_read_buffer.release();
}
} // end namespace kb
