//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "TwoRayGensSubgraphPass.h"
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

	unsigned int TwoRayGensSubgraphPass::m_variable__missindex_A_TwoRayGensMiss1 = 0;
	unsigned int TwoRayGensSubgraphPass::m_variable__chindex_A_TwoRayGensClosestHit1 = 0;
	unsigned int TwoRayGensSubgraphPass::m_variable__missindex_B_TwoRayGensMiss2A = 0;
	unsigned int TwoRayGensSubgraphPass::m_variable__missindex_B_TwoRayGensMiss2B = 0;
	unsigned int TwoRayGensSubgraphPass::m_variable__chindex_B_TwoRayGensClosestHit2 = 0;
	unsigned int TwoRayGensSubgraphPass::m_variable__rt_hit_group_count = 0;

void TwoRayGensSubgraphPipeline::loadPipeline(ShaderPipelines& pipeline)
{
	if (!pipeline.isRayTracingSupported())
	{
		return;
	}

	ShaderSignatureDesc emptyShaderSignature({});
	ShaderSignatureDesc hitShaderSignature({ getMaterialShaderParameter(), getMeshShaderParameter(), getCameraShaderParameter() });
	pipeline.loadRayTracingMissProgram(getStaticClassId(), A_TwoRayGensMiss1PipelineId, "Shaders/TwoRayGensSubgraph/A_TwoRayGensMiss1", emptyShaderSignature);
	pipeline.loadRayTracingHitProgram(getStaticClassId(), A_TwoRayGensClosestHit1PipelineId, "", "Shaders/TwoRayGensSubgraph/A_TwoRayGensClosestHit1", hitShaderSignature);
	pipeline.loadRayTracingMissProgram(getStaticClassId(), B_TwoRayGensMiss2APipelineId, "Shaders/TwoRayGensSubgraph/B_TwoRayGensMiss2A", emptyShaderSignature);
	pipeline.loadRayTracingMissProgram(getStaticClassId(), B_TwoRayGensMiss2BPipelineId, "Shaders/TwoRayGensSubgraph/B_TwoRayGensMiss2B", emptyShaderSignature);
	pipeline.loadRayTracingHitProgram(getStaticClassId(), B_TwoRayGensClosestHit2PipelineId, "", "Shaders/TwoRayGensSubgraph/B_TwoRayGensClosestHit2", hitShaderSignature);
	pipeline.loadRayTracingRayGenProgram(getStaticClassId(), A_DoRT1PipelineId, "Shaders/TwoRayGensSubgraph/A_DoRT1", emptyShaderSignature);
	pipeline.loadRayTracingRayGenProgram(getStaticClassId(), B_DoRT2PipelineId, "Shaders/TwoRayGensSubgraph/B_DoRT2", emptyShaderSignature);
}

void TwoRayGensSubgraphPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	if (!pipeline.isRayTracingSupported())
	{
		return;
	}
	pipeline.unloadPipeline(getStaticClassId(), A_TwoRayGensMiss1PipelineId);
	pipeline.unloadPipeline(getStaticClassId(), A_TwoRayGensClosestHit1PipelineId);
	pipeline.unloadPipeline(getStaticClassId(), B_TwoRayGensMiss2APipelineId);
	pipeline.unloadPipeline(getStaticClassId(), B_TwoRayGensMiss2BPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), B_TwoRayGensClosestHit2PipelineId);
	pipeline.unloadPipeline(getStaticClassId(), A_DoRT1PipelineId);
	pipeline.unloadPipeline(getStaticClassId(), B_DoRT2PipelineId);
}

void TwoRayGensSubgraphPass::createGlobalResourcesHandles()
{
}

void TwoRayGensSubgraphPass::deleteGlobalResourcesHandles()
{
}

void TwoRayGensSubgraphPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void TwoRayGensSubgraphPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<TwoRayGensSubgraphPassData>();

}

void TwoRayGensSubgraphPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void TwoRayGensSubgraphPass::addTwoRayGensSubgraphPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.TwoRayGensSubgraph;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<TwoRayGensSubgraphPassData>();

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
			desc.width = ((1 + 0)256) / 1 + 0;
			desc.height = ((1 + 0)256) / 1 + 0;
			passData.textureTexture = build.create("Texture Texture", desc);
		}

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("TwoRayGensSubgraph InitPass", InitPass);
	m_variable__missindex_A_TwoRayGensMiss1 = pipelines.getRtMissShaderIndex(TwoRayGensSubgraphPipeline::getStaticClassId(), TwoRayGensSubgraphPipeline::A_TwoRayGensMiss1PipelineId);
	m_variable__chindex_A_TwoRayGensClosestHit1 = pipelines.getRtHitShaderIndex(TwoRayGensSubgraphPipeline::getStaticClassId(), TwoRayGensSubgraphPipeline::A_TwoRayGensClosestHit1PipelineId);
	m_variable__missindex_B_TwoRayGensMiss2A = pipelines.getRtMissShaderIndex(TwoRayGensSubgraphPipeline::getStaticClassId(), TwoRayGensSubgraphPipeline::B_TwoRayGensMiss2APipelineId);
	m_variable__missindex_B_TwoRayGensMiss2B = pipelines.getRtMissShaderIndex(TwoRayGensSubgraphPipeline::getStaticClassId(), TwoRayGensSubgraphPipeline::B_TwoRayGensMiss2BPipelineId);
	m_variable__chindex_B_TwoRayGensClosestHit2 = pipelines.getRtHitShaderIndex(TwoRayGensSubgraphPipeline::getStaticClassId(), TwoRayGensSubgraphPipeline::B_TwoRayGensClosestHit2PipelineId);
	m_variable__rt_hit_group_count = pipelines.getRtHitShaderRecordStride();

	// Shader Constants: _A_TwoRayGens1CB
	{
		m_cb__A_TwoRayGens1CB.cameraPos = settings.cameraPos;
		m_cb__A_TwoRayGens1CB.clipToWorld = settings.clipToWorld;
		m_cb__A_TwoRayGens1CB.depthNearPlane = settings.depthNearPlane;
	}

	// Ray Shader: A_DoRT1
	auto& A_DoRT1Pass = [&](RenderGraphBuild& build)
	{
		auto rtShaderTable = scope.get<RenderGraphRtScene>().rtShaderTable;
		const uint32 rayGenShaderIndex	   = pipelines.getRtRayGenShaderIndex(TwoRayGensSubgraphPipeline::getStaticClassId(), TwoRayGensSubgraphPipeline::A_DoRT1PipelineId);
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
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__A_TwoRayGens1CB);
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

	renderGraph.addPassCallback("TwoRayGensSubgraph A_DoRT1", A_DoRT1Pass);

	// Shader Constants: _B_TwoRayGens2CB
	{
		m_cb__B_TwoRayGens2CB.cameraPos = settings.cameraPos;
		m_cb__B_TwoRayGens2CB.clipToWorld = settings.clipToWorld;
		m_cb__B_TwoRayGens2CB.depthNearPlane = settings.depthNearPlane;
	}

	// Ray Shader: B_DoRT2
	auto& B_DoRT2Pass = [&](RenderGraphBuild& build)
	{
		auto rtShaderTable = scope.get<RenderGraphRtScene>().rtShaderTable;
		const uint32 rayGenShaderIndex	   = pipelines.getRtRayGenShaderIndex(TwoRayGensSubgraphPipeline::getStaticClassId(), TwoRayGensSubgraphPipeline::B_DoRT2PipelineId);
		auto textureg_texture = passData.textureTexture = build.write(passData.textureTexture, RenderBindFlags::UnorderedAccess);
		auto bufferg_scene = passData.bufferScene;
		auto textureg_blueChannel = passData.textureBlueChannel = build.read(passData.textureBlueChannel, RenderBindFlags::ShaderResource);

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
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__B_TwoRayGens2CB);
			RenderBindingShaderResourceView srvs[] = {
				build::rayTracingAcceleration(bufferg_scene),
				defaultTexture2dView(registry, textureg_blueChannel),
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

	renderGraph.addPassCallback("TwoRayGensSubgraph B_DoRT2", B_DoRT2Pass);
}

} // namespace halcyon
