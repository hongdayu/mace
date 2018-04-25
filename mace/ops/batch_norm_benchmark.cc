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

#include "mace/core/operator.h"
#include "mace/core/runtime/opencl/opencl_runtime.h"
#include "mace/core/testing/test_benchmark.h"
#include "mace/ops/ops_test_util.h"

namespace mace {
namespace ops {
namespace test {

namespace {
template <DeviceType D, typename T>
void BatchNorm(
    int iters, int batch, int channels, int height, int width) {
  mace::testing::StopTiming();

  OpsTestNet net;

  // Add input data
  if (D == DeviceType::CPU) {
    net.AddRandomInput<D, T>("Input", {batch, channels, height, width});
  } else if (D == DeviceType::OPENCL) {
    net.AddRandomInput<D, T>("Input", {batch, height, width, channels});
  } else {
    MACE_NOT_IMPLEMENTED;
  }
  net.AddRandomInput<D, T>("Scale", {channels});
  net.AddRandomInput<D, T>("Offset", {channels});
  net.AddRandomInput<D, T>("Mean", {channels});
  net.AddRandomInput<D, T>("Var", {channels}, true);

  if (D == DeviceType::CPU) {
    OpDefBuilder("BatchNorm", "BatchNormBM")
      .Input("Input")
      .Input("Scale")
      .Input("Offset")
      .Input("Mean")
      .Input("Var")
      .AddFloatArg("epsilon", 1e-3)
      .Output("Output")
      .Finalize(net.NewOperatorDef());
  } else if (D == DeviceType::OPENCL) {
    BufferToImage<D, float>(&net, "Input", "InputImage",
                            kernels::BufferType::IN_OUT_CHANNEL);
    BufferToImage<D, float>(&net, "Scale", "ScaleImage",
                            kernels::BufferType::ARGUMENT);
    BufferToImage<D, float>(&net, "Offset", "OffsetImage",
                            kernels::BufferType::ARGUMENT);
    BufferToImage<D, float>(&net, "Mean", "MeanImage",
                            kernels::BufferType::ARGUMENT);
    BufferToImage<D, float>(&net, "Var", "VarImage",
                            kernels::BufferType::ARGUMENT);
    OpDefBuilder("BatchNorm", "BatchNormBM")
        .Input("InputImage")
        .Input("ScaleImage")
        .Input("OffsetImage")
        .Input("MeanImage")
        .Input("VarImage")
        .AddFloatArg("epsilon", 1e-3)
        .Output("Output")
        .Finalize(net.NewOperatorDef());
  } else {
    MACE_NOT_IMPLEMENTED;
  }

  // tuning
  setenv("MACE_TUNING", "1", 1);
  net.RunOp(D);
  unsetenv("MACE_TUNING");

  // Warm-up
  for (int i = 0; i < 5; ++i) {
    net.RunOp(D);
  }
  net.Sync();

  mace::testing::StartTiming();
  while (iters--) {
    net.RunOp(D);
  }
  net.Sync();
}
}  // namespace

#define BM_BATCH_NORM_MACRO(N, C, H, W, TYPE, DEVICE)                  \
  static void BM_BATCH_NORM_##N##_##C##_##H##_##W##_##TYPE##_##DEVICE( \
      int iters) {                                                     \
    const int64_t tot = static_cast<int64_t>(iters) * N * C * H * W;   \
    mace::testing::MaccProcessed(tot);                                 \
    mace::testing::BytesProcessed(tot *(sizeof(TYPE)));                \
    BatchNorm<DEVICE, TYPE>(iters, N, C, H, W);                        \
  }                                                                    \
  BENCHMARK(BM_BATCH_NORM_##N##_##C##_##H##_##W##_##TYPE##_##DEVICE)

#define BM_BATCH_NORM(N, C, H, W)                 \
  BM_BATCH_NORM_MACRO(N, C, H, W, float, CPU);    \
  BM_BATCH_NORM_MACRO(N, C, H, W, float, OPENCL); \
  BM_BATCH_NORM_MACRO(N, C, H, W, half, OPENCL);

BM_BATCH_NORM(1, 1, 512, 512);
BM_BATCH_NORM(1, 3, 128, 128);
BM_BATCH_NORM(1, 3, 512, 512);
BM_BATCH_NORM(1, 32, 112, 112);
BM_BATCH_NORM(1, 64, 256, 256);
BM_BATCH_NORM(1, 64, 512, 512);
BM_BATCH_NORM(1, 128, 56, 56);
BM_BATCH_NORM(1, 128, 256, 256);
BM_BATCH_NORM(1, 256, 14, 14);
BM_BATCH_NORM(1, 512, 14, 14);
BM_BATCH_NORM(1, 1024, 7, 7);
BM_BATCH_NORM(32, 1, 256, 256);
BM_BATCH_NORM(32, 3, 256, 256);

}  // namespace test
}  // namespace ops
}  // namespace mace
