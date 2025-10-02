//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "TwoRayGensPass.h"
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

	unsigned int TwoRayGensPass::m_variable__missindex_TwoRayGensMiss1 = 0;
	unsigned int TwoRayGensPass::m_variable__chindex_TwoRayGensClosestHit1 = 0;
	unsigned int TwoRayGensPass::m_variable__missindex_TwoRayGensMiss2A = 0;
	unsigned int TwoRayGensPass::m_variable__missindex_TwoRayGensMiss2B = 0;
	unsigned int TwoRayGensPass::m_variable__chindex_TwoRayGensClosestHit2 = 0;
	unsigned int TwoRayGensPass::m_variable__rt_hit_group_count = 0;

void TwoRayGensPipeline::loadPipeline(ShaderPipelines& pipeline)
{
	if (!pipeline.isRayTracingSupported())
	{
		return;
	}

	ShaderSignatureDesc emptyShaderSignature({});
	ShaderSignatureDesc hitShaderSignature({ getMaterialShaderParameter(), getMeshShaderParameter(), getCameraShaderParameter() });
	pipeline.loadRayTracingMissProgram(getStaticClassId(), TwoRayGensMiss1PipelineId, "Shaders/TwoRayGens/TwoRayGensMiss1", emptyShaderSignature);
	pipeline.loadRayTracingHitProgram(getStaticClassId(), TwoRayGensClosestHit1PipelineId, "", "Shaders/TwoRayGens/TwoRayGensClosestHit1", hitShaderSignature);
	pipeline.loadRayTracingMissProgram(getStaticClassId(), TwoRayGensMiss2APipelineId, "Shaders/TwoRayGens/TwoRayGensMiss2A", emptyShaderSignature);
	pipeline.loadRayTracingMissProgram(getStaticClassId(), TwoRayGensMiss2BPipelineId, "Shaders/TwoRayGens/TwoRayGensMiss2B", emptyShaderSignature);
	pipeline.loadRayTracingHitProgram(getStaticClassId(), TwoRayGensClosestHit2PipelineId, "", "Shaders/TwoRayGens/TwoRayGensClosestHit2", hitShaderSignature);
	pipeline.loadRayTracingRayGenProgram(getStaticClassId(), DoRT1PipelineId, "Shaders/TwoRayGens/DoRT1", emptyShaderSignature);
	pipeline.loadRayTracingRayGenProgram(getStaticClassId(), DoRT2PipelineId, "Shaders/TwoRayGens/DoRT2", emptyShaderSignature);
}

void TwoRayGensPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	if (!pipeline.isRayTracingSupported())
	{
		return;
	}
	pipeline.unloadPipeline(getStaticClassId(), TwoRayGensMiss1PipelineId);
	pipeline.unloadPipeline(getStaticClassId(), TwoRayGensClosestHit1PipelineId);
	pipeline.unloadPipeline(getStaticClassId(), TwoRayGensMiss2APipelineId);
	pipeline.unloadPipeline(getStaticClassId(), TwoRayGensMiss2BPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), TwoRayGensClosestHit2PipelineId);
	pipeline.unloadPipeline(getStaticClassId(), DoRT1PipelineId);
	pipeline.unloadPipeline(getStaticClassId(), DoRT2PipelineId);
}

void TwoRayGensPass::createGlobalResourcesHandles()
{
}

void TwoRayGensPass::deleteGlobalResourcesHandles()
{
}

void TwoRayGensPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void TwoRayGensPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<TwoRayGensPassData>();

}

void TwoRayGensPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void TwoRayGensPass::addTwoRayGensPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.TwoRayGens;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<TwoRayGensPassData>();

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
	renderGraph.addPassCallback("TwoRayGens InitPass", InitPass);
	m_variable__missindex_TwoRayGensMiss1 = pipelines.getRtMissShaderIndex(TwoRayGensPipeline::getStaticClassId(), TwoRayGensPipeline::TwoRayGensMiss1PipelineId);
	m_variable__chindex_TwoRayGensClosestHit1 = pipelines.getRtHitShaderIndex(TwoRayGensPipeline::getStaticClassId(), TwoRayGensPipeline::TwoRayGensClosestHit1PipelineId);
	m_variable__missindex_TwoRayGensMiss2A = pipelines.getRtMissShaderIndex(TwoRayGensPipeline::getStaticClassId(), TwoRayGensPipeline::TwoRayGensMiss2APipelineId);
	m_variable__missindex_TwoRayGensMiss2B = pipelines.getRtMissShaderIndex(TwoRayGensPipeline::getStaticClassId(), TwoRayGensPipeline::TwoRayGensMiss2BPipelineId);
	m_variable__chindex_TwoRayGensClosestHit2 = pipelines.getRtHitShaderIndex(TwoRayGensPipeline::getStaticClassId(), TwoRayGensPipeline::TwoRayGensClosestHit2PipelineId);
	m_variable__rt_hit_group_count = pipelines.getRtHitShaderRecordStride();

	// Shader Constants: _TwoRayGens1CB
	{
		m_cb__TwoRayGens1CB.cameraPos = settings.cameraPos;
		m_cb__TwoRayGens1CB.clipToWorld = settings.clipToWorld;
		m_cb__TwoRayGens1CB.depthNearPlane = settings.depthNearPlane;
	}

	// Ray Shader: DoRT1
	auto& DoRT1Pass = [&](RenderGraphBuild& build)
	{
		auto rtShaderTable = scope.get<RenderGraphRtScene>().rtShaderTable;
		const uint32 rayGenShaderIndex	   = pipelines.getRtRayGenShaderIndex(TwoRayGensPipeline::getStaticClassId(), TwoRayGensPipeline::DoRT1PipelineId);
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
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__TwoRayGens1CB);
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

	renderGraph.addPassCallback("TwoRayGens DoRT1", DoRT1Pass);

	// Shader Constants: _TwoRayGens2CB
	{
		m_cb__TwoRayGens2CB.cameraPos = settings.cameraPos;
		m_cb__TwoRayGens2CB.clipToWorld = settings.clipToWorld;
		m_cb__TwoRayGens2CB.depthNearPlane = settings.depthNearPlane;
	}

	// Ray Shader: DoRT2
	auto& DoRT2Pass = [&](RenderGraphBuild& build)
	{
		auto rtShaderTable = scope.get<RenderGraphRtScene>().rtShaderTable;
		const uint32 rayGenShaderIndex	   = pipelines.getRtRayGenShaderIndex(TwoRayGensPipeline::getStaticClassId(), TwoRayGensPipeline::DoRT2PipelineId);
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
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__TwoRayGens2CB);
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

	renderGraph.addPassCallback("TwoRayGens DoRT2", DoRT2Pass);
}

} // namespace halcyon
