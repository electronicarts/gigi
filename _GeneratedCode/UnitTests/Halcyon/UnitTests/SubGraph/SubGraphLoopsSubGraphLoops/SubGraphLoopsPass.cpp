//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "SubGraphLoopsPass.h"
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

void SubGraphLoopsPipeline::loadPipeline(ShaderPipelines& pipeline)
{

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			FilterSub_Iteration_0_DoBlurPipelineId,
			"Shaders/SubGraphLoops/FilterSub_Iteration_0_DoBlur",
			ShaderSignatureDesc(
				{ ShaderParameter(FilterSub_Iteration_0_DoBlurSrvCount, FilterSub_Iteration_0_DoBlurUavCount) },
				{ }));
	}

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			FilterSub_Iteration_1_DoBlurPipelineId,
			"Shaders/SubGraphLoops/FilterSub_Iteration_1_DoBlur",
			ShaderSignatureDesc(
				{ ShaderParameter(FilterSub_Iteration_1_DoBlurSrvCount, FilterSub_Iteration_1_DoBlurUavCount) },
				{ }));
	}

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			FilterSub_Iteration_2_DoBlurPipelineId,
			"Shaders/SubGraphLoops/FilterSub_Iteration_2_DoBlur",
			ShaderSignatureDesc(
				{ ShaderParameter(FilterSub_Iteration_2_DoBlurSrvCount, FilterSub_Iteration_2_DoBlurUavCount) },
				{ }));
	}

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			FilterSub_Iteration_3_DoBlurPipelineId,
			"Shaders/SubGraphLoops/FilterSub_Iteration_3_DoBlur",
			ShaderSignatureDesc(
				{ ShaderParameter(FilterSub_Iteration_3_DoBlurSrvCount, FilterSub_Iteration_3_DoBlurUavCount) },
				{ }));
	}

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			FilterSub_Iteration_4_DoBlurPipelineId,
			"Shaders/SubGraphLoops/FilterSub_Iteration_4_DoBlur",
			ShaderSignatureDesc(
				{ ShaderParameter(FilterSub_Iteration_4_DoBlurSrvCount, FilterSub_Iteration_4_DoBlurUavCount) },
				{ }));
	}
}

void SubGraphLoopsPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	pipeline.unloadPipeline(getStaticClassId(), FilterSub_Iteration_0_DoBlurPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), FilterSub_Iteration_1_DoBlurPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), FilterSub_Iteration_2_DoBlurPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), FilterSub_Iteration_3_DoBlurPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), FilterSub_Iteration_4_DoBlurPipelineId);
}

void SubGraphLoopsPass::createGlobalResourcesHandles()
{
}

void SubGraphLoopsPass::deleteGlobalResourcesHandles()
{
}

void SubGraphLoopsPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void SubGraphLoopsPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<SubGraphLoopsPassData>();

}

void SubGraphLoopsPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void SubGraphLoopsPass::addSubGraphLoopsPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.SubGraphLoops;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<SubGraphLoopsPassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		// Texture: FilterSub_Iteration_0_Output
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2d;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			RenderGraphTextureDesc formatSrcDesc;
			build.getTextureDesc(passData.textureInput, &formatSrcDesc);
			desc.format = formatSrcDesc.format;
			RenderGraphTextureDesc sizeSrcDesc;
			build.getTextureDesc(passData.textureInput, &sizeSrcDesc);
			desc.width = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
			desc.height = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
			passData.textureFilterSub_Iteration_0_Output = build.create("Texture FilterSub_Iteration_0_Output", desc);
		}

		// Texture: FilterSub_Iteration_1_Output
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2d;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			RenderGraphTextureDesc formatSrcDesc;
			build.getTextureDesc(passData.textureInput, &formatSrcDesc);
			desc.format = formatSrcDesc.format;
			RenderGraphTextureDesc sizeSrcDesc;
			build.getTextureDesc(passData.textureInput, &sizeSrcDesc);
			desc.width = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
			desc.height = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
			passData.textureFilterSub_Iteration_1_Output = build.create("Texture FilterSub_Iteration_1_Output", desc);
		}

		// Texture: FilterSub_Iteration_2_Output
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2d;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			RenderGraphTextureDesc formatSrcDesc;
			build.getTextureDesc(passData.textureInput, &formatSrcDesc);
			desc.format = formatSrcDesc.format;
			RenderGraphTextureDesc sizeSrcDesc;
			build.getTextureDesc(passData.textureInput, &sizeSrcDesc);
			desc.width = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
			desc.height = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
			passData.textureFilterSub_Iteration_2_Output = build.create("Texture FilterSub_Iteration_2_Output", desc);
		}

		// Texture: FilterSub_Iteration_3_Output
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2d;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			RenderGraphTextureDesc formatSrcDesc;
			build.getTextureDesc(passData.textureInput, &formatSrcDesc);
			desc.format = formatSrcDesc.format;
			RenderGraphTextureDesc sizeSrcDesc;
			build.getTextureDesc(passData.textureInput, &sizeSrcDesc);
			desc.width = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
			desc.height = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
			passData.textureFilterSub_Iteration_3_Output = build.create("Texture FilterSub_Iteration_3_Output", desc);
		}

		// Texture: FilterSub_Iteration_4_Output
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2d;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			RenderGraphTextureDesc formatSrcDesc;
			build.getTextureDesc(passData.textureInput, &formatSrcDesc);
			desc.format = formatSrcDesc.format;
			RenderGraphTextureDesc sizeSrcDesc;
			build.getTextureDesc(passData.textureInput, &sizeSrcDesc);
			desc.width = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
			desc.height = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
			passData.textureFilterSub_Iteration_4_Output = build.create("Texture FilterSub_Iteration_4_Output", desc);
		}

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("SubGraphLoops InitPass", InitPass);

	// Shader Constants: _FilterSub_Iteration_0_Blur_0CB
	{
		m_cb__FilterSub_Iteration_0_Blur_0CB.FilterSub_Iteration_0_sRGB = settings.FilterSub_Iteration_0_sRGB;
		m_cb__FilterSub_Iteration_0_Blur_0CB.__loopIndexValue_0 = m_variable___loopIndexValue_0;
	}

	// Compute Shader: FilterSub_Iteration_0_DoBlur
	auto& FilterSub_Iteration_0_DoBlurPass = [&](RenderGraphBuild& build)
	{
		auto textureInput = passData.textureInput = build.read(passData.textureInput, RenderBindFlags::ShaderResource);
		auto textureOutput = passData.textureFilterSub_Iteration_0_Output = build.write(passData.textureFilterSub_Iteration_0_Output, RenderBindFlags::UnorderedAccess);

		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureInput, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(SubGraphLoopsPipeline::getStaticClassId(), SubGraphLoopsPipeline::FilterSub_Iteration_0_DoBlurPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__FilterSub_Iteration_0_Blur_0CB);
			RenderBindingShaderResourceView srvs[] = {
				defaultTexture2dView(registry, textureInput),
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

	renderGraph.addPassCallback("SubGraphLoops FilterSub_Iteration_0_DoBlur", FilterSub_Iteration_0_DoBlurPass);

	// Copy Resource: FilterSub_Iteration_0_Copy_back
	auto& FilterSub_Iteration_0_Copy_backPass = [&](RenderGraphBuild& build)
	{
		RenderGraphTextureDesc srcDesc;
		build.getTextureDesc(passData.textureFilterSub_Iteration_0_Output, &srcDesc);

		passData.textureInput = build.writeAndUpdate(passData.textureInput, RenderBindFlags::UnorderedAccess);
		passData.textureFilterSub_Iteration_0_Output = build.read(passData.textureFilterSub_Iteration_0_Output, RenderBindFlags::ShaderResource);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderPoint point = {};
			RenderBox box	  = {};
			box.w = srcDesc.width;
			box.h = srcDesc.height;

			commandList.copyTexture(
				registry.getTexture(passData.textureInput), 0, point,
				registry.getTexture(passData.textureFilterSub_Iteration_0_Output), 0, box);
		};
	};

	renderGraph.addPassCallback("SubGraphLoops FilterSub_Iteration_0_Copy_back", FilterSub_Iteration_0_Copy_backPass);

	// Shader Constants: _FilterSub_Iteration_1_Blur_0CB
	{
		m_cb__FilterSub_Iteration_1_Blur_0CB.FilterSub_Iteration_1_sRGB = settings.FilterSub_Iteration_1_sRGB;
		m_cb__FilterSub_Iteration_1_Blur_0CB.__loopIndexValue_1 = m_variable___loopIndexValue_1;
	}

	// Compute Shader: FilterSub_Iteration_1_DoBlur
	auto& FilterSub_Iteration_1_DoBlurPass = [&](RenderGraphBuild& build)
	{
		auto textureInput = passData.textureInput = build.read(passData.textureInput, RenderBindFlags::ShaderResource);
		auto textureOutput = passData.textureFilterSub_Iteration_1_Output = build.write(passData.textureFilterSub_Iteration_1_Output, RenderBindFlags::UnorderedAccess);

		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureInput, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(SubGraphLoopsPipeline::getStaticClassId(), SubGraphLoopsPipeline::FilterSub_Iteration_1_DoBlurPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__FilterSub_Iteration_1_Blur_0CB);
			RenderBindingShaderResourceView srvs[] = {
				defaultTexture2dView(registry, textureInput),
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

	renderGraph.addPassCallback("SubGraphLoops FilterSub_Iteration_1_DoBlur", FilterSub_Iteration_1_DoBlurPass);

	// Copy Resource: FilterSub_Iteration_1_Copy_back
	auto& FilterSub_Iteration_1_Copy_backPass = [&](RenderGraphBuild& build)
	{
		RenderGraphTextureDesc srcDesc;
		build.getTextureDesc(passData.textureFilterSub_Iteration_1_Output, &srcDesc);

		passData.textureInput = build.writeAndUpdate(passData.textureInput, RenderBindFlags::UnorderedAccess);
		passData.textureFilterSub_Iteration_1_Output = build.read(passData.textureFilterSub_Iteration_1_Output, RenderBindFlags::ShaderResource);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderPoint point = {};
			RenderBox box	  = {};
			box.w = srcDesc.width;
			box.h = srcDesc.height;

			commandList.copyTexture(
				registry.getTexture(passData.textureInput), 0, point,
				registry.getTexture(passData.textureFilterSub_Iteration_1_Output), 0, box);
		};
	};

	renderGraph.addPassCallback("SubGraphLoops FilterSub_Iteration_1_Copy_back", FilterSub_Iteration_1_Copy_backPass);

	// Shader Constants: _FilterSub_Iteration_2_Blur_0CB
	{
		m_cb__FilterSub_Iteration_2_Blur_0CB.FilterSub_Iteration_2_sRGB = settings.FilterSub_Iteration_2_sRGB;
		m_cb__FilterSub_Iteration_2_Blur_0CB.__loopIndexValue_2 = m_variable___loopIndexValue_2;
	}

	// Compute Shader: FilterSub_Iteration_2_DoBlur
	auto& FilterSub_Iteration_2_DoBlurPass = [&](RenderGraphBuild& build)
	{
		auto textureInput = passData.textureInput = build.read(passData.textureInput, RenderBindFlags::ShaderResource);
		auto textureOutput = passData.textureFilterSub_Iteration_2_Output = build.write(passData.textureFilterSub_Iteration_2_Output, RenderBindFlags::UnorderedAccess);

		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureInput, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(SubGraphLoopsPipeline::getStaticClassId(), SubGraphLoopsPipeline::FilterSub_Iteration_2_DoBlurPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__FilterSub_Iteration_2_Blur_0CB);
			RenderBindingShaderResourceView srvs[] = {
				defaultTexture2dView(registry, textureInput),
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

	renderGraph.addPassCallback("SubGraphLoops FilterSub_Iteration_2_DoBlur", FilterSub_Iteration_2_DoBlurPass);

	// Copy Resource: FilterSub_Iteration_2_Copy_back
	auto& FilterSub_Iteration_2_Copy_backPass = [&](RenderGraphBuild& build)
	{
		RenderGraphTextureDesc srcDesc;
		build.getTextureDesc(passData.textureFilterSub_Iteration_2_Output, &srcDesc);

		passData.textureInput = build.writeAndUpdate(passData.textureInput, RenderBindFlags::UnorderedAccess);
		passData.textureFilterSub_Iteration_2_Output = build.read(passData.textureFilterSub_Iteration_2_Output, RenderBindFlags::ShaderResource);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderPoint point = {};
			RenderBox box	  = {};
			box.w = srcDesc.width;
			box.h = srcDesc.height;

			commandList.copyTexture(
				registry.getTexture(passData.textureInput), 0, point,
				registry.getTexture(passData.textureFilterSub_Iteration_2_Output), 0, box);
		};
	};

	renderGraph.addPassCallback("SubGraphLoops FilterSub_Iteration_2_Copy_back", FilterSub_Iteration_2_Copy_backPass);

	// Shader Constants: _FilterSub_Iteration_3_Blur_0CB
	{
		m_cb__FilterSub_Iteration_3_Blur_0CB.FilterSub_Iteration_3_sRGB = settings.FilterSub_Iteration_3_sRGB;
		m_cb__FilterSub_Iteration_3_Blur_0CB.__loopIndexValue_3 = m_variable___loopIndexValue_3;
	}

	// Compute Shader: FilterSub_Iteration_3_DoBlur
	auto& FilterSub_Iteration_3_DoBlurPass = [&](RenderGraphBuild& build)
	{
		auto textureInput = passData.textureInput = build.read(passData.textureInput, RenderBindFlags::ShaderResource);
		auto textureOutput = passData.textureFilterSub_Iteration_3_Output = build.write(passData.textureFilterSub_Iteration_3_Output, RenderBindFlags::UnorderedAccess);

		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureInput, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(SubGraphLoopsPipeline::getStaticClassId(), SubGraphLoopsPipeline::FilterSub_Iteration_3_DoBlurPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__FilterSub_Iteration_3_Blur_0CB);
			RenderBindingShaderResourceView srvs[] = {
				defaultTexture2dView(registry, textureInput),
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

	renderGraph.addPassCallback("SubGraphLoops FilterSub_Iteration_3_DoBlur", FilterSub_Iteration_3_DoBlurPass);

	// Copy Resource: FilterSub_Iteration_3_Copy_back
	auto& FilterSub_Iteration_3_Copy_backPass = [&](RenderGraphBuild& build)
	{
		RenderGraphTextureDesc srcDesc;
		build.getTextureDesc(passData.textureFilterSub_Iteration_3_Output, &srcDesc);

		passData.textureInput = build.writeAndUpdate(passData.textureInput, RenderBindFlags::UnorderedAccess);
		passData.textureFilterSub_Iteration_3_Output = build.read(passData.textureFilterSub_Iteration_3_Output, RenderBindFlags::ShaderResource);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderPoint point = {};
			RenderBox box	  = {};
			box.w = srcDesc.width;
			box.h = srcDesc.height;

			commandList.copyTexture(
				registry.getTexture(passData.textureInput), 0, point,
				registry.getTexture(passData.textureFilterSub_Iteration_3_Output), 0, box);
		};
	};

	renderGraph.addPassCallback("SubGraphLoops FilterSub_Iteration_3_Copy_back", FilterSub_Iteration_3_Copy_backPass);

	// Shader Constants: _FilterSub_Iteration_4_Blur_0CB
	{
		m_cb__FilterSub_Iteration_4_Blur_0CB.FilterSub_Iteration_4_sRGB = settings.FilterSub_Iteration_4_sRGB;
		m_cb__FilterSub_Iteration_4_Blur_0CB.__loopIndexValue_4 = m_variable___loopIndexValue_4;
	}

	// Compute Shader: FilterSub_Iteration_4_DoBlur
	auto& FilterSub_Iteration_4_DoBlurPass = [&](RenderGraphBuild& build)
	{
		auto textureInput = passData.textureInput = build.read(passData.textureInput, RenderBindFlags::ShaderResource);
		auto textureOutput = passData.textureFilterSub_Iteration_4_Output = build.write(passData.textureFilterSub_Iteration_4_Output, RenderBindFlags::UnorderedAccess);

		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureInput, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(SubGraphLoopsPipeline::getStaticClassId(), SubGraphLoopsPipeline::FilterSub_Iteration_4_DoBlurPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__FilterSub_Iteration_4_Blur_0CB);
			RenderBindingShaderResourceView srvs[] = {
				defaultTexture2dView(registry, textureInput),
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

	renderGraph.addPassCallback("SubGraphLoops FilterSub_Iteration_4_DoBlur", FilterSub_Iteration_4_DoBlurPass);

	// Copy Resource: FilterSub_Iteration_4_Copy_back
	auto& FilterSub_Iteration_4_Copy_backPass = [&](RenderGraphBuild& build)
	{
		RenderGraphTextureDesc srcDesc;
		build.getTextureDesc(passData.textureFilterSub_Iteration_4_Output, &srcDesc);

		passData.textureInput = build.writeAndUpdate(passData.textureInput, RenderBindFlags::UnorderedAccess);
		passData.textureFilterSub_Iteration_4_Output = build.read(passData.textureFilterSub_Iteration_4_Output, RenderBindFlags::ShaderResource);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			RenderPoint point = {};
			RenderBox box	  = {};
			box.w = srcDesc.width;
			box.h = srcDesc.height;

			commandList.copyTexture(
				registry.getTexture(passData.textureInput), 0, point,
				registry.getTexture(passData.textureFilterSub_Iteration_4_Output), 0, box);
		};
	};

	renderGraph.addPassCallback("SubGraphLoops FilterSub_Iteration_4_Copy_back", FilterSub_Iteration_4_Copy_backPass);
}

} // namespace halcyon
