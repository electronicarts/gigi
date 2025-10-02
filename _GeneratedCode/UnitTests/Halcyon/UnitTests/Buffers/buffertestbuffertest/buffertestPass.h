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

class buffertestPipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(buffertestPipeline, IShaderPipeline);

public:
	HCY_PIPELINE_INFO(BufferTest, 4, 4);

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class buffertestPass : public IbuffertestPass
{
	HCY_DECLARE_CLASS(buffertestPass, IbuffertestPass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addbuffertestPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:

	struct Struct_TestStruct
	{
		glm::vec4 TheFloat4 = {0.f, 0.f, 0.f, 0.f};
		glm::ivec4 TheInt4 = {0, 0, 0, 0};
		unsigned int TheBool = false;
	};

	struct Struct__BufferTest_0CB
	{
		float alpha1 = 1.f;
		float alpha2 = 0.f;
		float gain = 0.5f;
		float _padding0 = 0.f;
	};

	Struct__BufferTest_0CB m_cb__BufferTest_0CB;
};

} // namespace halcyon
