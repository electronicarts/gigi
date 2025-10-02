//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "Texture2DArrayRW_CSPass.h"
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

void Texture2DArrayRW_CSPipeline::loadPipeline(ShaderPipelines& pipeline)
{

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			RWPipelineId,
			"Shaders/Texture2DArrayRW_CS/RW",
			ShaderSignatureDesc(
				{ ShaderParameter(RWSrvCount, RWUavCount) },
				{ }));
	}
}

void Texture2DArrayRW_CSPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	pipeline.unloadPipeline(getStaticClassId(), RWPipelineId);
}

void Texture2DArrayRW_CSPass::createGlobalResourcesHandles()
{
	m_texture_loadedTexture_0 = RenderSystem::createTexture("Texture2DArrayRW_CS _loadedTexture_0 Texture");
}

void Texture2DArrayRW_CSPass::deleteGlobalResourcesHandles()
{
	RenderSystem::destroyResource(m_texture_loadedTexture_0);
}

void Texture2DArrayRW_CSPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
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
			HcyVerify(d->createTexture(m_texture_loadedTexture_0, m_texture_loadedTexture_0Desc, nullptr, "Texture2DArrayRW_CS _loadedTexture_0 Texture"));
		}
	}
}

void Texture2DArrayRW_CSPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<Texture2DArrayRW_CSPassData>();

	d.texture_loadedTexture_0 = renderGraph.import("Texture2DArrayRW_CS _loadedTexture_0 Texture", m_texture_loadedTexture_0, m_texture_loadedTexture_0Desc);
}

void Texture2DArrayRW_CSPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
		d->destroyResource(m_texture_loadedTexture_0);
	}
}

void Texture2DArrayRW_CSPass::addTexture2DArrayRW_CSPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.Texture2DArrayRW_CS;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<Texture2DArrayRW_CSPassData>();

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
	renderGraph.addPassCallback("Texture2DArrayRW_CS InitPass", InitPass);

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
		dispatchWidth = (dispatchWidth + 4 - 1) / 4;
		dispatchHeight = (dispatchHeight + 4 - 1) / 4;
		dispatchDepth = (dispatchDepth + 4 - 1) / 4;

		auto pipelineState = pipelines.pipelineState(Texture2DArrayRW_CSPipeline::getStaticClassId(), Texture2DArrayRW_CSPipeline::RWPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderBindingShaderResourceView srvs[] = {
				defaultTexture2dArrayView(registry, textureimportedColor),
				defaultTexture2dArrayView(registry, texture_loadedTexture_0),
				};
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dArrayRwView(registry, texturenodeTexture),
				defaultTexture2dArrayRwView(registry, textureimportedTexture),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, registry.createShaderViews(srvs, uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("Texture2DArrayRW_CS RW", RWPass);
}

} // namespace halcyon
