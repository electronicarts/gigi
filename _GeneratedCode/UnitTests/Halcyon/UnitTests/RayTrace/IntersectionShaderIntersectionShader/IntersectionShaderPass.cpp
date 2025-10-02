//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "IntersectionShaderPass.h"
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

	unsigned int IntersectionShaderPass::m_variable__missindex_Miss = 0;
	unsigned int IntersectionShaderPass::m_variable__chindex_ClosestHit = 0;
	unsigned int IntersectionShaderPass::m_variable__rt_hit_group_count = 0;

void IntersectionShaderPipeline::loadPipeline(ShaderPipelines& pipeline)
{
	if (!pipeline.isRayTracingSupported())
	{
		return;
	}

	ShaderSignatureDesc emptyShaderSignature({});
	ShaderSignatureDesc hitShaderSignature({ getMaterialShaderParameter(), getMeshShaderParameter(), getCameraShaderParameter() });
	pipeline.loadRayTracingMissProgram(getStaticClassId(), MissPipelineId, "Shaders/IntersectionShader/Miss", emptyShaderSignature);
	pipeline.loadRayTracingHitProgram(getStaticClassId(), ClosestHitPipelineId, "", "Shaders/IntersectionShader/ClosestHit", hitShaderSignature);
	pipeline.loadRayTracingRayGenProgram(getStaticClassId(), Do_RTPipelineId, "Shaders/IntersectionShader/Do_RT", emptyShaderSignature);
}

void IntersectionShaderPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	if (!pipeline.isRayTracingSupported())
	{
		return;
	}
	pipeline.unloadPipeline(getStaticClassId(), MissPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), ClosestHitPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), Do_RTPipelineId);
}

void IntersectionShaderPass::createGlobalResourcesHandles()
{
}

void IntersectionShaderPass::deleteGlobalResourcesHandles()
{
}

void IntersectionShaderPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void IntersectionShaderPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<IntersectionShaderPassData>();

}

void IntersectionShaderPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void IntersectionShaderPass::addIntersectionShaderPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.IntersectionShader;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<IntersectionShaderPassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		// Texture: Output
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2d;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			desc.format = RenderFormat::R8G8B8A8_SRGB;
			desc.width = ((1 + 0)256) / 1 + 0;
			desc.height = ((1 + 0)256) / 1 + 0;
			passData.textureOutput = build.create("Texture Output", desc);
		}

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("IntersectionShader InitPass", InitPass);
	m_variable__missindex_Miss = pipelines.getRtMissShaderIndex(IntersectionShaderPipeline::getStaticClassId(), IntersectionShaderPipeline::MissPipelineId);
	m_variable__chindex_ClosestHit = pipelines.getRtHitShaderIndex(IntersectionShaderPipeline::getStaticClassId(), IntersectionShaderPipeline::ClosestHitPipelineId);
	m_variable__rt_hit_group_count = pipelines.getRtHitShaderRecordStride();

	// Shader Constants: _Ray_GenCB
	{
		m_cb__Ray_GenCB.CameraPos = settings.CameraPos;
		m_cb__Ray_GenCB.InvViewProjMtx = settings.InvViewProjMtx;
	}

	// Ray Shader: Do_RT
	auto& Do_RTPass = [&](RenderGraphBuild& build)
	{
		auto rtShaderTable = scope.get<RenderGraphRtScene>().rtShaderTable;
		const uint32 rayGenShaderIndex	   = pipelines.getRtRayGenShaderIndex(IntersectionShaderPipeline::getStaticClassId(), IntersectionShaderPipeline::Do_RTPipelineId);
		auto bufferScene = passData.bufferAABBs;
		auto bufferAABBs = passData.bufferAABBsSRV = build.read(passData.bufferAABBsSRV, RenderBindFlags::ShaderResource);
		auto textureOutput = passData.textureOutput = build.write(passData.textureOutput, RenderBindFlags::UnorderedAccess);

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
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__Ray_GenCB);
			RenderBindingShaderResourceView srvs[] = {
				build::rayTracingAcceleration(bufferScene),
				defaultBufferView(registry, bufferAABBs),
				};
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dRwView(registry, textureOutput),
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

	renderGraph.addPassCallback("IntersectionShader Do_RT", Do_RTPass);
}

} // namespace halcyon
