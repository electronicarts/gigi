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

class MultipleUVMeshPipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(MultipleUVMeshPipeline, IShaderPipeline);

public:

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class MultipleUVMeshPass : public IMultipleUVMeshPass
{
	HCY_DECLARE_CLASS(MultipleUVMeshPass, IMultipleUVMeshPass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addMultipleUVMeshPass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:

	struct Struct_VertexBuffer
	{
		glm::vec3 pos = {0.f, 0.f, 0.f};
		glm::vec3 normal = {0.f, 0.f, 0.f};
		glm::vec2 UV0 = {0.f, 0.f};
		glm::vec2 uv1 = {0.f, 0.f};
	};
};

} // namespace halcyon
