//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "GPUWritePass.h"
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

void GPUWritePipeline::loadPipeline(ShaderPipelines& pipeline)
{

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			ComputeShaderPipelineId,
			"Shaders/GPUWrite/ComputeShader",
			ShaderSignatureDesc(
				{ ShaderParameter(ComputeShaderSrvCount, ComputeShaderUavCount) },
				{ }));
	}
}

void GPUWritePipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	pipeline.unloadPipeline(getStaticClassId(), ComputeShaderPipelineId);
}

void GPUWritePass::createGlobalResourcesHandles()
{
	m_textureTexture2D = RenderSystem::createTexture("GPUWrite Texture2D Texture");
	m_textureTexture2DArray = RenderSystem::createTexture("GPUWrite Texture2DArray Texture");
	m_textureTexture3D = RenderSystem::createTexture("GPUWrite Texture3D Texture");
	m_bufferFloatBuffer = RenderSystem::createBuffer("GPUWrite FloatBuffer Buffer");
	m_bufferStructBuffer = RenderSystem::createBuffer("GPUWrite StructBuffer Buffer");
}

void GPUWritePass::deleteGlobalResourcesHandles()
{
	RenderSystem::destroyResource(m_textureTexture2D);
	RenderSystem::destroyResource(m_textureTexture2DArray);
	RenderSystem::destroyResource(m_textureTexture3D);
	RenderSystem::destroyResource(m_bufferFloatBuffer);
	RenderSystem::destroyResource(m_bufferStructBuffer);
}

void GPUWritePass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{

	// Texture: Texture2D
	{
		RenderGraphTextureDesc& desc = m_textureTexture2DDesc;
		desc.type = RenderTextureType::Tex2d;
		desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
		desc.format = RenderFormat::R8G8B8A8_UNORM;
		desc.width = ((1 + 0)32) / 1 + 0;
		desc.height = ((1 + 0)32) / 1 + 0;
		for (auto& d : deviceGroup.allDevices())
		{
			HcyVerify(d->createTexture(m_textureTexture2D, m_textureTexture2DDesc, nullptr, "GPUWrite Texture2D Texture"));
		}
	}

	// Texture: Texture2DArray
	{
		RenderGraphTextureDesc& desc = m_textureTexture2DArrayDesc;
		desc.type = RenderTextureType::Tex2dArray;
		desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
		desc.format = m_textureTexture2DDesc.format;
		desc.width = ((1 + 0)32) / 1 + 0;
		desc.height = ((1 + 0)32) / 1 + 0;
		for (auto& d : deviceGroup.allDevices())
		{
			HcyVerify(d->createTexture(m_textureTexture2DArray, m_textureTexture2DArrayDesc, nullptr, "GPUWrite Texture2DArray Texture"));
		}
	}

	// Texture: Texture3D
	{
		RenderGraphTextureDesc& desc = m_textureTexture3DDesc;
		desc.type = RenderTextureType::Tex3d;
		desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
		desc.format = m_textureTexture2DDesc.format;
		desc.width = ((1 + 0)32) / 1 + 0;
		desc.height = ((1 + 0)32) / 1 + 0;
		for (auto& d : deviceGroup.allDevices())
		{
			HcyVerify(d->createTexture(m_textureTexture3D, m_textureTexture3DDesc, nullptr, "GPUWrite Texture3D Texture"));
		}
	}

	// Buffer: FloatBuffer
	{
		RenderGraphBufferDesc& desc = m_bufferFloatBufferDesc;
		desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
		desc.size = desc.stride * (((1 + 0) * 8) / 1 + 0);
		for (auto& d : deviceGroup.allDevices())
		{
			HcyVerify(d->createBuffer(m_bufferFloatBuffer, m_bufferFloatBufferDesc, nullptr, "GPUWrite FloatBuffer Buffer"));
		}
	}

	// Buffer: StructBuffer
	{
		RenderGraphBufferDesc& desc = m_bufferStructBufferDesc;
		desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
		desc.stride = 8;
		desc.size = desc.stride * (((1 + 0) * 1) / 1 + 0);
		for (auto& d : deviceGroup.allDevices())
		{
			HcyVerify(d->createBuffer(m_bufferStructBuffer, m_bufferStructBufferDesc, nullptr, "GPUWrite StructBuffer Buffer"));
		}
	}
}

void GPUWritePass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<GPUWritePassData>();

	d.textureTexture2D = renderGraph.import("GPUWrite Texture2D Texture", m_textureTexture2D, m_textureTexture2DDesc);
	d.textureTexture2DArray = renderGraph.import("GPUWrite Texture2DArray Texture", m_textureTexture2DArray, m_textureTexture2DArrayDesc);
	d.textureTexture3D = renderGraph.import("GPUWrite Texture3D Texture", m_textureTexture3D, m_textureTexture3DDesc);
	d.bufferFloatBuffer = renderGraph.import("GPUWrite FloatBuffer Buffer", m_bufferFloatBuffer, m_bufferFloatBufferDesc);
	d.bufferStructBuffer = renderGraph.import("GPUWrite StructBuffer Buffer", m_bufferStructBuffer, m_bufferStructBufferDesc);
}

void GPUWritePass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
		d->destroyResource(m_textureTexture2D);
		d->destroyResource(m_textureTexture2DArray);
		d->destroyResource(m_textureTexture3D);
		d->destroyResource(m_bufferFloatBuffer);
		d->destroyResource(m_bufferStructBuffer);
	}
}

void GPUWritePass::addGPUWritePass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.GPUWrite;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<GPUWritePassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("GPUWrite InitPass", InitPass);

	// Compute Shader: ComputeShader
	auto& ComputeShaderPass = [&](RenderGraphBuild& build)
	{
		auto texturetex2D = passData.textureTexture2D = build.write(passData.textureTexture2D, RenderBindFlags::UnorderedAccess);
		auto texturetex2DArray = passData.textureTexture2DArray = build.write(passData.textureTexture2DArray, RenderBindFlags::UnorderedAccess);
		auto texturetex3D = passData.textureTexture3D = build.write(passData.textureTexture3D, RenderBindFlags::UnorderedAccess);
		auto bufferfloatBuffer = passData.bufferFloatBuffer = build.write(passData.bufferFloatBuffer, RenderBindFlags::UnorderedAccess);
		auto bufferstructBuffer = passData.bufferStructBuffer = build.write(passData.bufferStructBuffer, RenderBindFlags::UnorderedAccess);

		uint32 dispatchWidth = ((1 + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((1 + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((1 + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(GPUWritePipeline::getStaticClassId(), GPUWritePipeline::ComputeShaderPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dRwView(registry, texturetex2D),
				defaultTexture2dArrayRwView(registry, texturetex2DArray),
				defaultTexture3dRwView(registry, texturetex3D),
				defaultBufferRwView(registry, bufferfloatBuffer),
				defaultBufferRwView(registry, bufferstructBuffer),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, registry.createShaderViews(uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("GPUWrite ComputeShader", ComputeShaderPass);
}

} // namespace halcyon
