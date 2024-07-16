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