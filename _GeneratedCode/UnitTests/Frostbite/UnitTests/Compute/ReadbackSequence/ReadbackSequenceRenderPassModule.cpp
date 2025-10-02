// (c) Electronic Arts.  All Rights Reserved.

#include "ReadbackSequenceRenderPassModule.h"

#include <Engine.Base/Misc/Vars.h>
#include <Engine.Math/Math.h>
#include <Engine.Render/Core/GetTextureResource.h>
#include <Engine.Render/Core2/Rc2ShaderProgramBinder.h>
#include <Engine.Render/FrameGraph/FrameGraph.h>
#include <Engine.Render/FrameGraph/FrameGraphBookmark.h>
#include <Engine.Render/FrameGraph/FrameGraphRenderPassData.h>
#include <Engine.Render/Raytrace/RaytracingUtil.h>
#include <Engine.World.Render/FrameGraphWorldRenderer.h>
#include <Engine.World.Render/ReadbackSequence/ReadbackSequenceShaderProgramsDefs.h>
#include <Engine.World.Render/Raytrace/RaytraceRenderModule.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>

namespace fb
{
ShaderProgramPipelineCache ReadbackSequenceRenderPassModule::s_pipelineCache;

template <typename T>
T Pow2GE(const T& A)
{
    float f = std::log2(float(A));
    f = std::ceilf(f);
    return (T)std::pow(2.0f, f);
}

void ReadbackSequenceRenderPassModule::addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard)
{
	// TODO: Provide this imported texture
	FrameGraphMutableResource& textureOutput = ;

	// Initialization - create resources internal to the technique
	{
		struct PassData
		{
		};

		frameGraph.addRc2CallbackPass<PassData>("ReadbackSequence.Initialization", [&](RenderPassBuilderBase& builder, PassData& data)
		{
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
		});
	}

	// Shader Constants: _ReadbackSequenceCS_0CB
	{
		m_cb__ReadbackSequenceCS_0CB.frameIndex = m_varframeIndex;
	}

	// Compute Shader: Node_1
	{
		struct PassData
		{
			uint dispatchX;
			uint dispatchY;
			uint dispatchZ;
			FrameGraphMutableResource textureOutput;
			Struct__ReadbackSequenceCS_0CB cb_ReadbackSequenceCS_0CB;
		};

		frameGraph.addRc2CallbackPass<PassData>("ReadbackSequence.Node_1",
		[&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureOutput = builder.writeAndUpdate(textureOutput, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.cb_ReadbackSequenceCS_0CB = m_cb__ReadbackSequenceCS_0CB;
			const FrameGraphTextureDesc& desc = builder.getTextureDesc(textureOutput);
			data.dispatchX = ((desc.width + 0) * 1) / 1 + 0;
			data.dispatchY = ((desc.height + 0) * 1) / 1 + 0;
			data.dispatchZ = ((desc.depth + 0) * 1) / 1 + 0;
			data.dispatchX = (data.dispatchX + 8 - 1) / 8;
			data.dispatchY = (data.dispatchY + 8 - 1) / 8;
			data.dispatchZ = (data.dispatchZ + 1 - 1) / 1;
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			const ComputePipelineInfo& info = s_pipelineCache.getComputePipeline(ShaderProgram_ReadbackSequence_Node_1_csmainCs, 0);
			commandRecorder.setPipeline(info.pipeline);

			Rc2ComputeShaderProgramBinder binder(commandRecorder, info.computeShaderInfo);

			binder.computeStage()
				.setUav(0, resources.getUav(data.textureOutput))
				.setConstantBuffer(sizeof(data.cb_ReadbackSequenceCS_0CB), &data.cb_ReadbackSequenceCS_0CB);
			binder.bind();

			commandRecorder.dispatch(data.dispatchX, data.dispatchY, data.dispatchZ);
		});
	}

	// bufferOutput TODO: Put this texture into the black board (Optional)
}

void ReadbackSequenceRenderPassModule::onInit(const InitParams& params)
{
	m_arena = params.memoryArena;

	// Register with framegraph
	renderPassBookmark::registerPass(RenderPassBookmark_AfterPost,
		[this](FrameGraph& frameGraph, FrameGraphBlackboard& blackboard) { addPass(frameGraph, blackboard); });
}

ReadbackSequenceRenderPassModule::~ReadbackSequenceRenderPassModule()
{
	s_pipelineCache.clear();
}

}
