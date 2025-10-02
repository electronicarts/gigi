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

class TextureCubeRW_RGSPipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(TextureCubeRW_RGSPipeline, IShaderPipeline);

public:
	HCY_PIPELINE_INFO(RWCHS, 0, 0);
	HCY_PIPELINE_INFO(RWMISS, 0, 0);
	HCY_PIPELINE_INFO(RW, 2, 2);

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class TextureCubeRW_RGSPass : public ITextureCubeRW_RGSPass
{
	HCY_DECLARE_CLASS(TextureCubeRW_RGSPass, ITextureCubeRW_RGSPass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addTextureCubeRW_RGSPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:

	static unsigned int m_variable__chindex_RWCHS;
	static unsigned int m_variable__missindex_RWMISS;
	static unsigned int m_variable__rt_hit_group_count;
	RenderGraphTextureDesc m_texture_loadedTexture_0Desc;
	RenderResourceHandle m_texture_loadedTexture_0;
};

} // namespace halcyon
