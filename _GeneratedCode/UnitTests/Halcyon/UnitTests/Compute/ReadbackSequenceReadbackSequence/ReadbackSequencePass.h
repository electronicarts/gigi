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

class ReadbackSequencePipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(ReadbackSequencePipeline, IShaderPipeline);

public:
	HCY_PIPELINE_INFO(Node_1, 0, 1);

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class ReadbackSequencePass : public IReadbackSequencePass
{
	HCY_DECLARE_CLASS(ReadbackSequencePass, IReadbackSequencePass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addReadbackSequencePass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:

	struct Struct__ReadbackSequenceCS_0CB
	{
		int frameIndex = 0;
		glm::vec3 _padding0 = {0.f, 0.f, 0.f};
	};

	Struct__ReadbackSequenceCS_0CB m_cb__ReadbackSequenceCS_0CB;
};

} // namespace halcyon
