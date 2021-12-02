//*****************************************************************************
// Copyright 2021 Intel Corporation
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
#include "gathernodeinputhandler.hpp"

#include <functional>

#include "logging.hpp"
#include "nodesessionmetadata.hpp"
#include "ov_utils.hpp"
#include "tensorinfo.hpp"

namespace ovms {

GatherNodeInputHandler::GatherNodeInputHandler(uint32_t inputsMissingCount, const CollapseDetails& collapsingDetails) :
    NodeInputHandler(inputsMissingCount),
    collapsingDetails(std::make_unique<CollapseDetails>(collapsingDetails)) {
    remainingDependencies = std::accumulate(
        collapsingDetails.collapsedSessionSizes.begin(),
        collapsingDetails.collapsedSessionSizes.end(),
        remainingDependencies,
        std::multiplies<session_id_t>());
}

Status GatherNodeInputHandler::setInput(const std::string& inputName, std::shared_ptr<ov::runtime::Tensor>& ptr, session_id_t shardId) {
    auto inputsShardsIt = shardsStorage.find(inputName);
    if (inputsShardsIt == shardsStorage.end()) {
        shard_map_t shardMap{{shardId, ptr}};
        auto itDidInsertPair = shardsStorage.emplace(inputName, std::move(shardMap));
        if (!itDidInsertPair.second) {
            SPDLOG_LOGGER_ERROR(dag_executor_logger, "Tried to insert the same input: {} twice with the same shardId: {}", inputName, shardId);
            return StatusCode::INTERNAL_ERROR;
        }
    } else {
        auto itDidEmplacePair = inputsShardsIt->second.emplace(shardId, ptr);
        if (!itDidEmplacePair.second) {
            SPDLOG_LOGGER_ERROR(dag_executor_logger, "Tried to put the same input: {} shard: {} twice", inputName, shardId);
            return StatusCode::INTERNAL_ERROR;
        }
    }
    return StatusCode::OK;
}

Status GatherNodeInputHandler::notifyFinishedDependency() {
    NodeInputHandler::notifyFinishedDependency();
    if (remainingDependencies > 0) {
        return StatusCode::OK;
    }
    for (auto& [inputName, shardMap] : shardsStorage) {
        const auto shardsCount = shardMap.size();
        SPDLOG_LOGGER_DEBUG(dag_executor_logger, "Consolidating: {} shards for input: {}", shardsCount, inputName);
        session_id_t firstShardId = 0;
        auto firstShard = shardMap.at(firstShardId);
        auto shardDims = firstShard->get_shape();
        auto precision = firstShard->get_element_type();
        auto newDims = shardDims;
        newDims.insert(newDims.begin(),
            collapsingDetails->collapsedSessionSizes.begin(),
            collapsingDetails->collapsedSessionSizes.end());
        std::shared_ptr<ov::runtime::Tensor> consolidatedTensor;
        auto status = createSharedTensor(consolidatedTensor, precision, newDims);
        if (!status.ok()) {
            return status;
        }
        for (auto& [shardId, tensor] : shardMap) {
            if ((tensor->get_element_type() != precision) ||
                (tensor->get_shape() != shardDims)) {
                std::stringstream firstShardShapeStream;
                std::copy(shardDims.begin(), shardDims.end(), std::ostream_iterator<size_t>(firstShardShapeStream, " "));
                auto currentShardShape = tensor->get_shape();
                std::stringstream currentShardShapeStream;
                std::copy(currentShardShape.begin(), currentShardShape.end(), std::ostream_iterator<size_t>(currentShardShapeStream, " "));
                SPDLOG_LOGGER_ERROR(dag_executor_logger, "Failed to consolidate tensor: {}; shards in gather node. First shard has different tensor precision:{};  or shape: {}; than current shard precision: {}; shape:{};",
                    inputName,
                    toString(ovElementTypeToOvmsPrecision(precision)),
                    firstShardShapeStream.str(),
                    toString(ovElementTypeToOvmsPrecision(tensor->get_element_type())),
                    currentShardShapeStream.str()); // TODO simplify double conversion?
                return StatusCode::PIPELINE_INCONSISTENT_SHARD_DIMENSIONS;
            }
            const auto memstep = tensor->get_byte_size();
            size_t offset = shardId * memstep;
            memcpy((char*)consolidatedTensor->data() + offset,
                tensor->data(),
                memstep);
        }
        inputTensors.insert({inputName, consolidatedTensor});
    }
    return StatusCode::OK;
}
}  // namespace ovms
