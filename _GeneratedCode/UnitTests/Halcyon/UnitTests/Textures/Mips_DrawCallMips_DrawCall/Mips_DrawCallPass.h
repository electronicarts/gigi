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

class Mips_DrawCallPipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(Mips_DrawCallPipeline, IShaderPipeline);

public:

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class Mips_DrawCallPass : public IMips_DrawCallPass
{
	HCY_DECLARE_CLASS(Mips_DrawCallPass, IMips_DrawCallPass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addMips_DrawCallPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:

	struct Struct_VertexFormat
	{
		glm::vec3 Position = {0.f, 0.f, 0.f};
		glm::vec3 Normal = {0.f, 0.f, 0.f};
		glm::vec2 UV = {0.f, 0.f};
	};

	struct Struct__VertexShaderCB
	{
		glm::mat4 ViewProjMtx = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
	};

	struct Struct__VertexShaderSphereCB
	{
		glm::mat4 ViewProjMtx = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
	};

	struct Struct__PixelShaderSphereCB
	{
		unsigned int UseMips = true;
		glm::vec3 _padding0 = {0.f, 0.f, 0.f};
	};

	Struct__VertexShaderCB m_cb__VertexShaderCB;

	Struct__VertexShaderSphereCB m_cb__VertexShaderSphereCB;

	Struct__PixelShaderSphereCB m_cb__PixelShaderSphereCB;
};

} // namespace halcyon
