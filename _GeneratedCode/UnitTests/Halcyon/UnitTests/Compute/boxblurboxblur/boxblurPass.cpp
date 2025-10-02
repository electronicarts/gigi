//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "boxblurPass.h"
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

void boxblurPipeline::loadPipeline(ShaderPipelines& pipeline)
{

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			BlurHPipelineId,
			"Shaders/boxblur/BlurH",
			ShaderSignatureDesc(
				{ ShaderParameter(BlurHSrvCount, BlurHUavCount) },
				{ }));
	}

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			BlurVPipelineId,
			"Shaders/boxblur/BlurV",
			ShaderSignatureDesc(
				{ ShaderParameter(BlurVSrvCount, BlurVUavCount) },
				{ }));
	}
}

void boxblurPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	pipeline.unloadPipeline(getStaticClassId(), BlurHPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), BlurVPipelineId);
}

void boxblurPass::createGlobalResourcesHandles()
{
}

void boxblurPass::deleteGlobalResourcesHandles()
{
}

void boxblurPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void boxblurPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<boxblurPassData>();

}

void boxblurPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void boxblurPass::addboxblurPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.boxblur;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<boxblurPassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		// Texture: PingPongTexture
		// An internal texture used during the blurring process
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2d;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			RenderGraphTextureDesc formatSrcDesc;
			build.getTextureDesc(passData.textureInputTexture, &formatSrcDesc);
			desc.format = formatSrcDesc.format;
			RenderGraphTextureDesc sizeSrcDesc;
			build.getTextureDesc(passData.textureInputTexture, &sizeSrcDesc);
			desc.width = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
			desc.height = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
			passData.texturePingPongTexture = build.create("Texture PingPongTexture", desc);
		}

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("boxblur InitPass", InitPass);

	// Shader Constants: _BoxBlur_0CB
	{
		m_cb__BoxBlur_0CB.radius = settings.radius;
		m_cb__BoxBlur_0CB.sRGB = settings.sRGB;
	}

	// Compute Shader: BlurH
	// Horizontal blur pass
	auto& BlurHPass = [&](RenderGraphBuild& build)
	{
		auto textureInput = passData.textureInputTexture = build.read(passData.textureInputTexture, RenderBindFlags::ShaderResource);
		auto textureOutput = passData.texturePingPongTexture = build.write(passData.texturePingPongTexture, RenderBindFlags::UnorderedAccess);

		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureInputTexture, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(boxblurPipeline::getStaticClassId(), boxblurPipeline::BlurHPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__BoxBlur_0CB);
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

	if (settings.enabled)
		renderGraph.addPassCallback("boxblur BlurH", BlurHPass);

	// Shader Constants: _BoxBlur_1CB
	{
		m_cb__BoxBlur_1CB.radius = settings.radius;
		m_cb__BoxBlur_1CB.sRGB = settings.sRGB;
	}

	// Compute Shader: BlurV
	// Vertical blur pass
	auto& BlurVPass = [&](RenderGraphBuild& build)
	{
		auto textureInput = passData.texturePingPongTexture = build.read(passData.texturePingPongTexture, RenderBindFlags::ShaderResource);
		auto textureOutput = passData.textureInputTexture = build.write(passData.textureInputTexture, RenderBindFlags::UnorderedAccess);

		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureInputTexture, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(boxblurPipeline::getStaticClassId(), boxblurPipeline::BlurVPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__BoxBlur_1CB);
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

	if (settings.enabled)
		renderGraph.addPassCallback("boxblur BlurV", BlurVPass);
}

} // namespace halcyon
