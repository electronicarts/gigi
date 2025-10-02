//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "AnyHitSimplePass.h"
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

	unsigned int AnyHitSimplePass::m_variable__missindex_Miss = 0;
	unsigned int AnyHitSimplePass::m_variable__chindex_ClosestHit = 0;
	unsigned int AnyHitSimplePass::m_variable__rt_hit_group_count = 0;

void AnyHitSimplePipeline::loadPipeline(ShaderPipelines& pipeline)
{
	if (!pipeline.isRayTracingSupported())
	{
		return;
	}

	ShaderSignatureDesc emptyShaderSignature({});
	ShaderSignatureDesc hitShaderSignature({ getMaterialShaderParameter(), getMeshShaderParameter(), getCameraShaderParameter() });
	pipeline.loadRayTracingMissProgram(getStaticClassId(), MissPipelineId, "Shaders/AnyHitSimple/Miss", emptyShaderSignature);
	pipeline.loadRayTracingHitProgram(getStaticClassId(), ClosestHitPipelineId, "", "Shaders/AnyHitSimple/ClosestHit", hitShaderSignature);
	pipeline.loadRayTracingRayGenProgram(getStaticClassId(), DoRTPipelineId, "Shaders/AnyHitSimple/DoRT", emptyShaderSignature);
}

void AnyHitSimplePipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	if (!pipeline.isRayTracingSupported())
	{
		return;
	}
	pipeline.unloadPipeline(getStaticClassId(), MissPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), ClosestHitPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), DoRTPipelineId);
}

void AnyHitSimplePass::createGlobalResourcesHandles()
{
}

void AnyHitSimplePass::deleteGlobalResourcesHandles()
{
}

void AnyHitSimplePass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void AnyHitSimplePass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<AnyHitSimplePassData>();

}

void AnyHitSimplePass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void AnyHitSimplePass::addAnyHitSimplePass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.AnyHitSimple;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<AnyHitSimplePassData>();

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
	renderGraph.addPassCallback("AnyHitSimple InitPass", InitPass);
	m_variable__missindex_Miss = pipelines.getRtMissShaderIndex(AnyHitSimplePipeline::getStaticClassId(), AnyHitSimplePipeline::MissPipelineId);
	m_variable__chindex_ClosestHit = pipelines.getRtHitShaderIndex(AnyHitSimplePipeline::getStaticClassId(), AnyHitSimplePipeline::ClosestHitPipelineId);
	m_variable__rt_hit_group_count = pipelines.getRtHitShaderRecordStride();

	// Shader Constants: _RayGenCB
	{
		m_cb__RayGenCB.CameraPos = settings.CameraPos;
		m_cb__RayGenCB.InvViewProjMtx = settings.InvViewProjMtx;
		m_cb__RayGenCB.depthNearPlane = settings.depthNearPlane;
	}

	// Ray Shader: DoRT
	auto& DoRTPass = [&](RenderGraphBuild& build)
	{
		auto rtShaderTable = scope.get<RenderGraphRtScene>().rtShaderTable;
		const uint32 rayGenShaderIndex	   = pipelines.getRtRayGenShaderIndex(AnyHitSimplePipeline::getStaticClassId(), AnyHitSimplePipeline::DoRTPipelineId);
		auto textureoutput = passData.textureOutput = build.write(passData.textureOutput, RenderBindFlags::UnorderedAccess);
		auto bufferscene = passData.bufferScene;

		glm::uvec2 rtDim;
		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureOutput, &sizeSrcDesc);
		rtDim.x = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		rtDim.y = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;

		auto rtPipelineState = pipelines.rtPipelineState();
		const RayTracingHelpers::GlobalShaderArgumentData rtGlobalShaderArgumentData = RayTracingHelpers::setupGlobalShaderArgumentPass(scope, build, pipelines);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			ShaderArgument raytracingArgument = RayTracingHelpers::createGlobalShaderArgument(registry, rtGlobalShaderArgumentData);
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__RayGenCB);
			RenderBindingShaderResourceView srvs[] = {
				build::rayTracingAcceleration(bufferscene),
				};
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dRwView(registry, textureoutput),
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

	renderGraph.addPassCallback("AnyHitSimple DoRT", DoRTPass);
}

} // namespace halcyon
