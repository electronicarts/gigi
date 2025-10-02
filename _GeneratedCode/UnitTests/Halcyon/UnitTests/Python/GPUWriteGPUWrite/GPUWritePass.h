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

class GPUWritePipeline : public IShaderPipeline
{
	HCY_DECLARE_CLASS(GPUWritePipeline, IShaderPipeline);

public:
	HCY_PIPELINE_INFO(ComputeShader, 0, 5);

	virtual void loadPipeline(ShaderPipelines& pipelines) override;
	virtual void unloadPipeline(ShaderPipelines& pipelines) override;
};

class GPUWritePass : public IGPUWritePass
{
	HCY_DECLARE_CLASS(GPUWritePass, IGPUWritePass);

public:
	virtual void createGlobalResourcesHandles() override;
	virtual void deleteGlobalResourcesHandles() override;

	virtual void createGlobalResources(const IRenderDeviceGroup& deviceGroup, const RenderPassResourceCreateInfo& info) override;
	virtual void importGlobalResources(RenderGraphScope& scope, RenderGraph& renderGraph, const RenderPassResourceImportInfo& info) override;
	virtual void deleteGlobalResources(const IRenderDeviceGroup& deviceGroup) override;

	virtual void addGPUWritePass(RenderGraph& renderGraph, RenderGraphScope& scope, const ShaderPipelines& pipelines) override;

private:

	struct Struct_BufferStruct
	{
		int theInt = 0;
		float theFloat = 0.f;
	};
	RenderGraphTextureDesc m_textureTexture2DDesc;
	RenderResourceHandle m_textureTexture2D;
	RenderGraphTextureDesc m_textureTexture2DArrayDesc;
	RenderResourceHandle m_textureTexture2DArray;
	RenderGraphTextureDesc m_textureTexture3DDesc;
	RenderResourceHandle m_textureTexture3D;
	RenderGraphBufferDesc m_bufferFloatBufferDesc;
	RenderResourceHandle m_bufferFloatBuffer;
	RenderGraphBufferDesc m_bufferStructBufferDesc;
	RenderResourceHandle m_bufferStructBuffer;
};

} // namespace halcyon
