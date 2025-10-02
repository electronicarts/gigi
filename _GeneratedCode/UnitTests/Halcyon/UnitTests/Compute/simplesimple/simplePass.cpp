//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "simplePass.h"
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

void simplePipeline::loadPipeline(ShaderPipelines& pipeline)
{

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			DoSimpleCSPipelineId,
			"Shaders/simple/DoSimpleCS",
			ShaderSignatureDesc(
				{ ShaderParameter(DoSimpleCSSrvCount, DoSimpleCSUavCount) },
				{ }));
	}
}

void simplePipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	pipeline.unloadPipeline(getStaticClassId(), DoSimpleCSPipelineId);
}

void simplePass::createGlobalResourcesHandles()
{
}

void simplePass::deleteGlobalResourcesHandles()
{
}

void simplePass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void simplePass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<simplePassData>();

}

void simplePass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void simplePass::addsimplePass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.simple;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<simplePassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("simple InitPass", InitPass);

	// Compute Shader: DoSimpleCS
	// Runs the shader
	auto& DoSimpleCSPass = [&](RenderGraphBuild& build)
	{
		auto textureInput = passData.textureInput = build.write(passData.textureInput, RenderBindFlags::UnorderedAccess);

		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureInput, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(simplePipeline::getStaticClassId(), simplePipeline::DoSimpleCSPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dRwView(registry, textureInput),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, registry.createShaderViews(uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("simple DoSimpleCS", DoSimpleCSPass);
}

} // namespace halcyon
