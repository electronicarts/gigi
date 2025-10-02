//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "ConstOverridePass.h"
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

void ConstOverridePipeline::loadPipeline(ShaderPipelines& pipeline)
{

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			ClearPipelineId,
			"Shaders/ConstOverride/Clear",
			ShaderSignatureDesc(
				{ ShaderParameter(ClearSrvCount, ClearUavCount) },
				{ }));
	}

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			Left_WriteColorPipelineId,
			"Shaders/ConstOverride/Left_WriteColor",
			ShaderSignatureDesc(
				{ ShaderParameter(Left_WriteColorSrvCount, Left_WriteColorUavCount) },
				{ }));
	}

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			Right_WriteColorPipelineId,
			"Shaders/ConstOverride/Right_WriteColor",
			ShaderSignatureDesc(
				{ ShaderParameter(Right_WriteColorSrvCount, Right_WriteColorUavCount) },
				{ }));
	}
}

void ConstOverridePipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	pipeline.unloadPipeline(getStaticClassId(), ClearPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), Left_WriteColorPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), Right_WriteColorPipelineId);
}

void ConstOverridePass::createGlobalResourcesHandles()
{
}

void ConstOverridePass::deleteGlobalResourcesHandles()
{
}

void ConstOverridePass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void ConstOverridePass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<ConstOverridePassData>();

}

void ConstOverridePass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void ConstOverridePass::addConstOverridePass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.ConstOverride;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<ConstOverridePassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		// Texture: Output
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2d;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			desc.format = RenderFormat::R8G8B8A8_SRGB;
			desc.width = ((1 + 0)512) / 1 + 0;
			desc.height = ((1 + 0)512) / 1 + 0;
			passData.textureOutput = build.create("Texture Output", desc);
		}

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("ConstOverride InitPass", InitPass);

	// Compute Shader: Clear
	auto& ClearPass = [&](RenderGraphBuild& build)
	{
		auto textureOutput = passData.textureOutput = build.write(passData.textureOutput, RenderBindFlags::UnorderedAccess);

		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureOutput, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(ConstOverridePipeline::getStaticClassId(), ConstOverridePipeline::ClearPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dRwView(registry, textureOutput),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, registry.createShaderViews(uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("ConstOverride Clear", ClearPass);

	// Compute Shader: Left_WriteColor
	auto& Left_WriteColorPass = [&](RenderGraphBuild& build)
	{
		auto textureColor = passData.textureOutput = build.write(passData.textureOutput, RenderBindFlags::UnorderedAccess);

		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureOutput, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(ConstOverridePipeline::getStaticClassId(), ConstOverridePipeline::Left_WriteColorPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dRwView(registry, textureColor),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, registry.createShaderViews(uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("ConstOverride Left_WriteColor", Left_WriteColorPass);

	// Compute Shader: Right_WriteColor
	auto& Right_WriteColorPass = [&](RenderGraphBuild& build)
	{
		auto textureColor = passData.textureOutput = build.write(passData.textureOutput, RenderBindFlags::UnorderedAccess);

		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureOutput, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(ConstOverridePipeline::getStaticClassId(), ConstOverridePipeline::Right_WriteColorPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dRwView(registry, textureColor),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, registry.createShaderViews(uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("ConstOverride Right_WriteColor", Right_WriteColorPass);
}

} // namespace halcyon
