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

class Texture3DRW_PSPipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(Texture3DRW_PSPipeline, IShaderPipeline);

public:

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class Texture3DRW_PSPass : public ITexture3DRW_PSPass
{
	HCY_DECLARE_CLASS(Texture3DRW_PSPass, ITexture3DRW_PSPass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addTexture3DRW_PSPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:
	RenderGraphTextureDesc m_texture_loadedTexture_0Desc;
	RenderResourceHandle m_texture_loadedTexture_0;
};

} // namespace halcyon
