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

class StructuredBufferPipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(StructuredBufferPipeline, IShaderPipeline);

public:
	HCY_PIPELINE_INFO(csmain, 0, 1);

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class StructuredBufferPass : public IStructuredBufferPass
{
	HCY_DECLARE_CLASS(StructuredBufferPass, IStructuredBufferPass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addStructuredBufferPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:

	struct Struct_TheStructure
	{
		int TheInt = 0;
		float TheFloat = 0.f;
		int TheEnum = (int)StructuredBufferSettings::Trinary::True;
		unsigned int TheBool = true;
		unsigned int TheUINT = 0;
		glm::vec2 TheFloat2 = {0.f, 0.f};
	};

	struct Struct__csmain_0CB
	{
		float frameDeltaTime = 0.f;
		int frameIndex = 0;
		glm::vec2 _padding0 = {0.f, 0.f};
	};

	Struct__csmain_0CB m_cb__csmain_0CB;
};

} // namespace halcyon
