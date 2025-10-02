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

class simpleRT_inlinePipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(simpleRT_inlinePipeline, IShaderPipeline);

public:
	HCY_PIPELINE_INFO(DoRT, 1, 1);

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class simpleRT_inlinePass : public IsimpleRT_inlinePass
{
	HCY_DECLARE_CLASS(simpleRT_inlinePass, IsimpleRT_inlinePass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addsimpleRT_inlinePass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:

	struct Struct_VertexBuffer
	{
		glm::vec3 Color = {0.f, 0.f, 0.f};
		glm::vec3 Position = {0.f, 0.f, 0.f};
		glm::vec3 Normal = {0.f, 0.f, 0.f};
	};

	struct Struct_VertexBufferSimple
	{
		glm::vec3 Position = {0.f, 0.f, 0.f};
	};

	struct Struct_VertexBufferFull
	{
		glm::vec3 Position = {0.f, 0.f, 0.f};
		glm::vec3 Color = {0.f, 0.f, 0.f};
		glm::vec3 Normal = {0.f, 0.f, 0.f};
		glm::vec4 Tangent = {0.f, 0.f, 0.f, 0.f};
		glm::vec2 UV = {0.f, 0.f};
		int MaterialID = 0;
	};

	struct Struct__SimpleRTCS_0CB
	{
		glm::vec3 cameraPos = {0.f, 0.f, 0.f};
		float _padding0 = 0.f;
		glm::mat4 clipToWorld = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
		float depthNearPlane = 0.f;
		glm::vec3 hitColor = {0.f, 1.f, 0.f};
		glm::vec3 missColor = {1.f, 0.f, 0.f};
		float _padding1 = 0.f;
	};

	Struct__SimpleRTCS_0CB m_cb__SimpleRTCS_0CB;
};

} // namespace halcyon
