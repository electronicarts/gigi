// (c) Electronic Arts.  All Rights Reserved.

#include "MultipleUVMeshRenderPassModule.h"

#include <Engine.Base/Misc/Vars.h>
#include <Engine.Math/Math.h>
#include <Engine.Render/Core/GetTextureResource.h>
#include <Engine.Render/Core2/Rc2ShaderProgramBinder.h>
#include <Engine.Render/FrameGraph/FrameGraph.h>
#include <Engine.Render/FrameGraph/FrameGraphBookmark.h>
#include <Engine.Render/FrameGraph/FrameGraphRenderPassData.h>
#include <Engine.Render/Raytrace/RaytracingUtil.h>
#include <Engine.World.Render/FrameGraphWorldRenderer.h>
#include <Engine.World.Render/MultipleUVMesh/MultipleUVMeshShaderProgramsDefs.h>
#include <Engine.World.Render/Raytrace/RaytraceRenderModule.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>

namespace fb
{
ShaderProgramPipelineCache MultipleUVMeshRenderPassModule::s_pipelineCache;

template <typename T>
T Pow2GE(const T& A)
{
    float f = std::log2(float(A));
    f = std::ceilf(f);
    return (T)std::pow(2.0f, f);
}

void MultipleUVMeshRenderPassModule::addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard)
{

	// Initialization - create resources internal to the technique
	{
		struct PassData
		{
		};

		frameGraph.addRc2CallbackPass<PassData>("MultipleUVMesh.Initialization", [&](RenderPassBuilderBase& builder, PassData& data)
		{
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
		});
	}

}

void MultipleUVMeshRenderPassModule::onInit(const InitParams& params)
{
	m_arena = params.memoryArena;

	// Register with framegraph
	renderPassBookmark::registerPass(RenderPassBookmark_AfterPost,
		[this](FrameGraph& frameGraph, FrameGraphBlackboard& blackboard) { addPass(frameGraph, blackboard); });
}

MultipleUVMeshRenderPassModule::~MultipleUVMeshRenderPassModule()
{
	s_pipelineCache.clear();
}

}
