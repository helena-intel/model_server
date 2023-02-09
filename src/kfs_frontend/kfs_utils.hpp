//*****************************************************************************
// Copyright 2022 Intel Corporation
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
#include <string>

#include "../precision.hpp"
#include "kfs_grpc_inference_service.hpp"

namespace ovms {
class Status;
class TensorInfo;
std::string tensorShapeToString(const KFSShapeType& tensorShape);

Precision KFSPrecisionToOvmsPrecision(const KFSDataType& s);
const KFSDataType& ovmsPrecisionToKFSPrecision(Precision precision);

size_t KFSDataTypeSize(const KFSDataType& datatype);
Status prepareConsolidatedTensorImpl(KFSResponse* response, const std::string& name, ov::element::Type_t precision, const ov::Shape& shape, char*& tensorOut, size_t size);
const std::string& getRequestServableName(const KFSRequest& request);
Status isNativeFileFormatUsed(const KFSRequest& request, const std::string& name, bool& nativeFileFormatUsed);
bool isNativeFileFormatUsed(const KFSTensorInputProto& proto);
bool isStringFormatUsed(const KFSTensorInputProto& proto, const TensorInfo& tensorInfo);
}  // namespace ovms
