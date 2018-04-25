// Copyright 2018 Xiaomi, Inc.  All rights reserved.
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

#include <vector>

#include "mace/ops/fused_conv_2d.h"
#include "mace/ops/ops_test_util.h"

namespace mace {
namespace ops {
namespace test {

class FusedConv2dOpTest : public OpsTestBase {};

namespace {
template<DeviceType D, typename T>
void TestNHWCSimple3x3VALID() {
  OpsTestNet net;
  // Add input data
  net.AddInputFromArray<D, float>(
    "Input", {1, 3, 3, 2},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1});
  net.AddInputFromArray<D, float>(
    "Filter", {3, 3, 1, 2},
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
     1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});
  net.AddInputFromArray<D, float>("Bias", {1}, {-0.1f});

  if (D == DeviceType::CPU) {
    net.TransformDataFormat<DeviceType::CPU, float>("Input",
                                                    NHWC,
                                                    "InputNCHW",
                                                    NCHW);
    net.TransformDataFormat<DeviceType::CPU, float>("Filter",
                                                    HWOI,
                                                    "FilterOIHW",
                                                    OIHW);
    OpDefBuilder("FusedConv2D", "FusedConv2dTest")
      .Input("InputNCHW")
      .Input("FilterOIHW")
      .Input("Bias")
      .Output("OutputNCHW")
      .AddIntsArg("strides", {1, 1})
      .AddIntArg("padding", Padding::VALID)
      .AddIntsArg("dilations", {1, 1})
      .AddIntArg("T", static_cast<int>(DataTypeToEnum<T>::value))
      .AddStringArg("activation", "RELU")
      .Finalize(net.NewOperatorDef());
    // Run
    net.RunOp(D);
    net.TransformDataFormat<DeviceType::CPU, float>("OutputNCHW",
                                                    NCHW,
                                                    "Output",
                                                    NHWC);
  } else if (D == DeviceType::OPENCL) {
    BufferToImage<D, T>(&net, "Input", "InputImage",
                        kernels::BufferType::IN_OUT_CHANNEL);
    BufferToImage<D, T>(&net, "Filter", "FilterImage",
                        kernels::BufferType::CONV2D_FILTER);
    BufferToImage<D, T>(&net, "Bias", "BiasImage",
                        kernels::BufferType::ARGUMENT);
    OpDefBuilder("FusedConv2D", "FusedConv2dTest")
      .Input("InputImage")
      .Input("FilterImage")
      .Input("BiasImage")
      .Output("OutputImage")
      .AddIntsArg("strides", {1, 1})
      .AddIntArg("padding", Padding::VALID)
      .AddIntsArg("dilations", {1, 1})
      .AddIntArg("T", static_cast<int>(DataTypeToEnum<T>::value))
      .AddStringArg("activation", "RELU")
      .Finalize(net.NewOperatorDef());

    net.RunOp(D);

    // Transfer output
    ImageToBuffer<D, float>(&net, "OutputImage", "Output",
                            kernels::BufferType::IN_OUT_CHANNEL);

  } else {
    MACE_NOT_IMPLEMENTED;
  }

  auto expected = CreateTensor<float>({1, 1, 1, 1}, {0.0f});
  ExpectTensorNear<float>(*expected, *net.GetOutput("Output"));
}

template<DeviceType D, typename T>
void TestNHWCSimple3x3SAME() {
  OpsTestNet net;

  // Add input data
  net.AddInputFromArray<D, float>(
    "Input", {1, 3, 3, 2},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1});
  net.AddInputFromArray<D, float>(
    "Filter", {3, 3, 1, 2},
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
     1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});
  net.AddInputFromArray<D, float>("Bias", {1}, {-0.1f});

