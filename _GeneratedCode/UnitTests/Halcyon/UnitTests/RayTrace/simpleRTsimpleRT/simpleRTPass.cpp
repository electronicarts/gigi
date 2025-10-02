//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "simpleRTPass.h"
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

	unsigned int simpleRTPass::m_variable__missindex_SimpleRTMiss = 0;
	unsigned int simpleRTPass::m_variable__chindex_SimpleRTClosestHit = 0;
	unsigned int simpleRTPass::m_variable__rt_hit_group_count = 0;

void simpleRTPipeline::loadPipeline(ShaderPipelines& pipeline)
{
	if (!pipeline.isRayTracingSupported())
	{
		return;
	}

	ShaderSignatureDesc emptyShaderSignature({});
	ShaderSignatureDesc hitShaderSignature({ getMaterialShaderParameter(), getMeshShaderParameter(), getCameraShaderParameter() });
	pipeline.loadRayTracingMissProgram(getStaticClassId(), SimpleRTMissPipelineId, "Shaders/simpleRT/SimpleRTMiss", emptyShaderSignature);
	pipeline.loadRayTracingHitProgram(getStaticClassId(), SimpleRTClosestHitPipelineId, "", "Shaders/simpleRT/SimpleRTClosestHit", hitShaderSignature);
	pipeline.loadRayTracingRayGenProgram(getStaticClassId(), DoRTPipelineId, "Shaders/simpleRT/DoRT", emptyShaderSignature);
}

void simpleRTPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	if (!pipeline.isRayTracingSupported())
	{
		return;
	}
	pipeline.unloadPipeline(getStaticClassId(), SimpleRTMissPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), SimpleRTClosestHitPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), DoRTPipelineId);
}

void simpleRTPass::createGlobalResourcesHandles()
{
}

void simpleRTPass::deleteGlobalResourcesHandles()
{
}

void simpleRTPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void simpleRTPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<simpleRTPassData>();

}

void simpleRTPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void simpleRTPass::addsimpleRTPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.simpleRT;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<simpleRTPassData>();

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
			desc.width = ((1 + 0)1280) / 1 + 0;
			desc.height = ((1 + 0)720) / 1 + 0;
			passData.textureTexture = build.create("Texture Texture", desc);
		}

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("simpleRT InitPass", InitPass);
	m_variable__missindex_SimpleRTMiss = pipelines.getRtMissShaderIndex(simpleRTPipeline::getStaticClassId(), simpleRTPipeline::SimpleRTMissPipelineId);
	m_variable__chindex_SimpleRTClosestHit = pipelines.getRtHitShaderIndex(simpleRTPipeline::getStaticClassId(), simpleRTPipeline::SimpleRTClosestHitPipelineId);
	m_variable__rt_hit_group_count = pipelines.getRtHitShaderRecordStride();

	// Shader Constants: _SimpleRTRayGenCB
	{
		m_cb__SimpleRTRayGenCB.cameraPos = settings.cameraPos;
		m_cb__SimpleRTRayGenCB.clipToWorld = settings.clipToWorld;
		m_cb__SimpleRTRayGenCB.depthNearPlane = settings.depthNearPlane;
		m_cb__SimpleRTRayGenCB.hitColor = settings.hitColor;
		m_cb__SimpleRTRayGenCB.missColor = settings.missColor;
	}

	// Ray Shader: DoRT
	auto& DoRTPass = [&](RenderGraphBuild& build)
	{
		auto rtShaderTable = scope.get<RenderGraphRtScene>().rtShaderTable;
		const uint32 rayGenShaderIndex	   = pipelines.getRtRayGenShaderIndex(simpleRTPipeline::getStaticClassId(), simpleRTPipeline::DoRTPipelineId);
		auto textureg_texture = passData.textureTexture = build.write(passData.textureTexture, RenderBindFlags::UnorderedAccess);
		auto bufferg_scene = passData.bufferScene;

		glm::uvec2 rtDim;
		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureTexture, &sizeSrcDesc);
		rtDim.x = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		rtDim.y = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;

		auto rtPipelineState = pipelines.rtPipelineState();
		const RayTracingHelpers::GlobalShaderArgumentData rtGlobalShaderArgumentData = RayTracingHelpers::setupGlobalShaderArgumentPass(scope, build, pipelines);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			ShaderArgument raytracingArgument = RayTracingHelpers::createGlobalShaderArgument(registry, rtGlobalShaderArgumentData);
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__SimpleRTRayGenCB);
			RenderBindingShaderResourceView srvs[] = {
				build::rayTracingAcceleration(bufferg_scene),
				};
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dRwView(registry, textureg_texture),
				};
			commandList.rayTrace(
				rtPipelineState,
				{ ShaderArgument(dynamicConstants.buffer, constantsOffset, registry.createShaderViews(srvs)),
					ShaderArgument(RenderResourceHandle(), registry.createShaderViews(uavs)),
					raytracingArgument },
				rtShaderTable,
				rtDim.x, rtDim.y,
				rayGenShaderIndex);
		};
	};

	if (settings.enabled)
		renderGraph.addPassCallback("simpleRT DoRT", DoRTPass);
}

} // namespace halcyon
