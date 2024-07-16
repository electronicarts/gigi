///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// clang-format off
#include "Schemas/Types.h"
#include "GigiBuildFlavor.h"

#include <string>
// clang-format on

GigiCompileResult GigiCompile(GigiBuildFlavor buildFlavor, const std::string& jsonFile, const std::string& outputDir, void (*PostLoad)(RenderGraph&), RenderGraph* outRenderGraph, bool GENERATE_GRAPHVIZ_FLAG);

// Backend PostLoad prototype functions
// clang-format off
#include "external/df_serialize/_common.h"
#define ENUM_ITEM(x, y) void PostLoad_##x(RenderGraph& renderGraph);
#include "external/df_serialize/_fillunsetdefines.h"
#include "Schemas/BackendList.h"
// clang-format on