  if (D == DeviceType::CPU) {
    net.TransformDataFormat<DeviceType::CPU, float>("Input",
                                                    NHWC,
                                                    "InputNCHW",
                                                    NCHW);
    net.TransformDataFormat<DeviceType::CPU, float>("Filter",
                                                    HWOI,
                                                    "FilterOIHW",
                                                    OIHW);
    OpDefBuilder("FusedConv2D", "FusedConv2dTest")
      .Input("InputNCHW")
      .Input("FilterOIHW")
      .Input("Bias")
      .Output("OutputNCHW")
      .AddIntsArg("strides", {1, 1})
      .AddIntArg("padding", Padding::SAME)
      .AddIntsArg("dilations", {1, 1})
      .AddIntArg("T", static_cast<int>(DataTypeToEnum<T>::value))
      .AddStringArg("activation", "RELU")
      .Finalize(net.NewOperatorDef());
    // Run
    net.RunOp(D);
    net.TransformDataFormat<DeviceType::CPU, float>("OutputNCHW",
                                                    NCHW,
                                                    "Output",
                                                    NHWC);
  } else if (D == DeviceType::OPENCL) {
    BufferToImage<D, T>(&net, "Input", "InputImage",
                        kernels::BufferType::IN_OUT_CHANNEL);
    BufferToImage<D, T>(&net, "Filter", "FilterImage",
                        kernels::BufferType::CONV2D_FILTER);
    BufferToImage<D, T>(&net, "Bias", "BiasImage",
                        kernels::BufferType::ARGUMENT);
    OpDefBuilder("FusedConv2D", "FusedConv2dTest")
      .Input("InputImage")
      .Input("FilterImage")
      .Input("BiasImage")
      .Output("OutputImage")
      .AddIntsArg("strides", {1, 1})
      .AddIntArg("padding", Padding::SAME)
      .AddIntsArg("dilations", {1, 1})
      .AddIntArg("T", static_cast<int>(DataTypeToEnum<T>::value))
      .AddStringArg("activation", "RELU")
      .Finalize(net.NewOperatorDef());
    // Run
    net.RunOp(D);

    // Transfer output
    ImageToBuffer<D, float>(&net, "OutputImage", "Output",
                            kernels::BufferType::IN_OUT_CHANNEL);

  } else {
    MACE_NOT_IMPLEMENTED;
  }

  auto expected = CreateTensor<float>(
    {1, 3, 3, 1}, {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f});

  ExpectTensorNear<float>(*expected, *net.GetOutput("Output"));
}
}  // namespace

TEST_F(FusedConv2dOpTest, CPUSimple) {
  TestNHWCSimple3x3VALID<DeviceType::CPU, float>();
  TestNHWCSimple3x3SAME<DeviceType::CPU, float>();
}

TEST_F(FusedConv2dOpTest, OPENCLSimple) {
  TestNHWCSimple3x3VALID<DeviceType::OPENCL, float>();
  TestNHWCSimple3x3SAME<DeviceType::OPENCL, float>();
}

namespace {
template<DeviceType D, typename T>
void TestNHWCSimple3x3WithoutBias() {
  OpsTestNet net;

  // Add input data
  net.AddInputFromArray<D, float>(
    "Input", {1, 3, 3, 2},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1});
  net.AddInputFromArray<D, float>(
    "Filter", {3, 3, 1, 2},
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
     1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f});

  if (D == DeviceType::CPU) {
    net.TransformDataFormat<DeviceType::CPU, float>("Input",
                                                    NHWC,
                                                    "InputNCHW",
                                                    NCHW);
    net.TransformDataFormat<DeviceType::CPU, float>("Filter",
                                                    HWOI,
                                                    "FilterOIHW",
                                                    OIHW);
    OpDefBuilder("FusedConv2D", "FusedConv2dTest")
      .Input("InputNCHW")
      .Input("FilterOIHW")
      .Output("OutputNCHW")
      .AddIntsArg("strides", {1, 1})
      .AddIntArg("padding", Padding::VALID)
      .AddIntsArg("dilations", {1, 1})
      .AddIntArg("T", static_cast<int>(DataTypeToEnum<T>::value))
      .AddStringArg("activation", "RELU")
      .Finalize(net.NewOperatorDef());

    // Run
    net.RunOp(D);
    net.TransformDataFormat<DeviceType::CPU, float>("OutputNCHW",
                                                    NCHW,
                                                    "Output",
                                                    NHWC);
  } else if (D == DeviceType::OPENCL) {
    BufferToImage<D, T>(&net, "Input", "InputImage",
                        kernels::BufferType::IN_OUT_CHANNEL);
    BufferToImage<D, T>(&net, "Filter", "FilterImage",
                        kernels::BufferType::CONV2D_FILTER);

    OpDefBuilder("FusedConv2D", "FusedConv2dTest")
      .Input("InputImage")
      .Input("FilterImage")
      .Output("OutputImage")
      .AddIntsArg("strides", {1, 1})
      .AddIntArg("padding", Padding::VALID)
      .AddIntsArg("dilations", {1, 1})
      .AddIntArg("T", static_cast<int>(DataTypeToEnum<T>::value))
      .AddStringArg("activation", "RELU")
      .Finalize(net.NewOperatorDef());
    // Run
    net.RunOp(D);
    // Transfer output
    ImageToBuffer<D, float>(&net, "OutputImage", "Output",
                            kernels::BufferType::IN_OUT_CHANNEL);
  } else {
    MACE_NOT_IMPLEMENTED;
  }

  // Check
  auto expected = CreateTensor<float>({1, 1, 1, 1}, {0.0f});

  ExpectTensorNear<float>(*expected, *net.GetOutput("Output"));
}
}  // namespace

