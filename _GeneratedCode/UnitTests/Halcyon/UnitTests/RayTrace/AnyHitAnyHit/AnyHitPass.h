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

class AnyHitPipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(AnyHitPipeline, IShaderPipeline);

public:
	HCY_PIPELINE_INFO(Miss, 0, 0);
	HCY_PIPELINE_INFO(ClosestHit, 0, 0);
	HCY_PIPELINE_INFO(DoRT, 2, 1);

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class AnyHitPass : public IAnyHitPass
{
	HCY_DECLARE_CLASS(AnyHitPass, IAnyHitPass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addAnyHitPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:

	struct Struct_VertexBuffer
	{
		glm::vec3 position = {0.f, 0.f, 0.f};
		glm::vec3 normal = {0.f, 0.f, 0.f};
		glm::vec2 uv = {0.f, 0.f};
		unsigned int materialId = 0;
	};

	struct Struct__RayGenCB
	{
		glm::vec3 CameraPos = {0.f, 0.f, 0.f};
		float _padding0 = 0.f;
		glm::mat4 InvViewProjMtx = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
		float depthNearPlane = 0.f;
		glm::vec3 _padding1 = {0.f, 0.f, 0.f};
	};

	static unsigned int m_variable__missindex_Miss;
	static unsigned int m_variable__chindex_ClosestHit;
	static unsigned int m_variable__rt_hit_group_count;

	Struct__RayGenCB m_cb__RayGenCB;
};

} // namespace halcyon
