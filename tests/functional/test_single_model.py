#
# Copyright (c) 2018-2019 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import numpy as np
import pytest

from constants import MODEL_SERVICE, ERROR_SHAPE
from model.models_information import Resnet
from utils.grpc import create_channel, infer, get_model_metadata, model_metadata_response, \
    get_model_status
from utils.models_utils import ModelVersionState, ErrorCode, \
    ERROR_MESSAGE  # noqa
from utils.rest import infer_rest, get_model_metadata_response_rest, \
    get_model_status_response_rest


class TestSingleModelInference:

    def test_run_inference(self, resnet_multiple_batch_sizes,
                           start_server_single_model):
        """
        <b>Description</b>
        Submit request to gRPC interface serving a single resnet model

        <b>input data</b>
        - directory with the model in IR format
        - docker image with ie-serving-py service

        <b>fixtures used</b>
        - model downloader
        - service launching

        <b>Expected results</b>
        - response contains proper numpy shape

        """

        _, ports = start_server_single_model
        print("Downloaded model files:", resnet_multiple_batch_sizes)

        # Connect to grpc service
        stub = create_channel(port=ports["grpc_port"])

        imgs_v1_224 = np.ones(Resnet.input_shape, Resnet.dtype)
        output = infer(imgs_v1_224, input_tensor=Resnet.input_name, grpc_stub=stub,
                       model_spec_name=Resnet.name,
                       model_spec_version=None,
                       output_tensors=[Resnet.output_name])
        print("output shape", output[Resnet.output_name].shape)
        assert output[Resnet.output_name].shape == Resnet.output_shape, ERROR_SHAPE

    def test_get_model_metadata(self, resnet_multiple_batch_sizes,
                                start_server_single_model):

        _, ports = start_server_single_model
        print("Downloaded model files:", resnet_multiple_batch_sizes)
        stub = create_channel(port=ports["grpc_port"])

        expected_input_metadata = {Resnet.input_name: {'dtype': 1, 'shape': list(Resnet.input_shape)}}
        expected_output_metadata = {Resnet.output_name: {'dtype': 1, 'shape': list(Resnet.output_shape)}}
        request = get_model_metadata(model_name=Resnet.name)
        response = stub.GetModelMetadata(request, 10)
        input_metadata, output_metadata = model_metadata_response(
            response=response)
        print(output_metadata)
        assert response.model_spec.name == Resnet.name
        assert expected_input_metadata == input_metadata
        assert expected_output_metadata == output_metadata

    def test_get_model_status(self, resnet_multiple_batch_sizes,
                              start_server_single_model):

        print("Downloaded model files:", resnet_multiple_batch_sizes)

        _, ports = start_server_single_model
        stub = create_channel(port=ports["grpc_port"], service=MODEL_SERVICE)
        request = get_model_status(model_name=Resnet.name)
        response = stub.GetModelStatus(request, 10)
        versions_statuses = response.model_version_status
        version_status = versions_statuses[0]
        assert version_status.version == 1
        assert version_status.state == ModelVersionState.AVAILABLE
        assert version_status.status.error_code == ErrorCode.OK
        assert version_status.status.error_message == ERROR_MESSAGE[
            ModelVersionState.AVAILABLE][ErrorCode.OK]

    @pytest.mark.skip(reason="not implemented yet")
    @pytest.mark.parametrize("request_format",
                             ['row_name', 'row_noname', 'column_name', 'column_noname'])
    def test_run_inference_rest(self, resnet_multiple_batch_sizes,
                                start_server_single_model,
                                request_format):
        """
        <b>Description</b>
        Submit request to REST API interface serving a single resnet model

        <b>input data</b>
        - directory with the model in IR format
        - docker image with ie-serving-py service

        <b>fixtures used</b>
        - model downloader
        - service launching

        <b>Expected results</b>
        - response contains proper numpy shape

        """

        print("Downloaded model files:", resnet_multiple_batch_sizes)

        _, ports = start_server_single_model
        imgs_v1_224 = np.ones(Resnet.input_shape, Resnet.dtype)
        rest_url = 'http://localhost:{}/v1/models/resnet:predict'.format(ports["rest_port"])
        output = infer_rest(imgs_v1_224, input_tensor=Resnet.input_name,
                            rest_url=rest_url,
                            output_tensors=[Resnet.output_name],
                            request_format=request_format)
        print("output shape", output[Resnet.output_name].shape)
        assert output[Resnet.output_name].shape == Resnet.output_shape, ERROR_SHAPE

    @pytest.mark.skip(reason="not implemented yet")
    def test_get_model_metadata_rest(self, resnet_multiple_batch_sizes,
                                     start_server_single_model):
        print("Downloaded model files:", resnet_multiple_batch_sizes)

        _, ports = start_server_single_model
        expected_input_metadata = {Resnet.input_name: {'dtype': 1, 'shape': list(Resnet.input_shape)}}
        expected_output_metadata = {Resnet.output_name: {'dtype': 1, 'shape': list(Resnet.output_shape)}}
        rest_url = 'http://localhost:{}/v1/models/resnet/metadata'.format(ports["rest_port"])
        response = get_model_metadata_response_rest(rest_url)
        input_metadata, output_metadata = model_metadata_response(
            response=response)
        print(output_metadata)
        assert response.model_spec.name == Resnet.name
        assert expected_input_metadata == input_metadata
        assert expected_output_metadata == output_metadata

    @pytest.mark.skip(reason="not implemented yet")
    def test_get_model_status_rest(self, resnet_multiple_batch_sizes,
                                   start_server_single_model):
        print("Downloaded model files:", resnet_multiple_batch_sizes)

        _, ports = start_server_single_model
        rest_url = 'http://localhost:{}/v1/models/resnet'.format(
                    ports["rest_port"])
        response = get_model_status_response_rest(rest_url)
        versions_statuses = response.model_version_status
        version_status = versions_statuses[0]
        assert version_status.version == 1
        assert version_status.state == ModelVersionState.AVAILABLE
        assert version_status.status.error_code == ErrorCode.OK
        assert version_status.status.error_message == ERROR_MESSAGE[
            ModelVersionState.AVAILABLE][ErrorCode.OK]