TEST_F(FusedConv2dOpTest, CPUWithoutBias) {
  TestNHWCSimple3x3WithoutBias<DeviceType::CPU, float>();
}

TEST_F(FusedConv2dOpTest, OPENCLWithoutBias) {
  TestNHWCSimple3x3WithoutBias<DeviceType::OPENCL, float>();
}

namespace {
template<DeviceType D>
void TestConv1x1() {
  // Construct graph
  OpsTestNet net;

  // Add input data
  net.AddInputFromArray<D, float>(
    "Input", {1, 3, 10, 5},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1});
  net.AddInputFromArray<D, float>(
    "Filter", {1, 1, 2, 5},
    {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f});
  net.AddInputFromArray<D, float>("Bias", {2}, {0.1f, 0.2f});

  if (D == DeviceType::CPU) {
    net.TransformDataFormat<DeviceType::CPU, float>("Input",
                                                    NHWC,
                                                    "InputNCHW",
                                                    NCHW);
    net.TransformDataFormat<DeviceType::CPU, float>("Filter",
                                                    HWOI,
                                                    "FilterOIHW",
                                                    OIHW);
    OpDefBuilder("FusedConv2D", "FusedConv2dTest")
      .Input("InputNCHW")
      .Input("FilterOIHW")
      .Input("Bias")
      .Output("OutputNCHW")
      .AddIntsArg("strides", {1, 1})
      .AddIntArg("padding", Padding::VALID)
      .AddIntsArg("dilations", {1, 1})
      .Finalize(net.NewOperatorDef());
    // Run
    net.RunOp(D);
    net.TransformDataFormat<DeviceType::CPU, float>("OutputNCHW",
                                                    NCHW,
                                                    "Output",
                                                    NHWC);
  } else if (D == DeviceType::OPENCL) {
    BufferToImage<D, float>(&net, "Input", "InputImage",
                            kernels::BufferType::IN_OUT_CHANNEL);
    BufferToImage<D, float>(&net, "Filter", "FilterImage",
                            kernels::BufferType::CONV2D_FILTER);
    BufferToImage<D, float>(&net, "Bias", "BiasImage",
                            kernels::BufferType::ARGUMENT);

    OpDefBuilder("FusedConv2D", "FusedConv2dTest")
      .Input("InputImage")
      .Input("FilterImage")
      .Input("BiasImage")
      .Output("OutputImage")
      .AddIntsArg("strides", {1, 1})
      .AddIntArg("padding", Padding::VALID)
      .AddIntsArg("dilations", {1, 1})
      .Finalize(net.NewOperatorDef());
    // Run
    net.RunOp(D);

    ImageToBuffer<D, float>(&net, "OutputImage", "Output",
                            kernels::BufferType::IN_OUT_CHANNEL);
  } else {
    MACE_NOT_IMPLEMENTED;
  }

