//
// Created by happymonkey1 on 5/11/24.
//

#ifndef KB_SEARCH_TF_IDF_DATA_H
#define KB_SEARCH_TF_IDF_DATA_H

#include "kablunk/core/core.h"
#include <string>

namespace kb {

struct term_frequency_data {
    // term
    // TODO: can this be a string view?
    std::string m_term{};
    // key (or uri) to the document
    std::string_view m_document_key{};
    // raw count of the frequency of the term in the document
    u64 m_term_occurrence = 0;
    // total number of tokens in the document
    u64 m_total_token_count = 0;
    // computed tf score
    f64 m_tf_score = 0.;
    // computed idf score
    f64 m_idf_score = 0.;
    // compute tf idf score
    f64 m_tf_idf_score = 0.f;
    // flag for whether the data is stale
    bool m_stale = false;
};

} // end namespace kb

#endif //KB_SEARCH_TF_IDF_DATA_H
