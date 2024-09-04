#!/bin/bash
#
# Copyright 2024 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

if [ -d "$1/facebook/opt-125m" ]; then
  echo "Models directory $1 exists. Skipping downloading models."
  exit 0
fi

echo "Downloading LLM testing models to directory $1"
export PIP_EXTRA_INDEX_URL="https://download.pytorch.org/whl/cpu"
if [ "$2" = "docker" ]; then
    sed -i '/openvino~=/d' /openvino_tokenizers/pyproject.toml
    python3 -m pip wheel -v --no-deps --wheel-dir wheel /openvino_tokenizers
    python3 -m pip install $(find wheel -name 'openvino_tokenizers*.whl')
    python3 -m pip install "optimum-intel"@git+https://github.com/huggingface/optimum-intel.git nncf
else
    python3 -m venv .venv
    . .venv/bin/activate
    pip3 install -U pip
    pip3 install --pre "optimum-intel[nncf,openvino]"@git+https://github.com/huggingface/optimum-intel.git openvino-tokenizers
fi

optimum-cli export openvino --disable-convert-tokenizer --model facebook/opt-125m --weight-format int8 $1/facebook/opt-125m
convert_tokenizer -o $1/facebook/opt-125m --with-detokenizer --skip-special-tokens --streaming-detokenizer --not-add-special-tokens facebook/opt-125m