  // Check
  auto expected = CreateTensor<float>(
    {1, 3, 10, 2},
    {5.1f, 10.2f, 5.1f, 10.2f, 5.1f, 10.2f, 5.1f, 10.2f, 5.1f, 10.2f,
     5.1f, 10.2f, 5.1f, 10.2f, 5.1f, 10.2f, 5.1f, 10.2f, 5.1f, 10.2f,
     5.1f, 10.2f, 5.1f, 10.2f, 5.1f, 10.2f, 5.1f, 10.2f, 5.1f, 10.2f,
     5.1f, 10.2f, 5.1f, 10.2f, 5.1f, 10.2f, 5.1f, 10.2f, 5.1f, 10.2f,
     5.1f, 10.2f, 5.1f, 10.2f, 5.1f, 10.2f, 5.1f, 10.2f, 5.1f, 10.2f,
     5.1f, 10.2f, 5.1f, 10.2f, 5.1f, 10.2f, 5.1f, 10.2f, 5.1f, 10.2f});

  ExpectTensorNear<float>(*expected, *net.GetOutput("Output"));
}
}  // namespace

TEST_F(FusedConv2dOpTest, CPUConv1x1) { TestConv1x1<DeviceType::CPU>(); }

TEST_F(FusedConv2dOpTest, OPENCLConv1x1) { TestConv1x1<DeviceType::OPENCL>(); }

namespace {
template<DeviceType D, typename T>
void TestComplexConvNxNS12(const std::vector<index_t> &shape) {
  testing::internal::LogToStderr();
  auto func = [&](int kernel_h, int kernel_w, int stride_h, int stride_w,
                  Padding type) {
    // generate random input
    static unsigned int seed = time(NULL);
    index_t batch = 3 + (rand_r(&seed) % 10);
    index_t height = shape[0];
    index_t width = shape[1];
    index_t input_channels = shape[2] + (rand_r(&seed) % 10);
    index_t output_channels = shape[3] + (rand_r(&seed) % 10);

    OpsTestNet net;

    // Add input data
    net.AddRandomInput<D, T>("Input", {batch, height, width, input_channels});
    net.AddRandomInput<D, T>(
      "Filter", {kernel_h, kernel_w, output_channels, input_channels});
    net.AddRandomInput<D, T>("Bias", {output_channels});

    net.TransformDataFormat<DeviceType::CPU, float>("Input",
                                                    NHWC,
                                                    "InputNCHW",
                                                    NCHW);
    net.TransformDataFormat<DeviceType::CPU, float>("Filter",
                                                    HWOI,
                                                    "FilterOIHW",
                                                    OIHW);

    // Construct graph
    OpDefBuilder("FusedConv2D", "FusedConv2dTest")
      .Input("InputNCHW")
      .Input("FilterOIHW")
      .Input("Bias")
      .Output("OutputNCHW")
      .AddIntsArg("strides", {stride_h, stride_w})
      .AddIntArg("padding", type)
      .AddIntsArg("dilations", {1, 1})
      .AddIntArg("T", static_cast<int>(DataTypeToEnum<T>::value))
      .Finalize(net.NewOperatorDef());

    // run on cpu
    net.RunOp();
    net.TransformDataFormat<DeviceType::CPU, float>("OutputNCHW",
                                                    NCHW,
                                                    "Output",
                                                    NHWC);

    // Check
    Tensor expected;
    expected.Copy(*net.GetOutput("Output"));

    // run on gpu
    BufferToImage<D, T>(&net, "Input", "InputImage",
                        kernels::BufferType::IN_OUT_CHANNEL);
    BufferToImage<D, T>(&net, "Filter", "FilterImage",
                        kernels::BufferType::CONV2D_FILTER);
    BufferToImage<D, T>(&net, "Bias", "BiasImage",
                        kernels::BufferType::ARGUMENT);

    OpDefBuilder("FusedConv2D", "FusedConv2dTest")
      .Input("InputImage")
      .Input("FilterImage")
      .Input("BiasImage")
      .Output("OutputImage")
      .AddIntsArg("strides", {stride_h, stride_w})
      .AddIntArg("padding", type)
      .AddIntsArg("dilations", {1, 1})
      .AddIntArg("T", static_cast<int>(DataTypeToEnum<T>::value))
      .Finalize(net.NewOperatorDef());
    // Run on device
    net.RunOp(D);

    ImageToBuffer<D, T>(&net, "OutputImage", "OPENCLOutput",
                        kernels::BufferType::IN_OUT_CHANNEL);
    ExpectTensorNear<float>(expected, *net.GetOutput("OPENCLOutput"),
                            1e-5, 1e-4);
  };

  for (int kernel_size : {1, 3}) {
    for (int stride : {1, 2}) {
      func(kernel_size, kernel_size, stride, stride, VALID);
      func(kernel_size, kernel_size, stride, stride, SAME);
    }
  }
}
}  // namespace

