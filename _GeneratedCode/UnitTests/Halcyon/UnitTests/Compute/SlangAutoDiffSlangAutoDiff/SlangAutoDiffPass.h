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

class SlangAutoDiffPipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(SlangAutoDiffPipeline, IShaderPipeline);

public:
	HCY_PIPELINE_INFO(Initialize, 0, 1);
	HCY_PIPELINE_INFO(GradientDescend, 0, 1);
	HCY_PIPELINE_INFO(Render, 1, 1);

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class SlangAutoDiffPass : public ISlangAutoDiffPass
{
	HCY_DECLARE_CLASS(SlangAutoDiffPass, ISlangAutoDiffPass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addSlangAutoDiffPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:

	struct Struct__Init_0CB
	{
		int FrameIndex = 0;
		glm::vec3 _padding0 = {0.f, 0.f, 0.f};
		glm::vec4 MouseState = {0.f, 0.f, 0.f, 0.f};
		glm::vec4 MouseStateLastFrame = {0.f, 0.f, 0.f, 0.f};
		int NumGaussians = 10;
		glm::vec3 iResolution = {0.f, 0.f, 0.f};
		unsigned int initialized = false;
		glm::vec3 _padding1 = {0.f, 0.f, 0.f};
	};

	struct Struct__Render_0CB
	{
		int NumGaussians = 10;
		unsigned int QuantizeDisplay = false;
		glm::vec2 _padding0 = {0.f, 0.f};
	};

	struct Struct__Descend_0CB
	{
		float LearningRate = 0.100000001f;
		float MaximumStepSize = 0.00999999978f;
		int NumGaussians = 10;
		unsigned int UseBackwardAD = true;
	};
	RenderGraphBufferDesc m_bufferDataDesc;
	RenderResourceHandle m_bufferData;  // first 2 floats is position, nect 2 floats are the gradient. Then 5 floats per Gaussian.

	Struct__Init_0CB m_cb__Init_0CB;

	Struct__Render_0CB m_cb__Render_0CB;

	Struct__Descend_0CB m_cb__Descend_0CB;
};

} // namespace halcyon
