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

class ConstOverridePipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(ConstOverridePipeline, IShaderPipeline);

public:
	HCY_PIPELINE_INFO(Clear, 0, 1);
	HCY_PIPELINE_INFO(Left_WriteColor, 0, 1);
	HCY_PIPELINE_INFO(Right_WriteColor, 0, 1);

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class ConstOverridePass : public IConstOverridePass
{
	HCY_DECLARE_CLASS(ConstOverridePass, IConstOverridePass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addConstOverridePass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:

	const unsigned int m_variable___literal_0 = 50;  // Made to replace variable "MinX" with a constant value in subgraph node "Left"
	const unsigned int m_variable___literal_1 = 100;  // Made to replace variable "MaxX" with a constant value in subgraph node "Left"
	const unsigned int m_variable___literal_2 = 150;  // Made to replace variable "MinX" with a constant value in subgraph node "Right"
	const unsigned int m_variable___literal_3 = 200;  // Made to replace variable "MaxX" with a constant value in subgraph node "Right"
};

} // namespace halcyon