TEST_F(FusedConv2dOpTest, OPENCLUnalignedConvNxNS12) {
  TestComplexConvNxNS12<DeviceType::OPENCL, float>({107, 113, 5, 7});
}

namespace {
template<DeviceType D>
void TestHalfComplexConvNxNS12(const std::vector<index_t> &shape,
                               const int kernel, const int stride,
                               Padding type) {
  testing::internal::LogToStderr();
  // generate random input
  srand(time(NULL));
  index_t batch = 3;
  index_t height = shape[0];
  index_t width = shape[1];
  index_t input_channels = shape[2];
  index_t output_channels = shape[3];

  OpsTestNet net;

  std::vector<float> float_input_data;
  GenerateRandomRealTypeData({batch, height, width, input_channels},
                             &float_input_data);
  std::vector<float> float_filter_data;
  GenerateRandomRealTypeData(
      {kernel, kernel, output_channels, input_channels},
      &float_filter_data);
  std::vector<float> float_bias_data;
  GenerateRandomRealTypeData({output_channels}, &float_bias_data);
  // Add input data
  net.AddInputFromArray<D, float>(
      "Input", {batch, height, width, input_channels}, float_input_data);
  net.AddInputFromArray<D, float>(
      "Filter", {kernel, kernel, output_channels, input_channels},
      float_filter_data);
  net.AddInputFromArray<D, float>("Bias", {output_channels}, float_bias_data);

  net.TransformDataFormat<DeviceType::CPU, float>("Input",
                                                  NHWC,
                                                  "InputNCHW",
                                                  NCHW);
  net.TransformDataFormat<DeviceType::CPU, float>("Filter",
                                                  HWOI,
                                                  "FilterOIHW",
                                                  OIHW);

  // Construct graph
  OpDefBuilder("FusedConv2D", "FusedConv2dTest")
    .Input("InputNCHW")
    .Input("FilterOIHW")
    .Input("Bias")
    .Output("OutputNCHW")
    .AddIntsArg("strides", {stride, stride})
    .AddIntArg("padding", type)
    .AddIntsArg("dilations", {1, 1})
    .Finalize(net.NewOperatorDef());

  // run on cpu
  net.RunOp();
  net.TransformDataFormat<DeviceType::CPU, float>("OutputNCHW",
                                                  NCHW,
                                                  "Output",
                                                  NHWC);

  // Check
  Tensor expected;
  expected.Copy(*net.GetOutput("Output"));

  // run on gpu
  BufferToImage<D, half>(&net, "Input", "InputImage",
                         kernels::BufferType::IN_OUT_CHANNEL);
  BufferToImage<D, half>(&net, "Filter", "FilterImage",
                         kernels::BufferType::CONV2D_FILTER);
  BufferToImage<D, half>(&net, "Bias", "BiasImage",
                         kernels::BufferType::ARGUMENT);

  OpDefBuilder("FusedConv2D", "FusedConv2dTest")
      .Input("InputImage")
      .Input("FilterImage")
      .Input("BiasImage")
      .Output("OutputImage")
      .AddIntsArg("strides", {stride, stride})
      .AddIntArg("padding", type)
      .AddIntsArg("dilations", {1, 1})
      .AddIntArg("T", static_cast<int>(DataType::DT_HALF))
      .Finalize(net.NewOperatorDef());
  // Run on device
  net.RunOp(D);

  ImageToBuffer<D, float>(&net, "OutputImage", "OPENCLOutput",
                          kernels::BufferType::IN_OUT_CHANNEL);

  ExpectTensorNear<float>(expected, *net.GetOutput("OPENCLOutput"),
                          1e-2, 1e-1);
}
}  // namespace

