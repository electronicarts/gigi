//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "simpleRT_inlinePass.h"
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

void simpleRT_inlinePipeline::loadPipeline(ShaderPipelines& pipeline)
{

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			DoRTPipelineId,
			"Shaders/simpleRT_inline/DoRT",
			ShaderSignatureDesc(
				{ ShaderParameter(DoRTSrvCount, DoRTUavCount) },
				{ }));
	}
}

void simpleRT_inlinePipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	pipeline.unloadPipeline(getStaticClassId(), DoRTPipelineId);
}

void simpleRT_inlinePass::createGlobalResourcesHandles()
{
}

void simpleRT_inlinePass::deleteGlobalResourcesHandles()
{
}

void simpleRT_inlinePass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void simpleRT_inlinePass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<simpleRT_inlinePassData>();

}

void simpleRT_inlinePass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void simpleRT_inlinePass::addsimpleRT_inlinePass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.simpleRT_inline;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<simpleRT_inlinePassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		// Texture: Texture
		// The texture that is rendered to
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2d;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			desc.format = RenderFormat::R8G8B8A8_SRGB;
			desc.width = ((1 + 0)512) / 1 + 0;
			desc.height = ((1 + 0)512) / 1 + 0;
			passData.textureTexture = build.create("Texture Texture", desc);
		}

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("simpleRT_inline InitPass", InitPass);

	// Shader Constants: _SimpleRTCS_0CB
	{
		m_cb__SimpleRTCS_0CB.cameraPos = settings.cameraPos;
		m_cb__SimpleRTCS_0CB.clipToWorld = settings.clipToWorld;
		m_cb__SimpleRTCS_0CB.depthNearPlane = settings.depthNearPlane;
		m_cb__SimpleRTCS_0CB.hitColor = settings.hitColor;
		m_cb__SimpleRTCS_0CB.missColor = settings.missColor;
	}

	// Compute Shader: DoRT
	auto& DoRTPass = [&](RenderGraphBuild& build)
	{
		auto textureg_texture = passData.textureTexture = build.write(passData.textureTexture, RenderBindFlags::UnorderedAccess);
		auto bufferg_vertexBuffer = passData.bufferScene_VB = build.read(passData.bufferScene_VB, RenderBindFlags::ShaderResource);

		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureTexture, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(simpleRT_inlinePipeline::getStaticClassId(), simpleRT_inlinePipeline::DoRTPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__SimpleRTCS_0CB);
			RenderBindingShaderResourceView srvs[] = {
				defaultBufferView(registry, bufferg_vertexBuffer),
				};
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dRwView(registry, textureg_texture),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, constantsOffset, registry.createShaderViews(srvs, uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	if (settings.enabled)
		renderGraph.addPassCallback("simpleRT_inline DoRT", DoRTPass);
}

} // namespace halcyon
