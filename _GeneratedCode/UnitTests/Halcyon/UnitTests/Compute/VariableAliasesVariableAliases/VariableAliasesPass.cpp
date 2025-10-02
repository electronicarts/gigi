//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#include "VariableAliasesPass.h"
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

void VariableAliasesPipeline::loadPipeline(ShaderPipelines& pipeline)
{

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			Set_RedPipelineId,
			"Shaders/VariableAliases/Set_Red",
			ShaderSignatureDesc(
				{ ShaderParameter(Set_RedSrvCount, Set_RedUavCount) },
				{ }));
	}

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			Set_GreenPipelineId,
			"Shaders/VariableAliases/Set_Green",
			ShaderSignatureDesc(
				{ ShaderParameter(Set_GreenSrvCount, Set_GreenUavCount) },
				{ }));
	}

	{
		pipeline.loadGenericComputePipeline(
			getStaticClassId(),
			Set_BluePipelineId,
			"Shaders/VariableAliases/Set_Blue",
			ShaderSignatureDesc(
				{ ShaderParameter(Set_BlueSrvCount, Set_BlueUavCount) },
				{ }));
	}
}

void VariableAliasesPipeline::unloadPipeline(ShaderPipelines& pipeline)
{
	pipeline.unloadPipeline(getStaticClassId(), Set_RedPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), Set_GreenPipelineId);
	pipeline.unloadPipeline(getStaticClassId(), Set_BluePipelineId);
}

void VariableAliasesPass::createGlobalResourcesHandles()
{
}

void VariableAliasesPass::deleteGlobalResourcesHandles()
{
}

void VariableAliasesPass::createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info)
{
}

void VariableAliasesPass::importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info)
{
	auto& d = scope.add<VariableAliasesPassData>();

}

void VariableAliasesPass::deleteGlobalResources(const IRenderDeviceGroup& deviceGroup)
{
	for (auto& d : deviceGroup.allDevices())
	{
	}
}

void VariableAliasesPass::addVariableAliasesPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines)
{
	auto& settings = scope.get<RenderGraphViewData>().renderSettings.VariableAliases;
	auto& dynamicConstants	= scope.get<RenderGraphDynamicConstants>();
	auto& passData = scope.get<VariableAliasesPassData>();

	// Initialization - create textures internal to the technique
	auto& InitPass = [&](RenderGraphBuild& build)
	{

		// Texture: Color
		{
			RenderGraphTextureDesc desc;
			desc.type = RenderTextureType::Tex2d;
			desc.bindFlags = RenderBindFlags::ShaderResource | RenderBindFlags::UnorderedAccess;
			desc.format = RenderFormat::R8G8B8A8_UNORM;
			desc.width = (unsigned int)settings.RenderSize[0];
			desc.height = (unsigned int)settings.RenderSize[1];
			desc.depth = 1;
			passData.textureColor = build.create("Texture Color", desc);
		}

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
		};
	};
	renderGraph.addPassCallback("VariableAliases InitPass", InitPass);

	// Shader Constants: _SetChannel_0CB_0
	{
		m_cb__SetChannel_0CB_0._alias_Channel = settings.Node1Channel;
		m_cb__SetChannel_0CB_0._alias_Value = settings.Node1Value;
	}

	// Compute Shader: Set_Red
	auto& Set_RedPass = [&](RenderGraphBuild& build)
	{
		auto textureOutput = passData.textureColor = build.write(passData.textureColor, RenderBindFlags::UnorderedAccess);

		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureColor, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(VariableAliasesPipeline::getStaticClassId(), VariableAliasesPipeline::Set_RedPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__SetChannel_0CB_0);
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dRwView(registry, textureOutput),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, constantsOffset, registry.createShaderViews(uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("VariableAliases Set_Red", Set_RedPass);

	// Shader Constants: _SetChannel_0CB_1
	{
		m_cb__SetChannel_0CB_1._alias_Channel = settings.Node2Channel;
		m_cb__SetChannel_0CB_1._alias_Value = settings.Node2Value;
	}

	// Compute Shader: Set_Green
	auto& Set_GreenPass = [&](RenderGraphBuild& build)
	{
		auto textureOutput = passData.textureColor = build.write(passData.textureColor, RenderBindFlags::UnorderedAccess);

		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureColor, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(VariableAliasesPipeline::getStaticClassId(), VariableAliasesPipeline::Set_GreenPipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__SetChannel_0CB_1);
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dRwView(registry, textureOutput),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, constantsOffset, registry.createShaderViews(uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("VariableAliases Set_Green", Set_GreenPass);

	// Shader Constants: _SetChannel_1CB_0
	{
		m_cb__SetChannel_1CB_0._alias_Channel = settings.Node3Channel;
		m_cb__SetChannel_1CB_0._alias_Value = settings.Node3Value;
	}

	// Compute Shader: Set_Blue
	auto& Set_BluePass = [&](RenderGraphBuild& build)
	{
		auto textureOutput = passData.textureColor = build.write(passData.textureColor, RenderBindFlags::UnorderedAccess);

		RenderGraphTextureDesc sizeSrcDesc;
		build.getTextureDesc(passData.textureColor, &sizeSrcDesc);
		uint32 dispatchWidth = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
		uint32 dispatchHeight = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
		uint32 dispatchDepth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;
		dispatchWidth = (dispatchWidth + 8 - 1) / 8;
		dispatchHeight = (dispatchHeight + 8 - 1) / 8;
		dispatchDepth = (dispatchDepth + 1 - 1) / 1;

		auto pipelineState = pipelines.pipelineState(VariableAliasesPipeline::getStaticClassId(), VariableAliasesPipeline::Set_BluePipelineId);

		return [=](RenderGraphRegistry& registry, RenderCommandList& commandList)
		{
			usize constantsOffset = dynamicConstants.constants->allocateAndMemcpy(m_cb__SetChannel_1CB_0);
			RenderBindingUnorderedAccessView uavs[] = {
				defaultTexture2dRwView(registry, textureOutput),
				};
			commandList.dispatch3d(
				pipelineState,{ ShaderArgument(dynamicConstants.buffer, constantsOffset, registry.createShaderViews(uavs)) },
				dispatchWidth, dispatchHeight, dispatchDepth,
				1, 1, 1);
		};
	};

	renderGraph.addPassCallback("VariableAliases Set_Blue", Set_BluePass);
}

} // namespace halcyon
