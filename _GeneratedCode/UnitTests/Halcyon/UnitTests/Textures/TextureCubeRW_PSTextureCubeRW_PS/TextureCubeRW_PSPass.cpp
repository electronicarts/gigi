//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "TextureCubeRW_PSPass.h"
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

void TextureCubeRW_PSPipeline::loadPipeline(ShaderPipelines& pipeline)
{
}

void TextureCubeRW_PSPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
}

void TextureCubeRW_PSPass::createGlobalResourcesHandles()
{
	m_texture_loadedTexture_0 = RenderSystem::createTexture("TextureCubeRW_PS _loadedTexture_0 Texture");
}

void TextureCubeRW_PSPass::deleteGlobalResourcesHandles()
{
	RenderSystem::destroyResource(m_texture_loadedTexture_0);
}

void TextureCubeRW_PSPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{

	// Texture: _loadedTexture_0
	{
		RenderGraphTextureDesc& desc = m_texture_loadedTexture_0Desc;
		desc.type = RenderTextureType::TexCube;
		desc.bindFlags = RenderBindFlags::ShaderResource;
		desc.format = RenderFormat::R8G8B8A8_UNORM;
		desc.width = ((1 + 0)1) / 1 + 0;
		desc.height = ((1 + 0)1) / 1 + 0;
		for (auto& d : deviceGroup.allDevices())
		{
			HcyVerify(d->createTexture(m_texture_loadedTexture_0, m_texture_loadedTexture_0Desc, nullptr, "TextureCubeRW_PS _loadedTexture_0 Texture"));
		}
	}
}

void TextureCubeRW_PSPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<TextureCubeRW_PSPassData>();

	d.texture_loadedTexture_0 = renderGraph.import("TextureCubeRW_PS _loadedTexture_0 Texture", m_texture_loadedTexture_0, m_texture_loadedTexture_0Desc);
}

void TextureCubeRW_PSPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
		d->destroyResource(m_texture_loadedTexture_0);
	}
}

void TextureCubeRW_PSPass::addTextureCubeRW_PSPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.TextureCubeRW_PS;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<TextureCubeRW_PSPassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		// Texture: NodeTexture
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::TexCube;
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
	renderGraph.addPassCallback("TextureCubeRW_PS InitPass", InitPass);
}

} // namespace halcyon
