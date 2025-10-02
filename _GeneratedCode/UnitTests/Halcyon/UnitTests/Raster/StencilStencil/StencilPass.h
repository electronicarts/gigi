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

class StencilPipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(StencilPipeline, IShaderPipeline);

public:

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class StencilPass : public IStencilPass
{
	HCY_DECLARE_CLASS(StencilPass, IStencilPass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addStencilPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:

	struct Struct_VertexBuffer
	{
		glm::vec3 pos = {0.f, 0.f, 0.f};
		glm::vec3 normal = {0.f, 0.f, 0.f};
		glm::vec2 uv = {0.f, 0.f};
	};

	struct Struct__Draw1VSCB
	{
		glm::mat4 ViewProjMtx = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
	};
	RenderGraphTextureDesc m_textureDepth_StencilDesc;
	RenderResourceHandle m_textureDepth_Stencil;

	Struct__Draw1VSCB m_cb__Draw1VSCB;
	RenderGraphTextureDesc m_texture_loadedTexture_0Desc;
	RenderResourceHandle m_texture_loadedTexture_0;
};

} // namespace halcyon
