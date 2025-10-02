//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "IndirectDispatchPass.h"
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

void IndirectDispatchPipeline::loadPipeline(ShaderPipelines& pipeline)
{

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			Clear_Render_TargetPipelineId,
			"Shaders/IndirectDispatch/Clear_Render_Target",
			ShaderSignatureDesc(
				{ ShaderParameter(Clear_Render_TargetSrvCount, Clear_Render_TargetUavCount) },
				{ }));
	}

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			Fill_Indirect_Dispatch_CountPipelineId,
			"Shaders/IndirectDispatch/Fill_Indirect_Dispatch_Count",
			ShaderSignatureDesc(
				{ ShaderParameter(Fill_Indirect_Dispatch_CountSrvCount, Fill_Indirect_Dispatch_CountUavCount) },
				{ }));
	}

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			Do_Indirect_Dispatch_1PipelineId,
			"Shaders/IndirectDispatch/Do_Indirect_Dispatch_1",
			ShaderSignatureDesc(
				{ ShaderParameter(Do_Indirect_Dispatch_1SrvCount, Do_Indirect_Dispatch_1UavCount) },
				{ }));
	}

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			Do_Indirect_Dispatch_2PipelineId,
			"Shaders/IndirectDispatch/Do_Indirect_Dispatch_2",
			ShaderSignatureDesc(
				{ ShaderParameter(Do_Indirect_Dispatch_2SrvCount, Do_Indirect_Dispatch_2UavCount) },
				{ }));
	}
}

void IndirectDispatchPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	pipeline.unloadPipeline(getStaticClassId(), Clear_Render_TargetPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), Fill_Indirect_Dispatch_CountPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), Do_Indirect_Dispatch_1PipelineId);
	pipeline.unloadPipeline(getStaticClassId(), Do_Indirect_Dispatch_2PipelineId);
}

void IndirectDispatchPass::createGlobalResourcesHandles()
{
}

void IndirectDispatchPass::deleteGlobalResourcesHandles()
{
}

void IndirectDispatchPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void IndirectDispatchPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<IndirectDispatchPassData>();

}

void IndirectDispatchPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void IndirectDispatchPass::addIndirectDispatchPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.IndirectDispatch;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<IndirectDispatchPassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		// Texture: Render_Target
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2d;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			desc.format = RenderFormat::R8G8B8A8_SRGB;
			desc.width = ((1 + 0)256) / 1 + 0;
			desc.height = ((1 + 0)256) / 1 + 0;
			passData.textureRender_Target = build.create("Texture Render_Target", desc);
		}

		// Buffer: Indirect_Dispatch_Count
		{
			RenderGraphBufferDesc desc;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			desc.size = desc.stride * (((1 + 0) * 12) / 1 + 0);
			passData.bufferIndirect_Dispatch_Count = build.create("Buffer Indirect_Dispatch_Count", desc);
		}

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("IndirectDispatch InitPass", InitPass);

	// Compute Shader: Clear_Render_Target
	auto& Clear_Render_TargetPass = [&](RenderGraphBuild& build)
	{
		auto textureRender_Target = passData.textureRender_Target = build.write(passData.textureRender_Target, RenderBindFlags::UnorderedAccess);

		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureRender_Target, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(IndirectDispatchPipeline::getStaticClassId(), IndirectDispatchPipeline::Clear_Render_TargetPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dRwView(registry, textureRender_Target),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, registry.createShaderViews(uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("IndirectDispatch Clear_Render_Target", Clear_Render_TargetPass);

	// Shader Constants: _Fill_Indirect_Dispatch_Count_0CB
	{
		m_cb__Fill_Indirect_Dispatch_Count_0CB.Dispatch_Count_1 = settings.Dispatch_Count_1;
		m_cb__Fill_Indirect_Dispatch_Count_0CB.Dispatch_Count_2 = settings.Dispatch_Count_2;
	}

	// Compute Shader: Fill_Indirect_Dispatch_Count
	auto& Fill_Indirect_Dispatch_CountPass = [&](RenderGraphBuild& build)
	{
		auto bufferIndirectDispatchCount = passData.bufferIndirect_Dispatch_Count = build.write(passData.bufferIndirect_Dispatch_Count, RenderBindFlags::UnorderedAccess);

		uint32 dispatchWidth = ((1 + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((1 + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((1 + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 1 - 1) / 1;
		dispatchHeight = (dispatchHeight + 1 - 1) / 1;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(IndirectDispatchPipeline::getStaticClassId(), IndirectDispatchPipeline::Fill_Indirect_Dispatch_CountPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__Fill_Indirect_Dispatch_Count_0CB);
			RenderBindingUnorderedAccessView uavs[] = {
				defaultBufferRwView(registry, bufferIndirectDispatchCount),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, constantsOffset, registry.createShaderViews(uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("IndirectDispatch Fill_Indirect_Dispatch_Count", Fill_Indirect_Dispatch_CountPass);

	// Compute Shader: Do_Indirect_Dispatch_1
	auto& Do_Indirect_Dispatch_1Pass = [&](RenderGraphBuild& build)
	{
		auto textureRender_Target = passData.textureRender_Target = build.write(passData.textureRender_Target, RenderBindFlags::UnorderedAccess);
		auto indirectDispatchBuffer = build.read(passData.bufferIndirect_Dispatch_Count, RenderBindFlags::IndirectBuffer);

		auto pipelineState = pipelines.pipelineState(IndirectDispatchPipeline::getStaticClassId(), IndirectDispatchPipeline::Do_Indirect_Dispatch_1PipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dRwView(registry, textureRender_Target),
				};
			RenderResourceHandle dummy;

			commandList.dispatchIndirect(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, registry.createShaderViews(uavs)) },
				registry.getBuffer(indirectDispatchBuffer), 0,
				dummy, 0,
				1);
		};
	};

	renderGraph.addPassCallback("IndirectDispatch Do_Indirect_Dispatch_1", Do_Indirect_Dispatch_1Pass);

	// Compute Shader: Do_Indirect_Dispatch_2
	auto& Do_Indirect_Dispatch_2Pass = [&](RenderGraphBuild& build)
	{
		auto textureRender_Target = passData.textureRender_Target = build.write(passData.textureRender_Target, RenderBindFlags::UnorderedAccess);
		auto indirectDispatchBuffer = build.read(passData.bufferIndirect_Dispatch_Count, RenderBindFlags::IndirectBuffer);

		auto pipelineState = pipelines.pipelineState(IndirectDispatchPipeline::getStaticClassId(), IndirectDispatchPipeline::Do_Indirect_Dispatch_2PipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dRwView(registry, textureRender_Target),
				};
			RenderResourceHandle dummy;

			commandList.dispatchIndirect(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, registry.createShaderViews(uavs)) },
				registry.getBuffer(indirectDispatchBuffer), 0,
				dummy, 0,
				1);
		};
	};

	renderGraph.addPassCallback("IndirectDispatch Do_Indirect_Dispatch_2", Do_Indirect_Dispatch_2Pass);
}

} // namespace halcyon
