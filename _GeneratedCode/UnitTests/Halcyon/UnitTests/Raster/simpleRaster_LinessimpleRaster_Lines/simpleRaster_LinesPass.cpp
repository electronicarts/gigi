//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "simpleRaster_LinesPass.h"
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

void simpleRaster_LinesPipeline::loadPipeline(ShaderPipelines& pipeline)
{
}

void simpleRaster_LinesPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
}

void simpleRaster_LinesPass::createGlobalResourcesHandles()
{
}

void simpleRaster_LinesPass::deleteGlobalResourcesHandles()
{
}

void simpleRaster_LinesPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void simpleRaster_LinesPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<simpleRaster_LinesPassData>();

}

void simpleRaster_LinesPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void simpleRaster_LinesPass::addsimpleRaster_LinesPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.simpleRaster_Lines;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<simpleRaster_LinesPassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		// Texture: Color_Buffer
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2d;
			desc.bindFlags = RenderBindFlags::ShaderResource;
			desc.format = RenderFormat::R8G8B8A8_SRGB;
			desc.width = ((1 + 0)512) / 1 + 0;
			desc.height = ((1 + 0)512) / 1 + 0;
			passData.textureColor_Buffer = build.create("Texture Color_Buffer", desc);
		}

		// Texture: Depth_Buffer
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2d;
			desc.bindFlags = RenderBindFlags::ShaderResource;
			desc.format = RenderFormat::D32_FLOAT;
			RenderGraphTextureDesc sizeSrcDesc;
			build.getTextureDesc(passData.textureColor_Buffer, &sizeSrcDesc);
			desc.width = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
			desc.height = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
			passData.textureDepth_Buffer = build.create("Texture Depth_Buffer", desc);
		}

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("simpleRaster_Lines InitPass", InitPass);

	// Shader Constants: _VertexShaderCB
	{
		m_cb__VertexShaderCB.ViewProjMtx = settings.ViewProjMtx;
	}
}

} // namespace halcyon
