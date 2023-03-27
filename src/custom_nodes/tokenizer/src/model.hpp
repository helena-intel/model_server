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
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace custom_nodes {
namespace tokenizer {

class BlingFireModel {
    int id;
    void* handle = nullptr;
    bool debug;

public:
    BlingFireModel(const std::string& modelPath, bool debug = false);
    ~BlingFireModel();

    std::vector<int64_t> tokenize(const std::string& text, int maxIdsArrLength);
    std::string detokenize(const std::vector<int64_t>& tokens, int maxBufferLength, bool skipSpecialTokens = false);
};

}  // namespace tokenizer
}  // namespace custom_nodes
