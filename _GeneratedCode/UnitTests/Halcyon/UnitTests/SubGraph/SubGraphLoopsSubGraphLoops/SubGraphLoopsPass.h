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

class SubGraphLoopsPipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(SubGraphLoopsPipeline, IShaderPipeline);

public:
	HCY_PIPELINE_INFO(FilterSub_Iteration_0_DoBlur, 1, 1);
	HCY_PIPELINE_INFO(FilterSub_Iteration_1_DoBlur, 1, 1);
	HCY_PIPELINE_INFO(FilterSub_Iteration_2_DoBlur, 1, 1);
	HCY_PIPELINE_INFO(FilterSub_Iteration_3_DoBlur, 1, 1);
	HCY_PIPELINE_INFO(FilterSub_Iteration_4_DoBlur, 1, 1);

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class SubGraphLoopsPass : public ISubGraphLoopsPass
{
	HCY_DECLARE_CLASS(SubGraphLoopsPass, ISubGraphLoopsPass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addSubGraphLoopsPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:

	struct Struct__FilterSub_Iteration_0_Blur_0CB
	{
		unsigned int FilterSub_Iteration_0_sRGB = true;
		int __loopIndexValue_0 = 0;
		glm::vec2 _padding0 = {0.f, 0.f};
	};

	struct Struct__FilterSub_Iteration_1_Blur_0CB
	{
		unsigned int FilterSub_Iteration_1_sRGB = true;
		int __loopIndexValue_1 = 1;
		glm::vec2 _padding0 = {0.f, 0.f};
	};

	struct Struct__FilterSub_Iteration_2_Blur_0CB
	{
		unsigned int FilterSub_Iteration_2_sRGB = true;
		int __loopIndexValue_2 = 2;
		glm::vec2 _padding0 = {0.f, 0.f};
	};

	struct Struct__FilterSub_Iteration_3_Blur_0CB
	{
		unsigned int FilterSub_Iteration_3_sRGB = true;
		int __loopIndexValue_3 = 3;
		glm::vec2 _padding0 = {0.f, 0.f};
	};

	struct Struct__FilterSub_Iteration_4_Blur_0CB
	{
		unsigned int FilterSub_Iteration_4_sRGB = true;
		int __loopIndexValue_4 = 4;
		glm::vec2 _padding0 = {0.f, 0.f};
	};

	int m_variable___loopIndexValue_0 = 0;
	int m_variable___loopIndexValue_1 = 1;
	int m_variable___loopIndexValue_2 = 2;
	int m_variable___loopIndexValue_3 = 3;
	int m_variable___loopIndexValue_4 = 4;

	Struct__FilterSub_Iteration_0_Blur_0CB m_cb__FilterSub_Iteration_0_Blur_0CB;

	Struct__FilterSub_Iteration_1_Blur_0CB m_cb__FilterSub_Iteration_1_Blur_0CB;

	Struct__FilterSub_Iteration_2_Blur_0CB m_cb__FilterSub_Iteration_2_Blur_0CB;

	Struct__FilterSub_Iteration_3_Blur_0CB m_cb__FilterSub_Iteration_3_Blur_0CB;

	Struct__FilterSub_Iteration_4_Blur_0CB m_cb__FilterSub_Iteration_4_Blur_0CB;
};

} // namespace halcyon