TEST_F(FusedConv2dOpTest, OPENCLHalfAlignedConv1x1S12) {
  TestHalfComplexConvNxNS12<DeviceType::OPENCL>({32, 32, 32, 64}, 1, 1, VALID);
  TestHalfComplexConvNxNS12<DeviceType::OPENCL>({31, 37, 31, 37}, 1, 1, SAME);
  TestHalfComplexConvNxNS12<DeviceType::OPENCL>({32, 32, 32, 64}, 1, 2, VALID);
  TestHalfComplexConvNxNS12<DeviceType::OPENCL>({31, 37, 31, 37}, 1, 2, SAME);
}
TEST_F(FusedConv2dOpTest, OPENCLHalfAlignedConv3x3S12) {
  TestHalfComplexConvNxNS12<DeviceType::OPENCL>({32, 32, 32, 64}, 3, 1, VALID);
  TestHalfComplexConvNxNS12<DeviceType::OPENCL>({31, 37, 31, 37}, 3, 1, SAME);
  TestHalfComplexConvNxNS12<DeviceType::OPENCL>({32, 32, 32, 64}, 3, 2, VALID);
  TestHalfComplexConvNxNS12<DeviceType::OPENCL>({31, 37, 31, 37}, 3, 2, SAME);
}

namespace {
template<DeviceType D, typename T>
void TestGeneralConvNxNS12(const std::vector<index_t> &image_shape,
                           const std::vector<index_t> &filter_shape) {
  testing::internal::LogToStderr();
  auto func = [&](int stride_h, int stride_w, Padding type) {
    srand(time(NULL));

    // generate random input
    index_t batch = 1;
    index_t height = image_shape[0];
    index_t width = image_shape[1];
    index_t kernel_h = filter_shape[0];
    index_t kernel_w = filter_shape[1];
    index_t output_channels = filter_shape[2];
    index_t input_channels = filter_shape[3];

    OpsTestNet net;

    // Add input data
    net.AddRandomInput<D, T>("Input", {batch, height, width, input_channels});
    net.AddRandomInput<D, T>(
      "Filter", {kernel_h, kernel_w, output_channels, input_channels});
    net.AddRandomInput<D, T>("Bias", {output_channels});

    net.TransformDataFormat<DeviceType::CPU, float>("Input",
                                                    NHWC,
                                                    "InputNCHW",
                                                    NCHW);
    net.TransformDataFormat<DeviceType::CPU, float>("Filter",
                                                    HWOI,
                                                    "FilterOIHW",
                                                    OIHW);

    // Construct graph
    OpDefBuilder("FusedConv2D", "FusedConv2dTest")
      .Input("InputNCHW")
      .Input("FilterOIHW")
      .Input("Bias")
      .Output("OutputNCHW")
      .AddIntsArg("strides", {stride_h, stride_w})
      .AddIntArg("padding", type)
      .AddIntsArg("dilations", {1, 1})
      .AddIntArg("T", static_cast<int>(DataTypeToEnum<T>::value))
      .Finalize(net.NewOperatorDef());

    // run on cpu
    net.RunOp();
    net.TransformDataFormat<DeviceType::CPU, float>("OutputNCHW",
                                                    NCHW,
                                                    "Output",
                                                    NHWC);
    // Check
    Tensor expected;
    expected.Copy(*net.GetOutput("Output"));

    // run on gpu
    BufferToImage<D, T>(&net, "Input", "InputImage",
                        kernels::BufferType::IN_OUT_CHANNEL);
    BufferToImage<D, T>(&net, "Filter", "FilterImage",
                        kernels::BufferType::CONV2D_FILTER);
    BufferToImage<D, T>(&net, "Bias", "BiasImage",
                        kernels::BufferType::ARGUMENT);

    OpDefBuilder("FusedConv2D", "FusedConv2dTest")
      .Input("InputImage")
      .Input("FilterImage")
      .Input("BiasImage")
      .Output("OutputImage")
      .AddIntsArg("strides", {stride_h, stride_w})
      .AddIntArg("padding", type)
      .AddIntsArg("dilations", {1, 1})
      .AddIntArg("T", static_cast<int>(DataTypeToEnum<T>::value))
      .Finalize(net.NewOperatorDef());
    // Run on device
    net.RunOp(D);

    ImageToBuffer<D, T>(&net, "OutputImage", "OPENCLOutput",
                        kernels::BufferType::IN_OUT_CHANNEL);
    ExpectTensorNear<float>(expected, *net.GetOutput("OPENCLOutput"),
                            1e-5, 1e-4);
  };

  for (int stride : {1, 2}) {
    func(stride, stride, VALID);
    func(stride, stride, SAME);
  }
}
}  // namespace

