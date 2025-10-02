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

class YesVertexType_NoIndex_NoInstancePipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(YesVertexType_NoIndex_NoInstancePipeline, IShaderPipeline);

public:

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class YesVertexType_NoIndex_NoInstancePass : public IYesVertexType_NoIndex_NoInstancePass
{
	HCY_DECLARE_CLASS(YesVertexType_NoIndex_NoInstancePass, IYesVertexType_NoIndex_NoInstancePass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addYesVertexType_NoIndex_NoInstancePass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:

	struct Struct__VertexShaderCB
	{
		glm::mat4 ViewProjMtx = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
	};

	Struct__VertexShaderCB m_cb__VertexShaderCB;
};

} // namespace halcyon
