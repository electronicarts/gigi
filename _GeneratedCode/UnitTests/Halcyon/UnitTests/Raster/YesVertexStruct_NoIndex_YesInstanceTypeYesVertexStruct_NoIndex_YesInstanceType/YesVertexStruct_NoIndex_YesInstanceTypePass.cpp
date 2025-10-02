//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "YesVertexStruct_NoIndex_YesInstanceTypePass.h"
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

void YesVertexStruct_NoIndex_YesInstanceTypePipeline::loadPipeline(ShaderPipelines& pipeline)
{
}

void YesVertexStruct_NoIndex_YesInstanceTypePipeline::unloadPipeline(ShaderPipelines& pipeline)
{
}

void YesVertexStruct_NoIndex_YesInstanceTypePass::createGlobalResourcesHandles()
{
}

void YesVertexStruct_NoIndex_YesInstanceTypePass::deleteGlobalResourcesHandles()
{
}

void YesVertexStruct_NoIndex_YesInstanceTypePass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void YesVertexStruct_NoIndex_YesInstanceTypePass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<YesVertexStruct_NoIndex_YesInstanceTypePassData>();

}

void YesVertexStruct_NoIndex_YesInstanceTypePass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void YesVertexStruct_NoIndex_YesInstanceTypePass::addYesVertexStruct_NoIndex_YesInstanceTypePass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.YesVertexStruct_NoIndex_YesInstanceType;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<YesVertexStruct_NoIndex_YesInstanceTypePassData>();

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
	renderGraph.addPassCallback("YesVertexStruct_NoIndex_YesInstanceType InitPass", InitPass);

	// Shader Constants: _VertexShaderCB
	{
		m_cb__VertexShaderCB.ViewProjMtx = settings.ViewProjMtx;
	}

	// Shader Constants: _PixelShaderCB
	{
		m_cb__PixelShaderCB.viewMode = (int)settings.viewMode;
	}
}

} // namespace halcyon
