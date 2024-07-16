///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

struct RenderGraph;

void MakeRenderGraphGraphViz(RenderGraph& renderGraph, const char* outFolder);
void MakeFlattenedRenderGraphGraphViz(RenderGraph& renderGraph, const char* outFolder);
void MakeSummaryRenderGraphGraphViz(RenderGraph& renderGraph, const char* outFolder);