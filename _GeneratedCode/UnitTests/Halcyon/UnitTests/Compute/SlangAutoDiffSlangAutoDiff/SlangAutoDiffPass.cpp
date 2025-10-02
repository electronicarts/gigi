//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "SlangAutoDiffPass.h"
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

void SlangAutoDiffPipeline::loadPipeline(ShaderPipelines& pipeline)
{

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			InitializePipelineId,
			"Shaders/SlangAutoDiff/Initialize",
			ShaderSignatureDesc(
				{ ShaderParameter(InitializeSrvCount, InitializeUavCount) },
				{ }));
	}

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			GradientDescendPipelineId,
			"Shaders/SlangAutoDiff/GradientDescend",
			ShaderSignatureDesc(
				{ ShaderParameter(GradientDescendSrvCount, GradientDescendUavCount) },
				{ }));
	}

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			RenderPipelineId,
			"Shaders/SlangAutoDiff/Render",
			ShaderSignatureDesc(
				{ ShaderParameter(RenderSrvCount, RenderUavCount) },
				{ }));
	}
}

void SlangAutoDiffPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	pipeline.unloadPipeline(getStaticClassId(), InitializePipelineId);
	pipeline.unloadPipeline(getStaticClassId(), GradientDescendPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), RenderPipelineId);
}

void SlangAutoDiffPass::createGlobalResourcesHandles()
{
	m_bufferData = RenderSystem::createBuffer("SlangAutoDiff Data Buffer");
}

void SlangAutoDiffPass::deleteGlobalResourcesHandles()
{
	RenderSystem::destroyResource(m_bufferData);
}

void SlangAutoDiffPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{

	// Buffer: Data
	// first 2 floats is position, nect 2 floats are the gradient. Then 5 floats per Gaussian.
	{
		RenderGraphBufferDesc& desc = m_bufferDataDesc;
		desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
		desc.size = desc.stride * (((settings.NumGaussians + 0) * 5) / 1 + 4);
		for (auto& d : deviceGroup.allDevices())
		{
			HcyVerify(d->createBuffer(m_bufferData, m_bufferDataDesc, nullptr, "SlangAutoDiff Data Buffer"));
		}
	}
}

void SlangAutoDiffPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<SlangAutoDiffPassData>();

	d.bufferData = renderGraph.import("SlangAutoDiff Data Buffer", m_bufferData, m_bufferDataDesc);
}

void SlangAutoDiffPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
		d->destroyResource(m_bufferData);
	}
}

void SlangAutoDiffPass::addSlangAutoDiffPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.SlangAutoDiff;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<SlangAutoDiffPassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("SlangAutoDiff InitPass", InitPass);

	// Shader Constants: _Init_0CB
	{
		m_cb__Init_0CB.FrameIndex = settings.FrameIndex;
		m_cb__Init_0CB.MouseState = settings.MouseState;
		m_cb__Init_0CB.MouseStateLastFrame = settings.MouseStateLastFrame;
		m_cb__Init_0CB.NumGaussians = settings.NumGaussians;
		m_cb__Init_0CB.iResolution = settings.iResolution;
		m_cb__Init_0CB.initialized = settings.initialized;
	}

	// Compute Shader: Initialize
	auto& InitializePass = [&](RenderGraphBuild& build)
	{
		auto bufferData = passData.bufferData = build.write(passData.bufferData, RenderBindFlags::UnorderedAccess);

		uint32 dispatchWidth = ((1 + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((1 + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((1 + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 1 - 1) / 1;
		dispatchHeight = (dispatchHeight + 1 - 1) / 1;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(SlangAutoDiffPipeline::getStaticClassId(), SlangAutoDiffPipeline::InitializePipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__Init_0CB);
			RenderBindingUnorderedAccessView uavs[] = {
				defaultBufferRwView(registry, bufferData),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, constantsOffset, registry.createShaderViews(uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("SlangAutoDiff Initialize", InitializePass);

	// Shader Constants: _Render_0CB
	{
		m_cb__Render_0CB.NumGaussians = settings.NumGaussians;
		m_cb__Render_0CB.QuantizeDisplay = settings.QuantizeDisplay;
	}

	// Shader Constants: _Descend_0CB
	{
		m_cb__Descend_0CB.LearningRate = settings.LearningRate;
		m_cb__Descend_0CB.MaximumStepSize = settings.MaximumStepSize;
		m_cb__Descend_0CB.NumGaussians = settings.NumGaussians;
		m_cb__Descend_0CB.UseBackwardAD = settings.UseBackwardAD;
	}

	// Compute Shader: GradientDescend
	auto& GradientDescendPass = [&](RenderGraphBuild& build)
	{
		auto bufferData = passData.bufferData = build.write(passData.bufferData, RenderBindFlags::UnorderedAccess);

		uint32 dispatchWidth = ((1 + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((1 + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((1 + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 1 - 1) / 1;
		dispatchHeight = (dispatchHeight + 1 - 1) / 1;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(SlangAutoDiffPipeline::getStaticClassId(), SlangAutoDiffPipeline::GradientDescendPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__Descend_0CB);
			RenderBindingUnorderedAccessView uavs[] = {
				defaultBufferRwView(registry, bufferData),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, constantsOffset, registry.createShaderViews(uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("SlangAutoDiff GradientDescend", GradientDescendPass);

	// Compute Shader: Render
	auto& RenderPass = [&](RenderGraphBuild& build)
	{
		auto bufferData = passData.bufferData = build.read(passData.bufferData, RenderBindFlags::ShaderResource);
		auto textureOutput = passData.textureOutput = build.write(passData.textureOutput, RenderBindFlags::UnorderedAccess);

		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureOutput, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(SlangAutoDiffPipeline::getStaticClassId(), SlangAutoDiffPipeline::RenderPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__Render_0CB);
			RenderBindingShaderResourceView srvs[] = {
				defaultBufferView(registry, bufferData),
				};
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dRwView(registry, textureOutput),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, constantsOffset, registry.createShaderViews(srvs, uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("SlangAutoDiff Render", RenderPass);
}

} // namespace halcyon
