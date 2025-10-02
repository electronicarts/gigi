//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "buffertestPass.h"
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

void buffertestPipeline::loadPipeline(ShaderPipelines& pipeline)
{

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			BufferTestPipelineId,
			"Shaders/buffertest/BufferTest",
			ShaderSignatureDesc(
				{ ShaderParameter(BufferTestSrvCount, BufferTestUavCount) },
				{ }));
	}
}

void buffertestPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	pipeline.unloadPipeline(getStaticClassId(), BufferTestPipelineId);
}

void buffertestPass::createGlobalResourcesHandles()
{
}

void buffertestPass::deleteGlobalResourcesHandles()
{
}

void buffertestPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void buffertestPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<buffertestPassData>();

}

void buffertestPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void buffertestPass::addbuffertestPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.buffertest;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<buffertestPassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		// Buffer: OutputTypedBuffer
		// An internal buffer used during the filtering process.
		{
			RenderGraphBufferDesc desc;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			RenderGraphBufferDesc formatSrcDesc;
			build.getBufferDesc(passData.bufferInputTypedBuffer, &formatSrcDesc);
			desc.stride = formatSrcDesc.stride;
			RenderGraphBufferDesc sizeSrcDesc;
			build.getBufferDesc(passData.bufferInputTypedBuffer, &sizeSrcDesc);
			desc.size = desc.stride * (((sizeSrcDesc.size + 0) * 1) / 1 + 0);
			passData.bufferOutputTypedBuffer = build.create("Buffer OutputTypedBuffer", desc);
		}

		// Buffer: OutputStructuredBuffer
		{
			RenderGraphBufferDesc desc;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			RenderGraphBufferDesc formatSrcDesc;
			build.getBufferDesc(passData.bufferInputStructuredBuffer, &formatSrcDesc);
			desc.stride = formatSrcDesc.stride;
			RenderGraphBufferDesc sizeSrcDesc;
			build.getBufferDesc(passData.bufferInputStructuredBuffer, &sizeSrcDesc);
			desc.size = desc.stride * (((sizeSrcDesc.size + 0) * 1) / 1 + 0);
			passData.bufferOutputStructuredBuffer = build.create("Buffer OutputStructuredBuffer", desc);
		}

		// Buffer: OutputTypedBufferRaw
		// An internal buffer used during the filtering process.
		{
			RenderGraphBufferDesc desc;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			RenderGraphBufferDesc formatSrcDesc;
			build.getBufferDesc(passData.bufferInputTypedBufferRaw, &formatSrcDesc);
			desc.stride = formatSrcDesc.stride;
			RenderGraphBufferDesc sizeSrcDesc;
			build.getBufferDesc(passData.bufferInputTypedBufferRaw, &sizeSrcDesc);
			desc.size = desc.stride * (((sizeSrcDesc.size + 0) * 1) / 1 + 0);
			passData.bufferOutputTypedBufferRaw = build.create("Buffer OutputTypedBufferRaw", desc);
		}

		// Buffer: OutputTypedStructBuffer
		// An internal buffer used during the filtering process.
		{
			RenderGraphBufferDesc desc;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			RenderGraphBufferDesc formatSrcDesc;
			build.getBufferDesc(passData.bufferInputTypedBuffer, &formatSrcDesc);
			desc.stride = formatSrcDesc.stride;
			RenderGraphBufferDesc sizeSrcDesc;
			build.getBufferDesc(passData.bufferInputTypedBuffer, &sizeSrcDesc);
			desc.size = desc.stride * (((sizeSrcDesc.size + 0) * 1) / 1 + 0);
			passData.bufferOutputTypedStructBuffer = build.create("Buffer OutputTypedStructBuffer", desc);
		}

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("buffertest InitPass", InitPass);

	// Shader Constants: _BufferTest_0CB
	{
		m_cb__BufferTest_0CB.alpha1 = settings.alpha1;
		m_cb__BufferTest_0CB.alpha2 = settings.alpha2;
		m_cb__BufferTest_0CB.gain = settings.gain;
	}

	// Compute Shader: BufferTest
	// BufferTest compute shader
	auto& BufferTestPass = [&](RenderGraphBuild& build)
	{
		auto bufferInputTyped = passData.bufferInputTypedBuffer = build.read(passData.bufferInputTypedBuffer, RenderBindFlags::ShaderResource);
		auto bufferOutputTyped = passData.bufferOutputTypedBuffer = build.write(passData.bufferOutputTypedBuffer, RenderBindFlags::UnorderedAccess);
		auto bufferInputTypedStruct = passData.bufferInputTypedStructBuffer = build.read(passData.bufferInputTypedStructBuffer, RenderBindFlags::ShaderResource);
		auto bufferOutputTypedStruct = passData.bufferOutputTypedStructBuffer = build.write(passData.bufferOutputTypedStructBuffer, RenderBindFlags::UnorderedAccess);
		auto bufferInputStructured = passData.bufferInputStructuredBuffer = build.read(passData.bufferInputStructuredBuffer, RenderBindFlags::ShaderResource);
		auto bufferOutputStructured = passData.bufferOutputStructuredBuffer = build.write(passData.bufferOutputStructuredBuffer, RenderBindFlags::UnorderedAccess);
		auto bufferInputTypedRaw = passData.bufferInputTypedBufferRaw = build.read(passData.bufferInputTypedBufferRaw, RenderBindFlags::ShaderResource);
		auto bufferOutputTypedRaw = passData.bufferOutputTypedBufferRaw = build.write(passData.bufferOutputTypedBufferRaw, RenderBindFlags::UnorderedAccess);

		RenderGraphBufferDesc sizeSrcDesc;
		build.getBufferDesc(passData.bufferInputTypedBuffer, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.size + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((1 + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((1 + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 64 - 1) / 64;
		dispatchHeight = (dispatchHeight + 1 - 1) / 1;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(buffertestPipeline::getStaticClassId(), buffertestPipeline::BufferTestPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__BufferTest_0CB);
			RenderBindingShaderResourceView srvs[] = {
				defaultBufferView(registry, bufferInputTyped),
				defaultBufferView(registry, bufferInputTypedStruct),
				defaultBufferView(registry, bufferInputStructured),
				defaultBufferView(registry, bufferInputTypedRaw),
				};
			RenderBindingUnorderedAccessView uavs[] = {
				defaultBufferRwView(registry, bufferOutputTyped),
				defaultBufferRwView(registry, bufferOutputTypedStruct),
				defaultBufferRwView(registry, bufferOutputStructured),
				defaultBufferRwView(registry, bufferOutputTypedRaw),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, constantsOffset, registry.createShaderViews(srvs, uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("buffertest BufferTest", BufferTestPass);
}

} // namespace halcyon
