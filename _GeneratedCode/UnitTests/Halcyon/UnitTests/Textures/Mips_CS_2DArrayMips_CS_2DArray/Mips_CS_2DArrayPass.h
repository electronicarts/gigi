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

class Mips_CS_2DArrayPipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(Mips_CS_2DArrayPipeline, IShaderPipeline);

public:
	HCY_PIPELINE_INFO(Mip0, 1, 1);
	HCY_PIPELINE_INFO(Mip1, 0, 2);
	HCY_PIPELINE_INFO(Mip2, 0, 2);
	HCY_PIPELINE_INFO(Mip3, 0, 2);

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class Mips_CS_2DArrayPass : public IMips_CS_2DArrayPass
{
	HCY_DECLARE_CLASS(Mips_CS_2DArrayPass, IMips_CS_2DArrayPass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addMips_CS_2DArrayPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:
	RenderGraphTextureDesc m_texture_loadedTexture_0Desc;
	RenderResourceHandle m_texture_loadedTexture_0;
};

} // namespace halcyon
