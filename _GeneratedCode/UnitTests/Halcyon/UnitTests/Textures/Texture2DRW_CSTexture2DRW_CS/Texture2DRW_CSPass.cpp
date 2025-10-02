//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "Texture2DRW_CSPass.h"
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

void Texture2DRW_CSPipeline::loadPipeline(ShaderPipelines& pipeline)
{

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			RWPipelineId,
			"Shaders/Texture2DRW_CS/RW",
			ShaderSignatureDesc(
				{ ShaderParameter(RWSrvCount, RWUavCount) },
				{ }));
	}
}

void Texture2DRW_CSPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	pipeline.unloadPipeline(getStaticClassId(), RWPipelineId);
}

void Texture2DRW_CSPass::createGlobalResourcesHandles()
{
	m_texture_loadedTexture_0 = RenderSystem::createTexture("Texture2DRW_CS _loadedTexture_0 Texture");
}

void Texture2DRW_CSPass::deleteGlobalResourcesHandles()
{
	RenderSystem::destroyResource(m_texture_loadedTexture_0);
}

void Texture2DRW_CSPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{

	// Texture: _loadedTexture_0
	{
		RenderGraphTextureDesc& desc = m_texture_loadedTexture_0Desc;
		desc.type = RenderTextureType::Tex2d;
		desc.bindFlags = RenderBindFlags::ShaderResource;
		desc.format = RenderFormat::R8G8B8A8_UNORM;
		desc.width = ((1 + 0)1) / 1 + 0;
		desc.height = ((1 + 0)1) / 1 + 0;
		for (auto& d : deviceGroup.allDevices())
		{
			HcyVerify(d->createTexture(m_texture_loadedTexture_0, m_texture_loadedTexture_0Desc, nullptr, "Texture2DRW_CS _loadedTexture_0 Texture"));
		}
	}
}

void Texture2DRW_CSPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<Texture2DRW_CSPassData>();

	d.texture_loadedTexture_0 = renderGraph.import("Texture2DRW_CS _loadedTexture_0 Texture", m_texture_loadedTexture_0, m_texture_loadedTexture_0Desc);
}

void Texture2DRW_CSPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
		d->destroyResource(m_texture_loadedTexture_0);
	}
}

void Texture2DRW_CSPass::addTexture2DRW_CSPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.Texture2DRW_CS;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<Texture2DRW_CSPassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		// Texture: NodeTexture
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2d;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			desc.format = RenderFormat::R8G8B8A8_UNORM;
			desc.width = ((1 + 0)512) / 1 + 0;
			desc.height = ((1 + 0)512) / 1 + 0;
			passData.textureNodeTexture = build.create("Texture NodeTexture", desc);
		}

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("Texture2DRW_CS InitPass", InitPass);

	// Compute Shader: RW
	auto& RWPass = [&](RenderGraphBuild& build)
	{
		auto texturenodeTexture = passData.textureNodeTexture = build.write(passData.textureNodeTexture, RenderBindFlags::UnorderedAccess);
		auto textureimportedTexture = passData.textureImportedTexture = build.write(passData.textureImportedTexture, RenderBindFlags::UnorderedAccess);
		auto textureimportedColor = passData.textureImportedColor = build.read(passData.textureImportedColor, RenderBindFlags::ShaderResource);
		auto texture_loadedTexture_0 = passData.texture_loadedTexture_0 = build.read(passData.texture_loadedTexture_0, RenderBindFlags::ShaderResource);

		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureNodeTexture, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(Texture2DRW_CSPipeline::getStaticClassId(), Texture2DRW_CSPipeline::RWPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderBindingShaderResourceView srvs[] = {
				defaultTexture2dView(registry, textureimportedColor),
				defaultTexture2dView(registry, texture_loadedTexture_0),
				};
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dRwView(registry, texturenodeTexture),
				defaultTexture2dRwView(registry, textureimportedTexture),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, registry.createShaderViews(srvs, uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("Texture2DRW_CS RW", RWPass);
}

} // namespace halcyon