TEST_F(FusedConv2dOpTest, OPENCL7X7ConvNxNS12) {
  TestGeneralConvNxNS12<DeviceType::OPENCL, float>({32, 32}, {7, 7, 64, 3});
}

TEST_F(FusedConv2dOpTest, OPENCL15X1ConvNxNS12) {
  TestGeneralConvNxNS12<DeviceType::OPENCL, float>({40, 40}, {15, 1, 64, 32});
}

namespace {
template<DeviceType D, typename T>
void TestAtrousConvNxN(const std::vector<index_t> &shape,
                       const int dilation) {
  testing::internal::LogToStderr();
  auto func = [&](int kernel_h, int kernel_w, int stride_h, int stride_w,
                  Padding type) {
    srand(time(NULL));

    // generate random input
    index_t batch = 1;
    index_t height = shape[0];
    index_t width = shape[1];
    index_t output_channels = shape[2];
    index_t input_channels = shape[3];

    OpsTestNet net;

    // Add input data
    net.AddRandomInput<D, T>("Input", {batch, height, width, input_channels});
    net.AddRandomInput<D, T>(
      "Filter", {kernel_h, kernel_w, output_channels, input_channels});
    net.AddRandomInput<D, T>("Bias", {output_channels});

    net.TransformDataFormat<DeviceType::CPU, float>("Input",
                                                    NHWC,
                                                    "InputNCHW",
                                                    NCHW);
    net.TransformDataFormat<DeviceType::CPU, float>("Filter",
                                                    HWOI,
                                                    "FilterOIHW",
                                                    OIHW);

    // Construct graph
    OpDefBuilder("FusedConv2D", "FusedConv2dTest")
      .Input("InputNCHW")
      .Input("FilterOIHW")
      .Input("Bias")
      .Output("OutputNCHW")
      .AddIntsArg("strides", {stride_h, stride_w})
      .AddIntArg("padding", type)
      .AddIntsArg("dilations", {dilation, dilation})
      .AddIntArg("T", static_cast<int>(DataTypeToEnum<T>::value))
      .Finalize(net.NewOperatorDef());

    // run on cpu
    net.RunOp();

    net.TransformDataFormat<DeviceType::CPU, float>("OutputNCHW",
                                                    NCHW,
                                                    "Output",
                                                    NHWC);

    // Check
    Tensor expected;
    expected.Copy(*net.GetOutput("Output"));

    // run on gpu
    BufferToImage<D, T>(&net, "Input", "InputImage",
                        kernels::BufferType::IN_OUT_CHANNEL);
    BufferToImage<D, T>(&net, "Filter", "FilterImage",
                        kernels::BufferType::CONV2D_FILTER);
    BufferToImage<D, T>(&net, "Bias", "BiasImage",
                        kernels::BufferType::ARGUMENT);

    OpDefBuilder("FusedConv2D", "FusedConv2dTest")
      .Input("InputImage")
      .Input("FilterImage")
      .Input("BiasImage")
      .Output("OutputImage")
      .AddIntsArg("strides", {stride_h, stride_w})
      .AddIntArg("padding", type)
      .AddIntsArg("dilations", {dilation, dilation})
      .AddIntArg("T", static_cast<int>(DataTypeToEnum<T>::value))
      .Finalize(net.NewOperatorDef());
    // Run on device
    net.RunOp(D);

    ImageToBuffer<D, T>(&net, "OutputImage", "OPENCLOutput",
                        kernels::BufferType::IN_OUT_CHANNEL);
    ExpectTensorNear<float>(expected, *net.GetOutput("OPENCLOutput"),
                            1e-5, 1e-4);
  };

  for (int kernel_size : {3}) {
    for (int stride : {1}) {
      func(kernel_size, kernel_size, stride, stride, VALID);
      func(kernel_size, kernel_size, stride, stride, SAME);
    }
  }
}
}  // namespace

TEST_F(FusedConv2dOpTest, OPENCLalignedAtrousConvNxN2) {
  TestAtrousConvNxN<DeviceType::OPENCL, float>({128, 128, 16, 16}, 2);
}

