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

class SubInSubPipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(SubInSubPipeline, IShaderPipeline);

public:
	HCY_PIPELINE_INFO(Inner1_Inner2_Rotate_Colors, 1, 1);

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class SubInSubPass : public ISubInSubPass
{
	HCY_DECLARE_CLASS(SubInSubPass, ISubInSubPass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addSubInSubPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:

	struct Struct__Inner1_Inner2_Inner2CS_0CB
	{
		glm::vec4 Inner1_Inner1Mult = {1.f, 0.5f, 2.f, 1.f};
	};

	Struct__Inner1_Inner2_Inner2CS_0CB m_cb__Inner1_Inner2_Inner2CS_0CB;
};

} // namespace halcyon
