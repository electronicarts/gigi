//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "Texture3DRW_PSPass.h"
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

void Texture3DRW_PSPipeline::loadPipeline(ShaderPipelines& pipeline)
{
}

void Texture3DRW_PSPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
}

void Texture3DRW_PSPass::createGlobalResourcesHandles()
{
	m_texture_loadedTexture_0 = RenderSystem::createTexture("Texture3DRW_PS _loadedTexture_0 Texture");
}

void Texture3DRW_PSPass::deleteGlobalResourcesHandles()
{
	RenderSystem::destroyResource(m_texture_loadedTexture_0);
}

void Texture3DRW_PSPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{

	// Texture: _loadedTexture_0
	{
		RenderGraphTextureDesc& desc = m_texture_loadedTexture_0Desc;
		desc.type = RenderTextureType::Tex3d;
		desc.bindFlags = RenderBindFlags::ShaderResource;
		desc.format = RenderFormat::R8G8B8A8_UNORM;
		desc.width = ((1 + 0)1) / 1 + 0;
		desc.height = ((1 + 0)1) / 1 + 0;
		for (auto& d : deviceGroup.allDevices())
		{
			HcyVerify(d->createTexture(m_texture_loadedTexture_0, m_texture_loadedTexture_0Desc, nullptr, "Texture3DRW_PS _loadedTexture_0 Texture"));
		}
	}
}

void Texture3DRW_PSPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<Texture3DRW_PSPassData>();

	d.texture_loadedTexture_0 = renderGraph.import("Texture3DRW_PS _loadedTexture_0 Texture", m_texture_loadedTexture_0, m_texture_loadedTexture_0Desc);
}

void Texture3DRW_PSPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
		d->destroyResource(m_texture_loadedTexture_0);
	}
}

void Texture3DRW_PSPass::addTexture3DRW_PSPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.Texture3DRW_PS;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<Texture3DRW_PSPassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		// Texture: NodeTexture
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex3d;
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
	renderGraph.addPassCallback("Texture3DRW_PS InitPass", InitPass);
}

} // namespace halcyon
