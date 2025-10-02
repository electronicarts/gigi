// (c) Electronic Arts.  All Rights Reserved.

#include "StructuredBufferRenderPassModule.h"

#include <Engine.Base/Misc/Vars.h>
#include <Engine.Math/Math.h>
#include <Engine.Render/Core/GetTextureResource.h>
#include <Engine.Render/Core2/Rc2ShaderProgramBinder.h>
#include <Engine.Render/FrameGraph/FrameGraph.h>
#include <Engine.Render/FrameGraph/FrameGraphBookmark.h>
#include <Engine.Render/FrameGraph/FrameGraphRenderPassData.h>
#include <Engine.Render/Raytrace/RaytracingUtil.h>
#include <Engine.World.Render/FrameGraphWorldRenderer.h>
#include <Engine.World.Render/StructuredBuffer/StructuredBufferShaderProgramsDefs.h>
#include <Engine.World.Render/Raytrace/RaytraceRenderModule.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>

namespace fb
{
ShaderProgramPipelineCache StructuredBufferRenderPassModule::s_pipelineCache;

template <typename T>
T Pow2GE(const T& A)
{
    float f = std::log2(float(A));
    f = std::ceilf(f);
    return (T)std::pow(2.0f, f);
}

void StructuredBufferRenderPassModule::addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard)
{
	// TODO: Provide this imported buffer
	FrameGraphMutableResource& bufferbuff = ;

	// Initialization - create resources internal to the technique
	{
		struct PassData
		{
		};

		frameGraph.addRc2CallbackPass<PassData>("StructuredBuffer.Initialization", [&](RenderPassBuilderBase& builder, PassData& data)
		{
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
		});
	}

	// Shader Constants: _csmain_0CB
	{
		m_cb__csmain_0CB.frameDeltaTime = m_varframeDeltaTime;
		m_cb__csmain_0CB.frameIndex = m_varframeIndex;
	}

	// Compute Shader: csmain
	{
		struct PassData
		{
			uint dispatchX;
			uint dispatchY;
			uint dispatchZ;
			FrameGraphMutableResource bufferbuff;
			Struct__csmain_0CB cb_csmain_0CB;
		};

		frameGraph.addRc2CallbackPass<PassData>("StructuredBuffer.csmain",
		[&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.bufferbuff = builder.writeAndUpdate(bufferbuff, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.cb_csmain_0CB = m_cb__csmain_0CB;
			const FrameGraphBufferDesc& desc = builder.getBufferDesc(bufferbuff);
			data.dispatchX = ((desc.size + 0) * 1) / 1 + 0;
			data.dispatchY = ((1 + 0) * 1) / 1 + 0;
			data.dispatchZ = ((1 + 0) * 1) / 1 + 0;
			data.dispatchX = (data.dispatchX + 1 - 1) / 1;
			data.dispatchY = (data.dispatchY + 1 - 1) / 1;
			data.dispatchZ = (data.dispatchZ + 1 - 1) / 1;
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			const ComputePipelineInfo& info = s_pipelineCache.getComputePipeline(ShaderProgram_StructuredBuffer_csmain_csmainCs, 0);
			commandRecorder.setPipeline(info.pipeline);

			Rc2ComputeShaderProgramBinder binder(commandRecorder, info.computeShaderInfo);

			binder.computeStage()
				.setUav(0, resources.getUav(data.bufferbuff))
				.setConstantBuffer(sizeof(data.cb_csmain_0CB), &data.cb_csmain_0CB);
			binder.bind();

			commandRecorder.dispatch(data.dispatchX, data.dispatchY, data.dispatchZ);
		});
	}

	// bufferbuff  TODO: Put this buffer into the black board (Optional)
}

void StructuredBufferRenderPassModule::onInit(const InitParams& params)
{
	m_arena = params.memoryArena;

	// Register with framegraph
	renderPassBookmark::registerPass(RenderPassBookmark_AfterPost,
		[this](FrameGraph& frameGraph, FrameGraphBlackboard& blackboard) { addPass(frameGraph, blackboard); });
}

StructuredBufferRenderPassModule::~StructuredBufferRenderPassModule()
{
	s_pipelineCache.clear();
}

}
