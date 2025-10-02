//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "SubInSubPass.h"
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

void SubInSubPipeline::loadPipeline(ShaderPipelines& pipeline)
{

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			Inner1_Inner2_Rotate_ColorsPipelineId,
			"Shaders/SubInSub/Inner1_Inner2_Rotate_Colors",
			ShaderSignatureDesc(
				{ ShaderParameter(Inner1_Inner2_Rotate_ColorsSrvCount, Inner1_Inner2_Rotate_ColorsUavCount) },
				{ }));
	}
}

void SubInSubPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	pipeline.unloadPipeline(getStaticClassId(), Inner1_Inner2_Rotate_ColorsPipelineId);
}

void SubInSubPass::createGlobalResourcesHandles()
{
}

void SubInSubPass::deleteGlobalResourcesHandles()
{
}

void SubInSubPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void SubInSubPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<SubInSubPassData>();

}

void SubInSubPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void SubInSubPass::addSubInSubPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.SubInSub;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<SubInSubPassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		// Texture: Inner1_Inner2_Output
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2d;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			RenderGraphTextureDesc formatSrcDesc;
			build.getTextureDesc(passData.textureInput, &formatSrcDesc);
			desc.format = formatSrcDesc.format;
			RenderGraphTextureDesc sizeSrcDesc;
			build.getTextureDesc(passData.textureInput, &sizeSrcDesc);
			desc.width = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
			desc.height = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
			passData.textureInner1_Inner2_Output = build.create("Texture Inner1_Inner2_Output", desc);
		}

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("SubInSub InitPass", InitPass);

	// Shader Constants: _Inner1_Inner2_Inner2CS_0CB
	{
		m_cb__Inner1_Inner2_Inner2CS_0CB.Inner1_Inner1Mult = settings.Inner1_Inner1Mult;
	}

	// Compute Shader: Inner1_Inner2_Rotate_Colors
	auto& Inner1_Inner2_Rotate_ColorsPass = [&](RenderGraphBuild& build)
	{
		auto textureInput = passData.textureInput = build.read(passData.textureInput, RenderBindFlags::ShaderResource);
		auto textureOutput = passData.textureInner1_Inner2_Output = build.write(passData.textureInner1_Inner2_Output, RenderBindFlags::UnorderedAccess);

		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureInput, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(SubInSubPipeline::getStaticClassId(), SubInSubPipeline::Inner1_Inner2_Rotate_ColorsPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__Inner1_Inner2_Inner2CS_0CB);
			RenderBindingShaderResourceView srvs[] = {
				defaultTexture2dView(registry, textureInput),
				};
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dRwView(registry, textureOutput),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, constantsOffset, registry.createShaderViews(srvs, uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("SubInSub Inner1_Inner2_Rotate_Colors", Inner1_Inner2_Rotate_ColorsPass);

	// Copy Resource: Inner1_Inner2_Copy_Back
	auto& Inner1_Inner2_Copy_BackPass = [&](RenderGraphBuild& build)
	{
		RenderGraphTextureDesc srcDesc;
		build.getTextureDesc(passData.textureInner1_Inner2_Output, &srcDesc);

		passData.textureInput = build.writeAndUpdate(passData.textureInput, RenderBindFlags::UnorderedAccess);
		passData.textureInner1_Inner2_Output = build.read(passData.textureInner1_Inner2_Output, RenderBindFlags::ShaderResource);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderPoint point = {};
			RenderBox box	  = {};
			box.w = srcDesc.width;
			box.h = srcDesc.height;

			commandList.copyTexture(
				registry.getTexture(passData.textureInput), 0, point,
				registry.getTexture(passData.textureInner1_Inner2_Output), 0, box);
		};
	};

	renderGraph.addPassCallback("SubInSub Inner1_Inner2_Copy_Back", Inner1_Inner2_Copy_BackPass);
}

} // namespace halcyon