TEST_F(FusedConv2dOpTest, OPENCLalignedAtrousConvNxN4) {
  TestAtrousConvNxN<DeviceType::OPENCL, float>({128, 128, 16, 16}, 4);
}

TEST_F(FusedConv2dOpTest, OPENCLUnalignedAtrousConvNxN) {
  TestAtrousConvNxN<DeviceType::OPENCL, float>({107, 113, 5, 7}, 2);
}

namespace {
template<DeviceType D>
void TestGeneralHalfAtrousConv(const std::vector<index_t> &image_shape,
                               const std::vector<index_t> &filter_shape,
                               const std::vector<int> &dilations) {
  testing::internal::LogToStderr();
  auto func = [&](int stride_h, int stride_w, Padding type) {
    srand(time(NULL));

    // generate random input
    index_t batch = 1;
    index_t height = image_shape[0];
    index_t width = image_shape[1];
    index_t kernel_h = filter_shape[0];
    index_t kernel_w = filter_shape[1];
    index_t output_channels = filter_shape[2];
    index_t input_channels = filter_shape[3];

    OpsTestNet net;

    // Add input data
    net.AddRandomInput<D, float>("Input",
                                 {batch, height, width, input_channels});
    net.AddRandomInput<D, float>(
      "Filter", {kernel_h, kernel_w, output_channels, input_channels});
    net.AddRandomInput<D, float>("Bias", {output_channels});

    net.TransformDataFormat<DeviceType::CPU, float>("Input",
                                                    NHWC,
                                                    "InputNCHW",
                                                    NCHW);
    net.TransformDataFormat<DeviceType::CPU, float>("Filter",
                                                    HWOI,
                                                    "FilterOIHW",
                                                    OIHW);

    // Construct graph
    OpDefBuilder("FusedConv2D", "FusedConv2dTest")
      .Input("InputNCHW")
      .Input("FilterOIHW")
      .Input("Bias")
      .Output("OutputNCHW")
      .AddIntsArg("strides", {stride_h, stride_w})
      .AddIntArg("padding", type)
      .AddIntsArg("dilations", {1, 1})
      .Finalize(net.NewOperatorDef());

    // run on cpu
    net.RunOp();

    net.TransformDataFormat<DeviceType::CPU, float>("OutputNCHW",
                                                    NCHW,
                                                    "Output",
                                                    NHWC);
    // Check
    Tensor expected;
    expected.Copy(*net.GetOutput("Output"));

    // run on gpu
    BufferToImage<D, half>(&net, "Input", "InputImage",
                           kernels::BufferType::IN_OUT_CHANNEL);
    BufferToImage<D, half>(&net, "Filter", "FilterImage",
                           kernels::BufferType::CONV2D_FILTER);
    BufferToImage<D, half>(&net, "Bias", "BiasImage",
                           kernels::BufferType::ARGUMENT);

    OpDefBuilder("FusedConv2D", "FusedConv2dTest")
      .Input("InputImage")
      .Input("FilterImage")
      .Input("BiasImage")
      .Output("OutputImage")
      .AddIntsArg("strides", {stride_h, stride_w})
      .AddIntArg("padding", type)
      .AddIntsArg("dilations", {1, 1})
      .AddIntArg("T", static_cast<int>(DataTypeToEnum<half>::value))
      .Finalize(net.NewOperatorDef());
    // Run on device
    net.RunOp(D);

    ImageToBuffer<D, float>(&net, "OutputImage", "OPENCLOutput",
                            kernels::BufferType::IN_OUT_CHANNEL);
    ExpectTensorNear<float>(expected, *net.GetOutput("OPENCLOutput"),
                            1e-2, 1e-1);
  };

  func(1, 1, VALID);
  func(1, 1, SAME);
}
}  // namespace

TEST_F(FusedConv2dOpTest, OPENCL7X7AtrousConvD2) {
  TestGeneralHalfAtrousConv<DeviceType::OPENCL>({32, 32}, {7, 7, 16, 3},
                                                {2, 2});
}

TEST_F(FusedConv2dOpTest, OPENCL15X15AtrousConvD4) {
  TestGeneralHalfAtrousConv<DeviceType::OPENCL>({63, 71}, {15, 15, 16, 16},
                                                {2, 2});
}

}  // namespace test
}  // namespace ops
}  // namespace mace
