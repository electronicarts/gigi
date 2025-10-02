//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "SubGraphTestPass.h"
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

void SubGraphTestPipeline::loadPipeline(ShaderPipelines& pipeline)
{

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			Swap_ColorsPipelineId,
			"Shaders/SubGraphTest/Swap_Colors",
			ShaderSignatureDesc(
				{ ShaderParameter(Swap_ColorsSrvCount, Swap_ColorsUavCount) },
				{ }));
	}
}

void SubGraphTestPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	pipeline.unloadPipeline(getStaticClassId(), Swap_ColorsPipelineId);
}

void SubGraphTestPass::createGlobalResourcesHandles()
{
}

void SubGraphTestPass::deleteGlobalResourcesHandles()
{
}

void SubGraphTestPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void SubGraphTestPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<SubGraphTestPassData>();

}

void SubGraphTestPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void SubGraphTestPass::addSubGraphTestPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.SubGraphTest;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<SubGraphTestPassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		// Texture: Inner_Exported_Tex
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2d;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			RenderGraphTextureDesc formatSrcDesc;
			build.getTextureDesc(passData.textureTest, &formatSrcDesc);
			desc.format = formatSrcDesc.format;
			RenderGraphTextureDesc sizeSrcDesc;
			build.getTextureDesc(passData.textureTest, &sizeSrcDesc);
			desc.width = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
			desc.height = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
			passData.textureInner_Exported_Tex = build.create("Texture Inner_Exported_Tex", desc);
		}

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("SubGraphTest InitPass", InitPass);

	// Copy Resource: Inner_Copy_Import_To_Export
	auto& Inner_Copy_Import_To_ExportPass = [&](RenderGraphBuild& build)
	{
		RenderGraphTextureDesc srcDesc;
		build.getTextureDesc(passData.textureTest, &srcDesc);

		passData.textureInner_Exported_Tex = build.writeAndUpdate(passData.textureInner_Exported_Tex, RenderBindFlags::UnorderedAccess);
		passData.textureTest = build.read(passData.textureTest, RenderBindFlags::ShaderResource);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderPoint point = {};
			RenderBox box	  = {};
			box.w = srcDesc.width;
			box.h = srcDesc.height;

			commandList.copyTexture(
				registry.getTexture(passData.textureInner_Exported_Tex), 0, point,
				registry.getTexture(passData.textureTest), 0, box);
		};
	};

	renderGraph.addPassCallback("SubGraphTest Inner_Copy_Import_To_Export", Inner_Copy_Import_To_ExportPass);

	// Compute Shader: Swap_Colors
	auto& Swap_ColorsPass = [&](RenderGraphBuild& build)
	{
		auto textureOutput = passData.textureInner_Exported_Tex = build.write(passData.textureInner_Exported_Tex, RenderBindFlags::UnorderedAccess);

		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureTest, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(SubGraphTestPipeline::getStaticClassId(), SubGraphTestPipeline::Swap_ColorsPipelineId);

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

	renderGraph.addPassCallback("SubGraphTest Swap_Colors", Swap_ColorsPass);
}

} // namespace halcyon
