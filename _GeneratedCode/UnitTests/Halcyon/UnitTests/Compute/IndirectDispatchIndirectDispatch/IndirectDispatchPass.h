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

class IndirectDispatchPipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(IndirectDispatchPipeline, IShaderPipeline);

public:
	HCY_PIPELINE_INFO(Clear_Render_Target, 0, 1);
	HCY_PIPELINE_INFO(Fill_Indirect_Dispatch_Count, 0, 1);
	HCY_PIPELINE_INFO(Do_Indirect_Dispatch_1, 0, 1);
	HCY_PIPELINE_INFO(Do_Indirect_Dispatch_2, 0, 1);

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class IndirectDispatchPass : public IIndirectDispatchPass
{
	HCY_DECLARE_CLASS(IndirectDispatchPass, IIndirectDispatchPass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addIndirectDispatchPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:

	struct Struct__Fill_Indirect_Dispatch_Count_0CB
	{
		glm::uvec3 Dispatch_Count_1 = {50, 50, 1};
		float _padding0 = 0.f;
		glm::uvec3 Dispatch_Count_2 = {100, 100, 1};
		float _padding1 = 0.f;
	};

	Struct__Fill_Indirect_Dispatch_Count_0CB m_cb__Fill_Indirect_Dispatch_Count_0CB;
};

} // namespace halcyon
