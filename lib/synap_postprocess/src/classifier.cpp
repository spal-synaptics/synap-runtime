// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.
#include "synap/classifier.hpp"

#include "synap/logging.hpp"
#include "synap/timer.hpp"
#include "synap/network.hpp"
#include "synap/string_utils.hpp"

#include <algorithm>
#include <numeric>

using namespace std;


namespace synaptics {
namespace synap {


/// Utility function to get highest elements in an array.
/// 
/// @param confidence: confidence array
/// @param count: number of elements in confidence[]
/// @param _top_count: number of top elements to get
/// @return vector containing sorted indexes of highest _top_count elements in confidence[]
static vector<int> get_top_n(const float* confidence, size_t count, size_t _top_count)
{
    vector<int> vix(count);
    iota(begin(vix), end(vix), 0);
    size_t n = _top_count ? min(count, _top_count) : count;
    partial_sort(begin(vix), begin(vix) + n, end(vix),
                 [confidence](int a, int b) -> bool { return confidence[a] > confidence[b]; });
    return vector<int>(begin(vix), begin(vix) + n);
}


Classifier::Result Classifier::process(const Tensors& tensors)
{
    if (tensors.size() != 1) {
        LOGE << "One tensor expected";
        return {};
    }

    Timer tmr;
    const float* confidence = tensors[0].as_float();
    auto confidence_size = tensors[0].item_count();
    if (!confidence) {
        LOGE << "Tensor data not available";
        return {};
    }

    Result result;
    result.success = true;
    
    // Get base class index to be used.
    // This allows to normalize the output of a classifier network which has been trained
    // with a subset or with additional background/unrecognized class in confidence[0].
    auto index_base = format_parse::get_int(tensors[0].format(), "class_index_base", 0);
    if (_top_count == 1) {
        // Return only top-most classification using fastest algorithms
        int index = distance(confidence, max_element(confidence, &confidence[confidence_size]));
        result.items.emplace_back(Result::Item{index + index_base, confidence[index]});
    }
    else {
        // General case
        vector<int> top = get_top_n(confidence, confidence_size, _top_count);
        result.items.reserve(_top_count);
        for (auto index: top) {
            result.items.emplace_back(Result::Item{index + index_base, confidence[index]});
        }
    }

    LOGV << "Post-processing time: " << tmr;
    return result;
}


}  // namespace synap
}  // namespace synaptics
