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

class boxblurPipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(boxblurPipeline, IShaderPipeline);

public:
	HCY_PIPELINE_INFO(BlurH, 1, 1);
	HCY_PIPELINE_INFO(BlurV, 1, 1);

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class boxblurPass : public IboxblurPass
{
	HCY_DECLARE_CLASS(boxblurPass, IboxblurPass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addboxblurPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:

	struct Struct__BoxBlur_0CB
	{
		int radius = 2;
		unsigned int sRGB = true;
		glm::vec2 _padding0 = {0.f, 0.f};
	};

	struct Struct__BoxBlur_1CB
	{
		int radius = 2;
		unsigned int sRGB = true;
		glm::vec2 _padding0 = {0.f, 0.f};
	};

	Struct__BoxBlur_0CB m_cb__BoxBlur_0CB;

	Struct__BoxBlur_1CB m_cb__BoxBlur_1CB;
};

} // namespace halcyon
