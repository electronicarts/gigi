//-----------------------------------------------------------------------------
// Copyright (c) Electronic Arts.  All rights reserved.
//-----------------------------------------------------------------------------
#pragma once

#include <Common/Render/RenderGraphPass.h>
#include <Common/Render/RenderGraphPassInterfaces.h>
#include <Common/Render/RenderPassPayload.h>
#include <Common/Render/ShaderPipelines.h>

namespace halcyon
{

class VariableAliasesPipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(VariableAliasesPipeline, IShaderPipeline);

public:
	HCY_PIPELINE_INFO(Set_Red, 0, 1);
	HCY_PIPELINE_INFO(Set_Green, 0, 1);
	HCY_PIPELINE_INFO(Set_Blue, 0, 1);

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class VariableAliasesPass : public IVariableAliasesPass
{
	HCY_DECLARE_CLASS(VariableAliasesPass, IVariableAliasesPass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addVariableAliasesPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:

	struct Struct__SetChannel_0CB
	{
		int _alias_Channel = 0;
		float _alias_Value = 0.f;
		glm::vec2 _padding0 = {0.f, 0.f};
	};

	struct Struct__SetChannel_1CB
	{
		int _alias_Channel = 0;
		float _alias_Value = 0.f;
		glm::vec2 _padding0 = {0.f, 0.f};
	};

	Struct__SetChannel_0CB m_cb__SetChannel_0CB_0;

	Struct__SetChannel_0CB m_cb__SetChannel_0CB_1;

	Struct__SetChannel_1CB m_cb__SetChannel_1CB_0;
};

} // namespace halcyon
