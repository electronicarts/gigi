//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "Mips_RGS_2DPass.h"
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

	unsigned int Mips_RGS_2DPass::m_variable__missindex_Miss = 0;
	unsigned int Mips_RGS_2DPass::m_variable__rt_hit_group_count = 0;

void Mips_RGS_2DPipeline::loadPipeline(ShaderPipelines& pipeline)
{
	if (!pipeline.isRayTracingSupported())
	{
		return;
	}

	ShaderSignatureDesc emptyShaderSignature({});
	ShaderSignatureDesc hitShaderSignature({ getMaterialShaderParameter(), getMeshShaderParameter(), getCameraShaderParameter() });
	pipeline.loadRayTracingMissProgram(getStaticClassId(), MissPipelineId, "Shaders/Mips_RGS_2D/Miss", emptyShaderSignature);
	pipeline.loadRayTracingRayGenProgram(getStaticClassId(), RayGenPipelineId, "Shaders/Mips_RGS_2D/RayGen", emptyShaderSignature);
}

void Mips_RGS_2DPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	if (!pipeline.isRayTracingSupported())
	{
		return;
	}
	pipeline.unloadPipeline(getStaticClassId(), MissPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), RayGenPipelineId);
}

void Mips_RGS_2DPass::createGlobalResourcesHandles()
{
}

void Mips_RGS_2DPass::deleteGlobalResourcesHandles()
{
}

void Mips_RGS_2DPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void Mips_RGS_2DPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<Mips_RGS_2DPassData>();

}

void Mips_RGS_2DPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void Mips_RGS_2DPass::addMips_RGS_2DPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.Mips_RGS_2D;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<Mips_RGS_2DPassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		// Texture: MipTex
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2d;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			desc.format = RenderFormat::R8G8B8A8_SRGB;
			desc.width = ((1 + 0)32) / 1 + 0;
			desc.height = ((1 + 0)32) / 1 + 0;
			passData.textureMipTex = build.create("Texture MipTex", desc);
		}

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("Mips_RGS_2D InitPass", InitPass);
	m_variable__missindex_Miss = pipelines.getRtMissShaderIndex(Mips_RGS_2DPipeline::getStaticClassId(), Mips_RGS_2DPipeline::MissPipelineId);
	m_variable__rt_hit_group_count = pipelines.getRtHitShaderRecordStride();

	// Ray Shader: RayGen
	auto& RayGenPass = [&](RenderGraphBuild& build)
	{
		auto rtShaderTable = scope.get<RenderGraphRtScene>().rtShaderTable;
		const uint32 rayGenShaderIndex	   = pipelines.getRtRayGenShaderIndex(Mips_RGS_2DPipeline::getStaticClassId(), Mips_RGS_2DPipeline::RayGenPipelineId);
		auto textureOutput = passData.textureMipTex = build.write(passData.textureMipTex, RenderBindFlags::UnorderedAccess);
		auto bufferScene = passData.bufferVertexBuffer;

		glm::uvec2 rtDim;
		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureMipTex, &sizeSrcDesc);
		rtDim.x = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		rtDim.y = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;

		auto rtPipelineState = pipelines.rtPipelineState();
		const RayTracingHelpers::GlobalShaderArgumentData rtGlobalShaderArgumentData = RayTracingHelpers::setupGlobalShaderArgumentPass(scope, build, pipelines);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			ShaderArgument raytracingArgument = RayTracingHelpers::createGlobalShaderArgument(registry, rtGlobalShaderArgumentData);
			RenderBindingShaderResourceView srvs[] = {
				build::rayTracingAcceleration(bufferScene),
				};
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dRwView(registry, textureOutput),
				};
			commandList.rayTrace(
				rtPipelineState,
				{ ShaderArgument(dynamicConstants.buffer, registry.createShaderViews(srvs)),
					ShaderArgument(RenderResourceHandle(), registry.createShaderViews(uavs)),
					raytracingArgument },
				rtShaderTable,
				rtDim.x, rtDim.y,
				rayGenShaderIndex);
		};
	};

	renderGraph.addPassCallback("Mips_RGS_2D RayGen", RayGenPass);
}

} // namespace halcyon
