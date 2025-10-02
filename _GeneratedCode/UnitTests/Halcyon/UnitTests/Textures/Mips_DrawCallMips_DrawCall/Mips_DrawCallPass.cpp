//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "Mips_DrawCallPass.h"
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

void Mips_DrawCallPipeline::loadPipeline(ShaderPipelines& pipeline)
{
}

void Mips_DrawCallPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
}

void Mips_DrawCallPass::createGlobalResourcesHandles()
{
}

void Mips_DrawCallPass::deleteGlobalResourcesHandles()
{
}

void Mips_DrawCallPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void Mips_DrawCallPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<Mips_DrawCallPassData>();

}

void Mips_DrawCallPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void Mips_DrawCallPass::addMips_DrawCallPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.Mips_DrawCall;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<Mips_DrawCallPassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		// Texture: Color_Buffer
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2dArray;
			desc.bindFlags = RenderBindFlags::ShaderResource;
			desc.format = RenderFormat::R8G8B8A8_SRGB;
			desc.width = ((1 + 0)400) / 1 + 0;
			desc.height = ((1 + 0)400) / 1 + 0;
			passData.textureColor_Buffer = build.create("Texture Color_Buffer", desc);
		}

		// Texture: Depth_Buffer
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2dArray;
			desc.bindFlags = RenderBindFlags::ShaderResource;
			desc.format = RenderFormat::D32_FLOAT;
			RenderGraphTextureDesc sizeSrcDesc;
			build.getTextureDesc(passData.textureColor_Buffer, &sizeSrcDesc);
			desc.width = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
			desc.height = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
			passData.textureDepth_Buffer = build.create("Texture Depth_Buffer", desc);
		}

		// Texture: Final_Color_Buffer
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2d;
			desc.bindFlags = RenderBindFlags::ShaderResource;
			desc.format = RenderFormat::R8G8B8A8_SRGB;
			desc.width = (unsigned int)settings.RenderSize[0];
			desc.height = (unsigned int)settings.RenderSize[1];
			desc.depth = 1;
			passData.textureFinal_Color_Buffer = build.create("Texture Final_Color_Buffer", desc);
		}

		// Texture: Final_Depth_Buffer
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2d;
			desc.bindFlags = RenderBindFlags::ShaderResource;
			desc.format = RenderFormat::D32_FLOAT;
			desc.width = (unsigned int)settings.RenderSize[0];
			desc.height = (unsigned int)settings.RenderSize[1];
			desc.depth = 1;
			passData.textureFinal_Depth_Buffer = build.create("Texture Final_Depth_Buffer", desc);
		}

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("Mips_DrawCall InitPass", InitPass);

	// Shader Constants: _VertexShaderCB
	{
		m_cb__VertexShaderCB.ViewProjMtx = settings.ViewProjMtx;
	}

	// Shader Constants: _VertexShaderSphereCB
	{
		m_cb__VertexShaderSphereCB.ViewProjMtx = settings.ViewProjMtx;
	}

	// Shader Constants: _PixelShaderSphereCB
	{
		m_cb__PixelShaderSphereCB.UseMips = settings.UseMips;
	}
}

} // namespace halcyon
