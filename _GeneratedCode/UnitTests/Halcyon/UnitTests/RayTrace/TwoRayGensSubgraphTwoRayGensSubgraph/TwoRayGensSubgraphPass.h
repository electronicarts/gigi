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

class TwoRayGensSubgraphPipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(TwoRayGensSubgraphPipeline, IShaderPipeline);

public:
	HCY_PIPELINE_INFO(A_TwoRayGensMiss1, 0, 0);
	HCY_PIPELINE_INFO(A_TwoRayGensClosestHit1, 0, 0);
	HCY_PIPELINE_INFO(B_TwoRayGensMiss2A, 0, 0);
	HCY_PIPELINE_INFO(B_TwoRayGensMiss2B, 0, 0);
	HCY_PIPELINE_INFO(B_TwoRayGensClosestHit2, 0, 0);
	HCY_PIPELINE_INFO(A_DoRT1, 1, 1);
	HCY_PIPELINE_INFO(B_DoRT2, 2, 1);

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class TwoRayGensSubgraphPass : public ITwoRayGensSubgraphPass
{
	HCY_DECLARE_CLASS(TwoRayGensSubgraphPass, ITwoRayGensSubgraphPass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addTwoRayGensSubgraphPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:

	struct Struct_VertexBuffer
	{
		glm::vec3 Color = {0.f, 0.f, 0.f};
		glm::vec3 Position = {0.f, 0.f, 0.f};
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

	struct Struct_A_VertexBuffer
	{
		glm::vec3 Color = {0.f, 0.f, 0.f};
		glm::vec3 Position = {0.f, 0.f, 0.f};
	};

	struct Struct_A_VertexBufferSimple
	{
		glm::vec3 Position = {0.f, 0.f, 0.f};
	};

	struct Struct_A_VertexBufferFull
	{
		glm::vec3 Position = {0.f, 0.f, 0.f};
		glm::vec3 Color = {0.f, 0.f, 0.f};
		glm::vec3 Normal = {0.f, 0.f, 0.f};
		glm::vec4 Tangent = {0.f, 0.f, 0.f, 0.f};
		glm::vec2 UV = {0.f, 0.f};
		int MaterialID = 0;
	};

	struct Struct_B_VertexBuffer
	{
		glm::vec3 Color = {0.f, 0.f, 0.f};
		glm::vec3 Position = {0.f, 0.f, 0.f};
	};

	struct Struct_B_VertexBufferSimple
	{
		glm::vec3 Position = {0.f, 0.f, 0.f};
	};

	struct Struct_B_VertexBufferFull
	{
		glm::vec3 Position = {0.f, 0.f, 0.f};
		glm::vec3 Color = {0.f, 0.f, 0.f};
		glm::vec3 Normal = {0.f, 0.f, 0.f};
		glm::vec4 Tangent = {0.f, 0.f, 0.f, 0.f};
		glm::vec2 UV = {0.f, 0.f};
		int MaterialID = 0;
	};

	struct Struct__A_TwoRayGens1CB
	{
		glm::vec3 cameraPos = {0.f, 0.f, 0.f};
		float _padding0 = 0.f;
		glm::mat4 clipToWorld = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
		float depthNearPlane = 0.f;
		glm::vec3 _padding1 = {0.f, 0.f, 0.f};
	};

	struct Struct__B_TwoRayGens2CB
	{
		glm::vec3 cameraPos = {0.f, 0.f, 0.f};
		float _padding0 = 0.f;
		glm::mat4 clipToWorld = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
		float depthNearPlane = 0.f;
		glm::vec3 _padding1 = {0.f, 0.f, 0.f};
	};

	static unsigned int m_variable__missindex_A_TwoRayGensMiss1;
	static unsigned int m_variable__chindex_A_TwoRayGensClosestHit1;
	static unsigned int m_variable__missindex_B_TwoRayGensMiss2A;
	static unsigned int m_variable__missindex_B_TwoRayGensMiss2B;
	static unsigned int m_variable__chindex_B_TwoRayGensClosestHit2;
	static unsigned int m_variable__rt_hit_group_count;

	Struct__A_TwoRayGens1CB m_cb__A_TwoRayGens1CB;

	Struct__B_TwoRayGens2CB m_cb__B_TwoRayGens2CB;
};

} // namespace halcyon
