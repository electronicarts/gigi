//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "Texture2DArrayRW_RGSPass.h"
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

	unsigned int Texture2DArrayRW_RGSPass::m_variable__chindex_RWCHS = 0;
	unsigned int Texture2DArrayRW_RGSPass::m_variable__missindex_RWMISS = 0;
	unsigned int Texture2DArrayRW_RGSPass::m_variable__rt_hit_group_count = 0;

void Texture2DArrayRW_RGSPipeline::loadPipeline(ShaderPipelines& pipeline)
{
	if (!pipeline.isRayTracingSupported())
	{
		return;
	}

	ShaderSignatureDesc emptyShaderSignature({});
	ShaderSignatureDesc hitShaderSignature({ getMaterialShaderParameter(), getMeshShaderParameter(), getCameraShaderParameter() });
	pipeline.loadRayTracingHitProgram(getStaticClassId(), RWCHSPipelineId, "", "Shaders/Texture2DArrayRW_RGS/RWCHS", hitShaderSignature);
	pipeline.loadRayTracingMissProgram(getStaticClassId(), RWMISSPipelineId, "Shaders/Texture2DArrayRW_RGS/RWMISS", emptyShaderSignature);
	pipeline.loadRayTracingRayGenProgram(getStaticClassId(), RWPipelineId, "Shaders/Texture2DArrayRW_RGS/RW", emptyShaderSignature);
}

void Texture2DArrayRW_RGSPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	if (!pipeline.isRayTracingSupported())
	{
		return;
	}
	pipeline.unloadPipeline(getStaticClassId(), RWCHSPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), RWMISSPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), RWPipelineId);
}

void Texture2DArrayRW_RGSPass::createGlobalResourcesHandles()
{
	m_texture_loadedTexture_0 = RenderSystem::createTexture("Texture2DArrayRW_RGS _loadedTexture_0 Texture");
}

void Texture2DArrayRW_RGSPass::deleteGlobalResourcesHandles()
{
	RenderSystem::destroyResource(m_texture_loadedTexture_0);
}

void Texture2DArrayRW_RGSPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{

	// Texture: _loadedTexture_0
	{
		RenderGraphTextureDesc& desc = m_texture_loadedTexture_0Desc;
		desc.type = RenderTextureType::Tex2dArray;
		desc.bindFlags = RenderBindFlags::ShaderResource;
		desc.format = RenderFormat::R8G8B8A8_UNORM;
		desc.width = ((1 + 0)1) / 1 + 0;
		desc.height = ((1 + 0)1) / 1 + 0;
		for (auto& d : deviceGroup.allDevices())
		{
			HcyVerify(d->createTexture(m_texture_loadedTexture_0, m_texture_loadedTexture_0Desc, nullptr, "Texture2DArrayRW_RGS _loadedTexture_0 Texture"));
		}
	}
}

void Texture2DArrayRW_RGSPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<Texture2DArrayRW_RGSPassData>();

	d.texture_loadedTexture_0 = renderGraph.import("Texture2DArrayRW_RGS _loadedTexture_0 Texture", m_texture_loadedTexture_0, m_texture_loadedTexture_0Desc);
}

void Texture2DArrayRW_RGSPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
		d->destroyResource(m_texture_loadedTexture_0);
	}
}

void Texture2DArrayRW_RGSPass::addTexture2DArrayRW_RGSPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.Texture2DArrayRW_RGS;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<Texture2DArrayRW_RGSPassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		// Texture: NodeTexture
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2dArray;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			desc.format = RenderFormat::R8G8B8A8_UNORM;
			desc.width = ((1 + 0)64) / 1 + 0;
			desc.height = ((1 + 0)64) / 1 + 0;
			passData.textureNodeTexture = build.create("Texture NodeTexture", desc);
		}

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("Texture2DArrayRW_RGS InitPass", InitPass);
	m_variable__chindex_RWCHS = pipelines.getRtHitShaderIndex(Texture2DArrayRW_RGSPipeline::getStaticClassId(), Texture2DArrayRW_RGSPipeline::RWCHSPipelineId);
	m_variable__missindex_RWMISS = pipelines.getRtMissShaderIndex(Texture2DArrayRW_RGSPipeline::getStaticClassId(), Texture2DArrayRW_RGSPipeline::RWMISSPipelineId);
	m_variable__rt_hit_group_count = pipelines.getRtHitShaderRecordStride();

	// Ray Shader: RW
	auto& RWPass = [&](RenderGraphBuild& build)
	{
		auto rtShaderTable = scope.get<RenderGraphRtScene>().rtShaderTable;
		const uint32 rayGenShaderIndex	   = pipelines.getRtRayGenShaderIndex(Texture2DArrayRW_RGSPipeline::getStaticClassId(), Texture2DArrayRW_RGSPipeline::RWPipelineId);
		auto texturenodeTexture = passData.textureNodeTexture = build.write(passData.textureNodeTexture, RenderBindFlags::UnorderedAccess);
		auto textureimportedTexture = passData.textureImportedTexture = build.write(passData.textureImportedTexture, RenderBindFlags::UnorderedAccess);
		auto textureimportedColor = passData.textureImportedColor = build.read(passData.textureImportedColor, RenderBindFlags::ShaderResource);
		auto texture_loadedTexture_0 = passData.texture_loadedTexture_0 = build.read(passData.texture_loadedTexture_0, RenderBindFlags::ShaderResource);

		glm::uvec2 rtDim;
		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureNodeTexture, &sizeSrcDesc);
		rtDim.x = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		rtDim.y = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;

		auto rtPipelineState = pipelines.rtPipelineState();
		const RayTracingHelpers::GlobalShaderArgumentData rtGlobalShaderArgumentData = RayTracingHelpers::setupGlobalShaderArgumentPass(scope, build, pipelines);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			ShaderArgument raytracingArgument = RayTracingHelpers::createGlobalShaderArgument(registry, rtGlobalShaderArgumentData);
			RenderBindingShaderResourceView srvs[] = {
				defaultTexture2dArrayView(registry, textureimportedColor),
				defaultTexture2dArrayView(registry, texture_loadedTexture_0),
				};
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dArrayRwView(registry, texturenodeTexture),
				defaultTexture2dArrayRwView(registry, textureimportedTexture),
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

	renderGraph.addPassCallback("Texture2DArrayRW_RGS RW", RWPass);
}

} // namespace halcyon
