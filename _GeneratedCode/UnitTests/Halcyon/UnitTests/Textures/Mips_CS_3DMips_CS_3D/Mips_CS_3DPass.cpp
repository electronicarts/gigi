//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "Mips_CS_3DPass.h"
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

void Mips_CS_3DPipeline::loadPipeline(ShaderPipelines& pipeline)
{

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			Mip0PipelineId,
			"Shaders/Mips_CS_3D/Mip0",
			ShaderSignatureDesc(
				{ ShaderParameter(Mip0SrvCount, Mip0UavCount) },
				{ }));
	}

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			Mip1PipelineId,
			"Shaders/Mips_CS_3D/Mip1",
			ShaderSignatureDesc(
				{ ShaderParameter(Mip1SrvCount, Mip1UavCount) },
				{ }));
	}

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			Mip2PipelineId,
			"Shaders/Mips_CS_3D/Mip2",
			ShaderSignatureDesc(
				{ ShaderParameter(Mip2SrvCount, Mip2UavCount) },
				{ }));
	}

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			Mip3PipelineId,
			"Shaders/Mips_CS_3D/Mip3",
			ShaderSignatureDesc(
				{ ShaderParameter(Mip3SrvCount, Mip3UavCount) },
				{ }));
	}
}

void Mips_CS_3DPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	pipeline.unloadPipeline(getStaticClassId(), Mip0PipelineId);
	pipeline.unloadPipeline(getStaticClassId(), Mip1PipelineId);
	pipeline.unloadPipeline(getStaticClassId(), Mip2PipelineId);
	pipeline.unloadPipeline(getStaticClassId(), Mip3PipelineId);
}

void Mips_CS_3DPass::createGlobalResourcesHandles()
{
	m_texture_loadedTexture_0 = RenderSystem::createTexture("Mips_CS_3D _loadedTexture_0 Texture");
}

void Mips_CS_3DPass::deleteGlobalResourcesHandles()
{
	RenderSystem::destroyResource(m_texture_loadedTexture_0);
}

void Mips_CS_3DPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{

	// Texture: _loadedTexture_0
	{
		RenderGraphTextureDesc& desc = m_texture_loadedTexture_0Desc;
		desc.type = RenderTextureType::Tex2d;
		desc.bindFlags = RenderBindFlags::ShaderResource;
		desc.format = RenderFormat::R8G8B8A8_SRGB;
		desc.width = ((1 + 0)1) / 1 + 0;
		desc.height = ((1 + 0)1) / 1 + 0;
		for (auto& d : deviceGroup.allDevices())
		{
			HcyVerify(d->createTexture(m_texture_loadedTexture_0, m_texture_loadedTexture_0Desc, nullptr, "Mips_CS_3D _loadedTexture_0 Texture"));
		}
	}
}

void Mips_CS_3DPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<Mips_CS_3DPassData>();

	d.texture_loadedTexture_0 = renderGraph.import("Mips_CS_3D _loadedTexture_0 Texture", m_texture_loadedTexture_0, m_texture_loadedTexture_0Desc);
}

void Mips_CS_3DPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
		d->destroyResource(m_texture_loadedTexture_0);
	}
}

