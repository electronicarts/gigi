// (c) Electronic Arts.  All Rights Reserved.

#include "simpleRenderPassModule.h"

#include <Engine.Base/Misc/Vars.h>
#include <Engine.Math/Math.h>
#include <Engine.Render/Core/GetTextureResource.h>
#include <Engine.Render/Core2/Rc2ShaderProgramBinder.h>
#include <Engine.Render/FrameGraph/FrameGraph.h>
#include <Engine.Render/FrameGraph/FrameGraphBookmark.h>
#include <Engine.Render/FrameGraph/FrameGraphRenderPassData.h>
#include <Engine.Render/Raytrace/RaytracingUtil.h>
#include <Engine.World.Render/FrameGraphWorldRenderer.h>
#include <Engine.World.Render/simple/simpleShaderProgramsDefs.h>
#include <Engine.World.Render/Raytrace/RaytraceRenderModule.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>

namespace fb
{
ShaderProgramPipelineCache simpleRenderPassModule::s_pipelineCache;

template <typename T>
T Pow2GE(const T& A)
{
    float f = std::log2(float(A));
    f = std::ceilf(f);
    return (T)std::pow(2.0f, f);
}

void simpleRenderPassModule::addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard)
{
	// TODO: Provide this imported texture
	FrameGraphMutableResource& textureInput = ;  // This is the input texture to be modified

	// Initialization - create resources internal to the technique
	{
		struct PassData
		{
		};

		frameGraph.addRc2CallbackPass<PassData>("simple.Initialization", [&](RenderPassBuilderBase& builder, PassData& data)
		{
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
		});
	}

	// Compute Shader: DoSimpleCS
	// Runs the shader
	{
		struct PassData
		{
			uint dispatchX;
			uint dispatchY;
			uint dispatchZ;
			FrameGraphMutableResource textureInput;
		};

		frameGraph.addRc2CallbackPass<PassData>("simple.DoSimpleCS",
		[&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureInput = builder.writeAndUpdate(textureInput, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			const FrameGraphTextureDesc& desc = builder.getTextureDesc(textureInput);
			data.dispatchX = ((desc.width + 0) * 1) / 1 + 0;
			data.dispatchY = ((desc.height + 0) * 1) / 1 + 0;
			data.dispatchZ = ((desc.depth + 0) * 1) / 1 + 0;
			data.dispatchX = (data.dispatchX + 8 - 1) / 8;
			data.dispatchY = (data.dispatchY + 8 - 1) / 8;
			data.dispatchZ = (data.dispatchZ + 1 - 1) / 1;
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			const ComputePipelineInfo& info = s_pipelineCache.getComputePipeline(ShaderProgram_simple_DoSimpleCS_MainCs, 0);
			commandRecorder.setPipeline(info.pipeline);

			Rc2ComputeShaderProgramBinder binder(commandRecorder, info.computeShaderInfo);

			binder.computeStage()
				.setUav(0, resources.getUav(data.textureInput));
			binder.bind();

			commandRecorder.dispatch(data.dispatchX, data.dispatchY, data.dispatchZ);
		});
	}

	// bufferInput TODO: Put this texture into the black board (Optional)
}

void simpleRenderPassModule::onInit(const InitParams& params)
{
	m_arena = params.memoryArena;

	// Register with framegraph
	renderPassBookmark::registerPass(RenderPassBookmark_AfterPost,
		[this](FrameGraph& frameGraph, FrameGraphBlackboard& blackboard) { addPass(frameGraph, blackboard); });
}

simpleRenderPassModule::~simpleRenderPassModule()
{
	s_pipelineCache.clear();
}

}
