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

class YesVertexStruct_YesIndex_NoInstancePipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(YesVertexStruct_YesIndex_NoInstancePipeline, IShaderPipeline);

public:

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class YesVertexStruct_YesIndex_NoInstancePass : public IYesVertexStruct_YesIndex_NoInstancePass
{
	HCY_DECLARE_CLASS(YesVertexStruct_YesIndex_NoInstancePass, IYesVertexStruct_YesIndex_NoInstancePass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addYesVertexStruct_YesIndex_NoInstancePass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:

	struct Struct_VertexFormat
	{
		glm::vec3 Position = {0.f, 0.f, 0.f};
		glm::vec2 UV = {0.f, 0.f};
	};

	struct Struct__VertexShaderCB
	{
		glm::mat4 ViewProjMtx = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
	};

	struct Struct__PixelShaderCB
	{
		int viewMode = (int)YesVertexStruct_YesIndex_NoInstanceSettings::ViewMode::UV;
		glm::vec3 _padding0 = {0.f, 0.f, 0.f};
	};

	Struct__VertexShaderCB m_cb__VertexShaderCB;

	Struct__PixelShaderCB m_cb__PixelShaderCB;
};

} // namespace halcyon