void Mips_CS_3DPass::addMips_CS_3DPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.Mips_CS_3D;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<Mips_CS_3DPassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		// Texture: Output
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex3d;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			desc.format = RenderFormat::R8G8B8A8_SRGB;
			desc.width = (unsigned int)settings.RenderSize[0];
			desc.height = (unsigned int)settings.RenderSize[1];
			desc.depth = (unsigned int)settings.RenderSize[2];
			passData.textureOutput = build.create("Texture Output", desc);
		}

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("Mips_CS_3D InitPass", InitPass);

	// Compute Shader: Mip0
	auto& Mip0Pass = [&](RenderGraphBuild& build)
	{
		auto textureOutput = passData.textureOutput = build.write(passData.textureOutput, RenderBindFlags::UnorderedAccess);
		auto texture_loadedTexture_0 = passData.texture_loadedTexture_0 = build.read(passData.texture_loadedTexture_0, RenderBindFlags::ShaderResource);

		uint32 dispatchWidth = ((settings.RenderSize[0] + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((settings.RenderSize[1] + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((settings.RenderSize[2] + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 4 - 1) / 4;
		dispatchHeight = (dispatchHeight + 4 - 1) / 4;
		dispatchDepth = (dispatchDepth + 4 - 1) / 4;

		auto pipelineState = pipelines.pipelineState(Mips_CS_3DPipeline::getStaticClassId(), Mips_CS_3DPipeline::Mip0PipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderBindingShaderResourceView srvs[] = {
				defaultTexture2dView(registry, texture_loadedTexture_0),
				};
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture3dRwView(registry, textureOutput),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, registry.createShaderViews(srvs, uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("Mips_CS_3D Mip0", Mip0Pass);

	// Compute Shader: Mip1
	auto& Mip1Pass = [&](RenderGraphBuild& build)
	{
		auto textureInput = passData.textureOutput = build.write(passData.textureOutput, RenderBindFlags::UnorderedAccess);
		auto textureOutput = passData.textureOutput = build.write(passData.textureOutput, RenderBindFlags::UnorderedAccess);

		uint32 dispatchWidth = ((settings.RenderSize[0] + 0) * 1) / 2 + 0;
		uint32 dispatchHeight = ((settings.RenderSize[1] + 0) * 1) / 2 + 0;
		uint32 dispatchDepth = ((settings.RenderSize[2] + 0) * 1) / 2 + 0;
		dispatchWidth = (dispatchWidth + 4 - 1) / 4;
		dispatchHeight = (dispatchHeight + 4 - 1) / 4;
		dispatchDepth = (dispatchDepth + 4 - 1) / 4;

		auto pipelineState = pipelines.pipelineState(Mips_CS_3DPipeline::getStaticClassId(), Mips_CS_3DPipeline::Mip1PipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture3dRwView(registry, textureInput),
				defaultTexture3dRwView(registry, textureOutput),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, registry.createShaderViews(uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("Mips_CS_3D Mip1", Mip1Pass);

	// Compute Shader: Mip2
	auto& Mip2Pass = [&](RenderGraphBuild& build)
	{
		auto textureInput = passData.textureOutput = build.write(passData.textureOutput, RenderBindFlags::UnorderedAccess);
		auto textureOutput = passData.textureOutput = build.write(passData.textureOutput, RenderBindFlags::UnorderedAccess);

		uint32 dispatchWidth = ((settings.RenderSize[0] + 0) * 1) / 4 + 0;
		uint32 dispatchHeight = ((settings.RenderSize[1] + 0) * 1) / 4 + 0;
		uint32 dispatchDepth = ((settings.RenderSize[2] + 0) * 1) / 4 + 0;
		dispatchWidth = (dispatchWidth + 4 - 1) / 4;
		dispatchHeight = (dispatchHeight + 4 - 1) / 4;
		dispatchDepth = (dispatchDepth + 4 - 1) / 4;

		auto pipelineState = pipelines.pipelineState(Mips_CS_3DPipeline::getStaticClassId(), Mips_CS_3DPipeline::Mip2PipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture3dRwView(registry, textureInput),
				defaultTexture3dRwView(registry, textureOutput),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, registry.createShaderViews(uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("Mips_CS_3D Mip2", Mip2Pass);

	// Compute Shader: Mip3
	auto& Mip3Pass = [&](RenderGraphBuild& build)
	{
		auto textureInput = passData.textureOutput = build.write(passData.textureOutput, RenderBindFlags::UnorderedAccess);
		auto textureOutput = passData.textureOutput = build.write(passData.textureOutput, RenderBindFlags::UnorderedAccess);

		uint32 dispatchWidth = ((settings.RenderSize[0] + 0) * 1) / 8 + 0;
		uint32 dispatchHeight = ((settings.RenderSize[1] + 0) * 1) / 8 + 0;
		uint32 dispatchDepth = ((settings.RenderSize[2] + 0) * 1) / 8 + 0;
		dispatchWidth = (dispatchWidth + 4 - 1) / 4;
		dispatchHeight = (dispatchHeight + 4 - 1) / 4;
		dispatchDepth = (dispatchDepth + 4 - 1) / 4;

		auto pipelineState = pipelines.pipelineState(Mips_CS_3DPipeline::getStaticClassId(), Mips_CS_3DPipeline::Mip3PipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture3dRwView(registry, textureInput),
				defaultTexture3dRwView(registry, textureOutput),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, registry.createShaderViews(uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("Mips_CS_3D Mip3", Mip3Pass);
}

} // namespace halcyon
