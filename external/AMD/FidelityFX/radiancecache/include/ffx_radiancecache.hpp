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
#include "ffx_radiancecache.h"

// Helper types for header initialization. Api definition is in .h file.

namespace ffx
{
    template <>
    struct struct_type<ffxCreateContextDescRadianceCache> : std::integral_constant<uint64_t, FFX_API_CREATE_CONTEXT_DESC_TYPE_RADIANCECACHE>
    {
    };

    struct CreateContextDescRadianceCache : public InitHelper<ffxCreateContextDescRadianceCache>
    {
    };

    template <>
    struct struct_type<ffxDispatchDescRadianceCache> : std::integral_constant<uint64_t, FFX_API_DISPATCH_DESC_TYPE_RADIANCECACHE>
    {
    };

    struct DispatchDescRadianceCache : public InitHelper<ffxDispatchDescRadianceCache>
    {
    };

    template <>
    struct struct_type<ffxDebugDispatchDescRadianceCache> : std::integral_constant<uint64_t, FFX_API_DEBUG_DISPATCH_DESC_TYPE_RADIANCECACHE>
    {
    };

    struct DebugDispatchDescRadianceCache : public InitHelper<ffxDebugDispatchDescRadianceCache>
    {
    };
}  // namespace ffx
