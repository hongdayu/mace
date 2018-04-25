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

#include <string>
#include "mace/core/operator.h"
#include "mace/core/testing/test_benchmark.h"
#include "mace/ops/ops_test_util.h"

namespace mace {
namespace ops {
namespace test {

namespace {
template <DeviceType D, typename T>
void ResizeBilinearBenchmark(int iters,
                             int batch,
                             int channels,
                             int input_height,
                             int input_width,
                             int output_height,
                             int output_width) {
  mace::testing::StopTiming();

  OpsTestNet net;

  // Add input data
  if (D == DeviceType::CPU) {
    net.AddRandomInput<D, float>("Input",
                                 {batch, channels, input_height, input_width});
  } else if (D == DeviceType::OPENCL) {
    net.AddRandomInput<D, float>("Input",
                                 {batch, input_height, input_width, channels});
  } else {
    MACE_NOT_IMPLEMENTED;
  }
  net.AddInputFromArray<D, index_t>("OutSize", {2},
                                    {output_height, output_width});

  if (D == DeviceType::CPU) {
    OpDefBuilder("ResizeBilinear", "ResizeBilinearBenchmark")
      .Input("Input")
      .Input("OutSize")
      .Output("Output")
      .AddIntsArg("size", {output_height, output_width})
      .AddIntArg("T", static_cast<int>(DataTypeToEnum<T>::value))
      .Finalize(net.NewOperatorDef());
  } else if (D == DeviceType::OPENCL) {
    BufferToImage<D, T>(&net, "Input", "InputImage",
                        kernels::BufferType::IN_OUT_CHANNEL);
    OpDefBuilder("ResizeBilinear", "ResizeBilinearBenchmark")
        .Input("InputImage")
        .Input("OutSize")
        .Output("OutputImage")
        .AddIntsArg("size", {output_height, output_width})
        .AddIntArg("T", static_cast<int>(DataTypeToEnum<T>::value))
        .Finalize(net.NewOperatorDef());
  } else {
    MACE_NOT_IMPLEMENTED;
  }

  // Warm-up
  for (int i = 0; i < 5; ++i) {
    net.RunOp(D);
  }

  mace::testing::StartTiming();
  while (iters--) {
    net.RunOp(D);
  }
  net.Sync();
}
}  // namespace

#define BM_RESIZE_BILINEAR_MACRO(N, C, H0, W0, H1, W1, TYPE, DEVICE)        \
  static void                                                               \
      BM_RESIZE_BILINEAR_##N##_##C##_##H0##_##W0##_##H1##_##W1##_##TYPE##_\
        ##DEVICE(                                                           \
          int iters) {                                                      \
    const int64_t macc = static_cast<int64_t>(iters) * N * C * H1 * W1 * 3; \
    const int64_t tot = static_cast<int64_t>(iters) * N * C * H0 * W0;      \
    mace::testing::MaccProcessed(macc);                                     \
    mace::testing::BytesProcessed(tot *(sizeof(TYPE)));                     \
    ResizeBilinearBenchmark<DEVICE, TYPE>(iters, N, C, H0, W0, H1, W1);     \
  }                                                                         \
  BENCHMARK(                                                                \
      BM_RESIZE_BILINEAR_##N##_##C##_##H0##_##W0##_##H1##_##W1##_##TYPE##_\
        ##DEVICE)

#define BM_RESIZE_BILINEAR(N, C, H0, W0, H1, W1)                 \
  BM_RESIZE_BILINEAR_MACRO(N, C, H0, W0, H1, W1, float, CPU);    \
  BM_RESIZE_BILINEAR_MACRO(N, C, H0, W0, H1, W1, float, OPENCL); \
  BM_RESIZE_BILINEAR_MACRO(N, C, H0, W0, H1, W1, half, OPENCL);

BM_RESIZE_BILINEAR(1, 128, 120, 120, 480, 480);

BM_RESIZE_BILINEAR(1, 256, 7, 7, 15, 15);
BM_RESIZE_BILINEAR(1, 256, 15, 15, 30, 30);
BM_RESIZE_BILINEAR(1, 128, 30, 30, 60, 60);
BM_RESIZE_BILINEAR(1, 128, 240, 240, 480, 480);
BM_RESIZE_BILINEAR(1, 3, 4032, 3016, 480, 480);
BM_RESIZE_BILINEAR(1, 3, 480, 480, 4032, 3016);

}  // namespace test
}  // namespace ops
}  // namespace mace
