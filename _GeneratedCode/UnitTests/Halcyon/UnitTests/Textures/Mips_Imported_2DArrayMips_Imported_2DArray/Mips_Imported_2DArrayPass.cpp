//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "Mips_Imported_2DArrayPass.h"
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

void Mips_Imported_2DArrayPipeline::loadPipeline(ShaderPipelines& pipeline)
{
}

void Mips_Imported_2DArrayPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
}

void Mips_Imported_2DArrayPass::createGlobalResourcesHandles()
{
}

void Mips_Imported_2DArrayPass::deleteGlobalResourcesHandles()
{
}

void Mips_Imported_2DArrayPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void Mips_Imported_2DArrayPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<Mips_Imported_2DArrayPassData>();

}

void Mips_Imported_2DArrayPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void Mips_Imported_2DArrayPass::addMips_Imported_2DArrayPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.Mips_Imported_2DArray;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<Mips_Imported_2DArrayPassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("Mips_Imported_2DArray InitPass", InitPass);
}

} // namespace halcyon
