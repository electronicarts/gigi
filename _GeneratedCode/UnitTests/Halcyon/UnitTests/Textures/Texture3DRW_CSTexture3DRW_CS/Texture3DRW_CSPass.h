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

class Texture3DRW_CSPipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(Texture3DRW_CSPipeline, IShaderPipeline);

public:
	HCY_PIPELINE_INFO(RW, 2, 2);

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class Texture3DRW_CSPass : public ITexture3DRW_CSPass
{
	HCY_DECLARE_CLASS(Texture3DRW_CSPass, ITexture3DRW_CSPass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addTexture3DRW_CSPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:
	RenderGraphTextureDesc m_texture_loadedTexture_0Desc;
	RenderResourceHandle m_texture_loadedTexture_0;
};

} // namespace halcyon
