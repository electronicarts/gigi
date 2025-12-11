// This file is part of the FidelityFX SDK.
//
// Copyright (C) 2025 Advanced Micro Devices, Inc.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once

#include "../../api/include/ffx_api.hpp"
#include "ffx_denoiser.h"

// Helper types for header initialization. Api definition is in .h file.

namespace ffx
{
    template <>
    struct struct_type<ffxCreateContextDescDenoiser> : std::integral_constant<uint64_t, FFX_API_CREATE_CONTEXT_DESC_TYPE_DENOISER>
    {
    };

    struct CreateContextDescDenoiser : public InitHelper<ffxCreateContextDescDenoiser>
    {
    };

    template <>
    struct struct_type<ffxDispatchDescDenoiser> : std::integral_constant<uint64_t, FFX_API_DISPATCH_DESC_TYPE_DENOISER>
    {
    };

    struct DispatchDescDenoiser : public InitHelper<ffxDispatchDescDenoiser>
    {
    };

    template <>
    struct struct_type<ffxDispatchDescDenoiserInput4Signals> : std::integral_constant<uint64_t, FFX_API_DISPATCH_DESC_INPUT_4_SIGNALS_TYPE_DENOISER>
    {
    };

    struct DispatchDescDenoiserInput4Signals : public InitHelper<ffxDispatchDescDenoiserInput4Signals>
    {
    };

    template <>
    struct struct_type<ffxDispatchDescDenoiserInput2Signals> : std::integral_constant<uint64_t, FFX_API_DISPATCH_DESC_INPUT_2_SIGNALS_TYPE_DENOISER>
    {
    };

    struct DispatchDescDenoiserInput2Signals : public InitHelper<ffxDispatchDescDenoiserInput2Signals>
    {
    };

    template <>
    struct struct_type<ffxDispatchDescDenoiserInput1Signal> : std::integral_constant<uint64_t, FFX_API_DISPATCH_DESC_INPUT_1_SIGNAL_TYPE_DENOISER>
    {
    };

    struct DispatchDescDenoiserInput1Signal : public InitHelper<ffxDispatchDescDenoiserInput1Signal>
    {
    };

    template <>
    struct struct_type<ffxDispatchDescDenoiserInputDominantLight> : std::integral_constant<uint64_t, FFX_API_DISPATCH_DESC_INPUT_DOMINANT_LIGHT_TYPE_DENOISER>
    {
    };

    struct DispatchDescDenoiserInputDominantLight : public InitHelper<ffxDispatchDescDenoiserInputDominantLight>
    {
    };

    template<>
    struct struct_type<ffxConfigureDescDenoiserSettings> : std::integral_constant<uint64_t, FFX_API_CONFIGURE_DESC_TYPE_DENOISER_SETTINGS>
    {
    };

    struct ConfigureDescDenoiserSettings : public InitHelper<ffxConfigureDescDenoiserSettings>
    {
    };

    template <>
    struct struct_type<ffxQueryDescDenoiserGetGPUMemoryUsage> : std::integral_constant<uint64_t, FFX_API_QUERY_DESC_TYPE_DENOISER_GPU_MEMORY_USAGE>
    {
    };

    struct QueryDescDenoiserGetGPUMemoryUsage : public InitHelper<ffxQueryDescDenoiserGetGPUMemoryUsage>
    {
    };

    template <>
    struct struct_type<ffxQueryDescDenoiserGetVersion> : std::integral_constant<uint64_t, FFX_API_QUERY_DESC_TYPE_DENOISER_GET_VERSION>
    {
    };

    struct QueryDescDenoiserGetVersion : public InitHelper<ffxQueryDescDenoiserGetVersion>
    {
    };

    template <>
    struct struct_type<ffxQueryDescDenoiserGetDefaultSettings> : std::integral_constant<uint64_t, FFX_API_QUERY_DESC_TYPE_DENOISER_GET_DEFAULT_SETTINGS>
    {
    };

    struct QueryDescDenoiserGetDefaultSettings : public InitHelper<ffxQueryDescDenoiserGetDefaultSettings>
    {
    };

}  // namespace ffx
