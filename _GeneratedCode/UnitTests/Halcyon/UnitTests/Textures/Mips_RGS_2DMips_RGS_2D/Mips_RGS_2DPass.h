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

class Mips_RGS_2DPipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(Mips_RGS_2DPipeline, IShaderPipeline);

public:
	HCY_PIPELINE_INFO(Miss, 0, 0);
	HCY_PIPELINE_INFO(RayGen, 1, 1);

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class Mips_RGS_2DPass : public IMips_RGS_2DPass
{
	HCY_DECLARE_CLASS(Mips_RGS_2DPass, IMips_RGS_2DPass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addMips_RGS_2DPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:

	static unsigned int m_variable__missindex_Miss;
	static unsigned int m_variable__rt_hit_group_count;
};

} // namespace halcyon
