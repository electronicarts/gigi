//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "StencilPass.h"
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

void StencilPipeline::loadPipeline(ShaderPipelines& pipeline)
{
}

void StencilPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
}

void StencilPass::createGlobalResourcesHandles()
{
	m_textureDepth_Stencil = RenderSystem::createTexture("Stencil Depth_Stencil Texture");
	m_texture_loadedTexture_0 = RenderSystem::createTexture("Stencil _loadedTexture_0 Texture");
}

void StencilPass::deleteGlobalResourcesHandles()
{
	RenderSystem::destroyResource(m_textureDepth_Stencil);
	RenderSystem::destroyResource(m_texture_loadedTexture_0);
}

void StencilPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{

	// Texture: Depth_Stencil
	{
		RenderGraphTextureDesc& desc = m_textureDepth_StencilDesc;
		desc.type = RenderTextureType::Tex2d;
		desc.bindFlags = RenderBindFlags::ShaderResource;
		desc.format = RenderFormat::D24_Unorm_S8;
		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureColor, &sizeSrcDesc);
		desc.width = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		desc.height = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		for (auto& d : deviceGroup.allDevices())
		{
			HcyVerify(d->createTexture(m_textureDepth_Stencil, m_textureDepth_StencilDesc, nullptr, "Stencil Depth_Stencil Texture"));
		}
	}

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
			HcyVerify(d->createTexture(m_texture_loadedTexture_0, m_texture_loadedTexture_0Desc, nullptr, "Stencil _loadedTexture_0 Texture"));
		}
	}
}

void StencilPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<StencilPassData>();

	d.textureDepth_Stencil = renderGraph.import("Stencil Depth_Stencil Texture", m_textureDepth_Stencil, m_textureDepth_StencilDesc);
	d.texture_loadedTexture_0 = renderGraph.import("Stencil _loadedTexture_0 Texture", m_texture_loadedTexture_0, m_texture_loadedTexture_0Desc);
}

void StencilPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
		d->destroyResource(m_textureDepth_Stencil);
		d->destroyResource(m_texture_loadedTexture_0);
	}
}

void StencilPass::addStencilPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.Stencil;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<StencilPassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		// Texture: Color
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2d;
			desc.bindFlags = RenderBindFlags::ShaderResource;
			desc.format = RenderFormat::R8G8B8A8_SRGB;
			desc.width = ((1 + 0)128) / 1 + 0;
			desc.height = ((1 + 0)128) / 1 + 0;
			passData.textureColor = build.create("Texture Color", desc);
		}

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("Stencil InitPass", InitPass);

	// Shader Constants: _Draw1VSCB
	{
		m_cb__Draw1VSCB.ViewProjMtx = settings.ViewProjMtx;
	}
}

} // namespace halcyon
