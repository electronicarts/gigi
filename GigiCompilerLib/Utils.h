///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// clang-format off
#include <string>
#include <codecvt>
#include "gigicompiler.h"
// clang-format on

enum class LogLevel
{
	Info,
	Warn,
	Error
};

using LogFn = void (*)(LogLevel level, const char* msg, ...);

std::wstring ToWideString(const char* string);
std::string FromWideString(const wchar_t* string);

template <typename LAMBDA>
void RenderGraphNodeLambda(RenderGraphNode& node, const LAMBDA& lambda)
{
    switch (node._index)
    {
        // clang-format off
        #include "external/df_serialize/_common.h"
        #define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
            case RenderGraphNode::c_index_##_NAME: \
            { \
                lambda(node.##_NAME); \
                break; \
            }
        #include "external/df_serialize/_fillunsetdefines.h"
        #include "Schemas/RenderGraphNodesVariant.h"
        // clang-format on
    }
}

template <typename LAMBDA>
void RenderGraphNodeLambda(const RenderGraphNode& node, const LAMBDA& lambda)
{
    switch (node._index)
    {
        // clang-format off
        #include "external/df_serialize/_common.h"
        #define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
            case RenderGraphNode::c_index_##_NAME: \
            { \
                lambda(node.##_NAME); \
                break; \
            }
        #include "external/df_serialize/_fillunsetdefines.h"
        #include "Schemas/RenderGraphNodesVariant.h"
        // clang-format on
    }
}

std::string GetVertexShaderStructDefinition(const Struct& s);

inline std::string DataFieldTypeToHLSLType(DataFieldType type)
{
    switch (type)
    {
        case DataFieldType::Int: return "int";
        case DataFieldType::Int2: return "int2";
        case DataFieldType::Int3: return "int3";
        case DataFieldType::Int4: return "int4";
        case DataFieldType::Uint: return "uint";
        case DataFieldType::Uint2: return "uint2";
        case DataFieldType::Uint3: return "uint3";
        case DataFieldType::Uint4: return "uint4";
        case DataFieldType::Float: return "float";
        case DataFieldType::Float2: return "float2";
        case DataFieldType::Float3: return "float3";
        case DataFieldType::Float4: return "float4";
        // bools are differently sized on cpu and gpu
        // There is also some oddities when using them in structured buffers and using 1 and 0
        // for true and false instead of 0xffffffff and 0. better to dodge the problem.
        case DataFieldType::Bool: return "uint";
        case DataFieldType::Float4x4: return "float4x4";
        case DataFieldType::Uint_16: return "uint";
        case DataFieldType::Count:
        {
            Assert(false, "Invalid data field type: Count");
            return __FUNCTION__ " invalid field type";
        }
        default:
        {
            Assert(false, "Unhandled data field type: %i", type);
            return __FUNCTION__ " unknown field type";
        }
    }
}
