//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "ReadbackSequencePass.h"
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

void ReadbackSequencePipeline::loadPipeline(ShaderPipelines& pipeline)
{

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			Node_1PipelineId,
			"Shaders/ReadbackSequence/Node_1",
			ShaderSignatureDesc(
				{ ShaderParameter(Node_1SrvCount, Node_1UavCount) },
				{ }));
	}
}

void ReadbackSequencePipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	pipeline.unloadPipeline(getStaticClassId(), Node_1PipelineId);
}

void ReadbackSequencePass::createGlobalResourcesHandles()
{
}

void ReadbackSequencePass::deleteGlobalResourcesHandles()
{
}

void ReadbackSequencePass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void ReadbackSequencePass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<ReadbackSequencePassData>();

}

void ReadbackSequencePass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void ReadbackSequencePass::addReadbackSequencePass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.ReadbackSequence;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<ReadbackSequencePassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("ReadbackSequence InitPass", InitPass);

	// Shader Constants: _ReadbackSequenceCS_0CB
	{
		m_cb__ReadbackSequenceCS_0CB.frameIndex = settings.frameIndex;
	}

	// Compute Shader: Node_1
	auto& Node_1Pass = [&](RenderGraphBuild& build)
	{
		auto textureoutput = passData.textureOutput = build.write(passData.textureOutput, RenderBindFlags::UnorderedAccess);

		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureOutput, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(ReadbackSequencePipeline::getStaticClassId(), ReadbackSequencePipeline::Node_1PipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__ReadbackSequenceCS_0CB);
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dRwView(registry, textureoutput),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, constantsOffset, registry.createShaderViews(uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("ReadbackSequence Node_1", Node_1Pass);
}

} // namespace halcyon
