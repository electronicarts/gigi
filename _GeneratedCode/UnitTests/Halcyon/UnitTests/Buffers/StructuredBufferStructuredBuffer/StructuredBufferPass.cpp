//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "StructuredBufferPass.h"
#include <Common/Math/LinearTransform.h>
#include <Common/Render/DynamicConstantsAllocator.h>
#include <Common/Render/RenderGraph.h>
#include <Common/Render/RenderGraphBuild.h>
#include <Common/Render/RenderGraphRegistry.h>
#include <Common/Render/RenderGraphScope.h>
#include <Common/Render/RenderPassPayload.h>
#include <Common/Render/RenderSystem.h>

namespace halcyon
{

void StructuredBufferPipeline::loadPipeline(ShaderPipelines& pipeline)
{

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			csmainPipelineId,
			"Shaders/StructuredBuffer/csmain",
			ShaderSignatureDesc(
				{ ShaderParameter(csmainSrvCount, csmainUavCount) },
				{ }));
	}
}

void StructuredBufferPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	pipeline.unloadPipeline(getStaticClassId(), csmainPipelineId);
}

void StructuredBufferPass::createGlobalResourcesHandles()
{
}

void StructuredBufferPass::deleteGlobalResourcesHandles()
{
}

void StructuredBufferPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void StructuredBufferPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<StructuredBufferPassData>();

}

void StructuredBufferPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void StructuredBufferPass::addStructuredBufferPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.StructuredBuffer;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<StructuredBufferPassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("StructuredBuffer InitPass", InitPass);

	// Shader Constants: _csmain_0CB
	{
		m_cb__csmain_0CB.frameDeltaTime = settings.frameDeltaTime;
		m_cb__csmain_0CB.frameIndex = settings.frameIndex;
	}

	// Compute Shader: csmain
	auto& csmainPass = [&](RenderGraphBuild& build)
	{
		auto bufferbuff = passData.bufferbuff = build.write(passData.bufferbuff, RenderBindFlags::UnorderedAccess);

		RenderGraphBufferDesc sizeSrcDesc;
		build.getBufferDesc(passData.bufferbuff, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.size + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((1 + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((1 + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 1 - 1) / 1;
		dispatchHeight = (dispatchHeight + 1 - 1) / 1;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(StructuredBufferPipeline::getStaticClassId(), StructuredBufferPipeline::csmainPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__csmain_0CB);
			RenderBindingUnorderedAccessView uavs[] = {
				defaultBufferRwView(registry, bufferbuff),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, constantsOffset, registry.createShaderViews(uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("StructuredBuffer csmain", csmainPass);
}

} // namespace halcyon
