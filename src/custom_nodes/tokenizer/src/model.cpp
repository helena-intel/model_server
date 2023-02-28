//*****************************************************************************
// Copyright 2023 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//*****************************************************************************
#include "model.hpp"

#include <iostream>
#include <string>
#include <atomic>
#include <memory>

#include "blingfiretokdll.h"

namespace custom_nodes {
namespace tokenizer {

static std::atomic<int> maxId{0};

BlingFireModel::BlingFireModel(const std::string& modelPath) : id(maxId++) {
    handle = BlingFire::LoadModel(modelPath.c_str());
    std::cout << "[tokenizer] [" << id << "] Model loaded from: " << modelPath << std::endl;
}

BlingFireModel::~BlingFireModel() {
    if (handle) {
        BlingFire::FreeModel(handle);
        std::cout << "[tokenizer] [" << id << "] Model unloaded." << std::endl;
    }
}

std::vector<int64_t> BlingFireModel::tokenize(const std::string& text, int maxIdsArrLength) {
    auto ids = std::make_unique<int32_t[]>(maxIdsArrLength);
    const int idsLength = BlingFire::TextToIds(handle, text.c_str(), text.size(), ids.get(), maxIdsArrLength);
    std::vector<int64_t> vec(idsLength);
    std::transform(ids.get(), ids.get() + idsLength, vec.begin(),
        [](int32_t val) { return static_cast<int64_t>(val); });
    return std::move(vec);
}

std::string BlingFireModel::detokenize(const std::vector<int64_t>& tokens, int maxBufferLength, bool skipSpecialTokens) {
    auto ids = std::make_unique<int32_t[]>(tokens.size());
    std::transform(tokens.begin(), tokens.end(), ids.get(),
        [](int64_t val) { return static_cast<int32_t>(val); });
    std::string str(maxBufferLength + 1, '\0'); // +1 due to null ending
    const int strLength = BlingFire::IdsToText(handle, ids.get(), tokens.size(), str.data(), maxBufferLength, skipSpecialTokens);
    str.resize(strLength - 1);  // remove null terminator
    return std::move(str);
}

}  // namespace tokenizer
}  // namespace custom_nodes
