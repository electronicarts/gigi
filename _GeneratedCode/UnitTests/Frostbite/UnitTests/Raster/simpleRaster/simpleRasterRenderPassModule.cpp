// (c) Electronic Arts.  All Rights Reserved.

#include "simpleRasterRenderPassModule.h"

#include <Engine.Base/Misc/Vars.h>
#include <Engine.Math/Math.h>
#include <Engine.Render/Core/GetTextureResource.h>
#include <Engine.Render/Core2/Rc2ShaderProgramBinder.h>
#include <Engine.Render/FrameGraph/FrameGraph.h>
#include <Engine.Render/FrameGraph/FrameGraphBookmark.h>
#include <Engine.Render/FrameGraph/FrameGraphRenderPassData.h>
#include <Engine.Render/Raytrace/RaytracingUtil.h>
#include <Engine.World.Render/FrameGraphWorldRenderer.h>
#include <Engine.World.Render/simpleRaster/simpleRasterShaderProgramsDefs.h>
#include <Engine.World.Render/Raytrace/RaytraceRenderModule.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>

namespace fb
{
ShaderProgramPipelineCache simpleRasterRenderPassModule::s_pipelineCache;

template <typename T>
T Pow2GE(const T& A)
{
    float f = std::log2(float(A));
    f = std::ceilf(f);
    return (T)std::pow(2.0f, f);
}

void simpleRasterRenderPassModule::addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard)
{
	// TODO: Provide this imported buffer
	FrameGraphResource& bufferVertexBuffer = ;

	// Initialization - create resources internal to the technique
	FrameGraphMutableResource textureColor_Buffer;
	FrameGraphMutableResource textureDepth_Buffer;
	FrameGraphMutableResource bufferVBCopy;  // This is here for the benefit of simpleRasterInSubgraph. Need a reference to a struct type in a subgraph.
	{
		struct PassData
		{
		};

		frameGraph.addRc2CallbackPass<PassData>("simpleRaster.Initialization", [&](RenderPassBuilderBase& builder, PassData& data)
		{

			// Texture: Color_Buffer
			{
				TextureType type = TextureType_2d;
				u32 bindFlags = RenderBindFlag_ShaderResource | RenderBindFlag_RenderTarget;
				RenderFormat format = RenderFormat_R8G8B8A8_SRGB;
				uint width = ((1 + 0) * 512) / 1 + 0;
				uint height = ((1 + 0) * 512) / 1 + 0;
				uint depth = ((1 + 0) * 1) / 1 + 0;

				FrameGraphTextureDesc desc;
				desc.name = "simpleRaster.Color_Buffer";
				desc.usage = RenderResourceUsage_Static;
				desc.initialState = FrameGraphTextureDesc::InitialState_Undefined;
				desc.type = type;
				desc.bindFlags = bindFlags;
				desc.format = format;
				desc.width = width;
				desc.height = height;
				desc.depth = depth;
				textureColor_Buffer = builder.createTexture(desc, FrameGraphResourceUsage_RenderTarget);
			}

			// Texture: Depth_Buffer
			{
				TextureType type = TextureType_2d;
				u32 bindFlags = RenderBindFlag_ShaderResource | RenderBindFlag_DepthStencil;
				RenderFormat format = RenderFormat_D32_FLOAT;
				FrameGraphTextureDesc sizeSrcDesc = builder.getTextureDesc(textureColor_Buffer);
				uint width = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
				uint height = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
				uint depth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;

				FrameGraphTextureDesc desc;
				desc.name = "simpleRaster.Depth_Buffer";
				desc.usage = RenderResourceUsage_Static;
				desc.initialState = FrameGraphTextureDesc::InitialState_Undefined;
				desc.type = type;
				desc.bindFlags = bindFlags;
				desc.format = format;
				desc.width = width;
				desc.height = height;
				desc.depth = depth;
				textureDepth_Buffer = builder.createTexture(desc, FrameGraphResourceUsage_DepthWrite);
			}

			// Buffer: VBCopy
			// This is here for the benefit of simpleRasterInSubgraph. Need a reference to a struct type in a subgraph.
			{
				uint32_t bindFlags = RenderBindFlag_ShaderResource | RenderBindFlag_VertexBuffer;
				uint32_t memoryFlags = RenderResourceMemoryFlags_ZeroInitialize;
				RenderFormat format = RenderFormat::RenderFormat_Unknown;
				uint32_t structByteStride = 24;
				memoryFlags |= RenderResourceMemoryFlags_StructuredBuffer;
				FrameGraphBufferDesc sizeSrcDesc = builder.getBufferDesc(bufferVertexBuffer);
				uint32_t size = ((sizeSrcDesc.size + 0) * 1) / 1 + 0;

				FrameGraphBufferDesc desc;
				desc.name = "simpleRaster.VBCopy";
				desc.usage = RenderResourceUsage_Static;
				desc.initialState = FrameGraphBufferDesc::InitialState_Undefined;
				desc.bindFlags = bindFlags;
				desc.memoryFlags = memoryFlags;
				desc.format = format;
				desc.structByteStride = structByteStride;
				desc.size = size;
				bufferVBCopy = builder.createBuffer(desc, FrameGraphResourceUsage_CopyDestination);
			}
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
		});
	}

	// Copy Resource: CopyVBs
	// This is here for the benefit of simpleRasterInSubgraph. Need a reference to a struct type in a subgraph.
	{
		struct PassData
		{
			FrameGraphResource source;
			FrameGraphMutableResource dest;
		};

		frameGraph.addRc2CallbackPass<PassData>("simpleRaster.CopyVBs", [&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.source = builder.read(bufferVertexBuffer);
			data.dest = builder.writeAndUpdate(bufferVBCopy, FrameGraphResourceUsage_CopyDestination);
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			IResource* srcResource = resources.getRenderResource(data.source);
			IResource* destResource = resources.getRenderResource(data.dest);
			commandRecorder.transitionResource(srcResource, ResourceBindFlags_CopySource);
			commandRecorder.transitionResource(destResource, ResourceBindFlags_CopyDest);
			commandRecorder.copyResource(destResource, srcResource);
		});
	}

	// Shader Constants: _VertexShaderCB
	{
		m_cb__VertexShaderCB.ViewProjMtx = m_varViewProjMtx;
	}

	// Draw Call: Rasterize
	{
		struct PassData
		{
			FrameGraphMutableResource textureColor_Buffer;
			FrameGraphMutableResource textureDepth_Buffer;
			size_t vertexCountPerInstance;
			size_t instanceCount;
			FrameGraphResource vertexBuffer;
			size_t vbStrideBytes;
			size_t vbSizeBytes;
			Struct__VertexShaderCB cb_VertexShaderCB;
		};

		frameGraph.addRc2CallbackPass<PassData>("simpleRaster.Rasterize",
		[&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureColor_Buffer = builder.writeAndUpdate(textureColor_Buffer, FrameGraphResourceUsage::FrameGraphResourceUsage_RenderTarget);
			data.textureDepth_Buffer = builder.writeAndUpdate(textureDepth_Buffer, FrameGraphResourceUsage::FrameGraphResourceUsage_DepthWrite);
			data.cb_VertexShaderCB = m_cb__VertexShaderCB;

			FrameGraphBufferDesc vbDesc = builder.getBufferDesc(bufferVBCopy);
			data.vertexBuffer = builder.read(bufferVBCopy);
			data.vbStrideBytes = vbDesc.structByteStride > 0 ? vbDesc.structByteStride : getFormatBytesPerElement(convertRenderFormat(vbDesc.format));
			data.vbSizeBytes = vbDesc.size;

			uint32_t vbCount = vbDesc.size / (uint32_t)data.vbStrideBytes;
			data.vertexCountPerInstance = vbCount;

			data.instanceCount = 1;
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			commandRecorder.transitionResource(resources.getBuffer(bufferVBCopy), ResourceBindFlags_VertexBuffer);

			RenderPassDesc renderPassDesc;
			renderPassDesc.name = "simpleRaster.Rasterize";
			renderPassDesc.renderTargetCount = 1;
			renderPassDesc.renderTargets[0] = resources.getRtvInfo(data.textureColor_Buffer).view;
			renderPassDesc.renderTargetUsage[0] = AttachmentUsage_LoadAction_Clear | AttachmentUsage_StoreAction_Write;
			renderPassDesc.renderTargetClearValue[0] = { 0.200000f, 0.200000f, 0.200000f, 1.000000f };
			renderPassDesc.depthTarget = resources.getDsvInfo(data.textureDepth_Buffer).view;
			renderPassDesc.depthTargetUsage = AttachmentUsage_LoadAction_Clear | AttachmentUsage_StoreAction_Write;
			renderPassDesc.depthClearValue = 0.000000f;
			if (isFormatDepthStencilCompatible(resources.getResourceDesc(data.textureDepth_Buffer).format))
			{
				renderPassDesc.stencilTargetUsage = AttachmentUsage_LoadAction_Load | AttachmentUsage_StoreAction_Write;
			}
			else
			{
				renderPassDesc.stencilTargetUsage = AttachmentUsage_Invalid;
			}
			commandRecorder.beginRenderPass(renderPassDesc);

			RasterizerStateDesc rasterizerStateDesc;
			rasterizerStateDesc.cullMode = CullMode_None;
			rasterizerStateDesc.frontCounterClockwise = false;
			rasterizerStateDesc.conservativeRaster = false;
			commandRecorder.setPipelineRasterizerState(rasterizerStateDesc);

			DepthStencilStateDesc depthStencilStateDesc;
			depthStencilStateDesc.depthTestEnable = true;
			depthStencilStateDesc.depthWriteEnable = true;
			depthStencilStateDesc.stencilTestEnable = isFormatDepthStencilCompatible(resources.getResourceDesc(data.textureDepth_Buffer).format);
			depthStencilStateDesc.depthFunc = RenderCompareFunction_Greater;
			depthStencilStateDesc.stencilReadMask = 255;
			depthStencilStateDesc.stencilWriteMask = 255;
			depthStencilStateDesc.frontFaceOp.stencilFailOp = StencilOp_Keep;
			depthStencilStateDesc.frontFaceOp.stencilDepthFailOp = StencilOp_Keep;
			depthStencilStateDesc.frontFaceOp.stencilPassOp = StencilOp_Keep;
			depthStencilStateDesc.frontFaceOp.stencilFunc = RenderCompareFunction_Always;
			depthStencilStateDesc.backFaceOp.stencilFailOp = StencilOp_Keep;
			depthStencilStateDesc.backFaceOp.stencilDepthFailOp = StencilOp_Keep;
			depthStencilStateDesc.backFaceOp.stencilPassOp = StencilOp_Keep;
			depthStencilStateDesc.backFaceOp.stencilFunc = RenderCompareFunction_Always;
			commandRecorder.setPipelineDepthStencilState(depthStencilStateDesc);

			BlendStateDesc blendStateDesc;
			blendStateDesc.targetBlend.blendEnable = true;
			blendStateDesc.targetBlend.renderTargetWriteMask = ColorWriteEnable_Red | ColorWriteEnable_Green | ColorWriteEnable_Blue;
			blendStateDesc.targetBlend.srcBlend = BlendFactor_SrcAlpha;
			blendStateDesc.targetBlend.destBlend = BlendFactor_InvSrcAlpha;
			blendStateDesc.targetBlend.blendOp = BlendOp_Add;
			blendStateDesc.targetBlend.srcBlendAlpha = BlendFactor_One;
			blendStateDesc.targetBlend.destBlendAlpha = BlendFactor_Zero;
			blendStateDesc.targetBlend.blendOpAlpha = BlendOp_Add;
			commandRecorder.setPipelineBlendState(blendStateDesc);

			eastl::vector<InputElementDesc> inputElementDescs;
			inputElementDescs.push(InputElementDesc("POSITION", 0, RenderFormat::RenderFormat_R32G32B32_FLOAT, 0, 0, InputClassification_PerVertex, 0));
			inputElementDescs.push(InputElementDesc("NORMAL", 0, RenderFormat::RenderFormat_R32G32B32_FLOAT, 0, 12, InputClassification_PerVertex, 0));

			InputLayoutDesc inputLayoutDesc = InputLayoutDesc::build(inputElementDescs);

			ResourceFormat colorTargetFormats[] =
			{
				resources.getResourceDesc(data.textureColor_Buffer).format,
			};

			GraphicsPipelineCompileInfo graphicsPipelineCompileInfo;
			graphicsPipelineCompileInfo.shaderProgram = ShaderProgram_simpleRaster_Rasterize_VSPS;
			graphicsPipelineCompileInfo.primitiveTopologyType = PrimitiveTopologyType_Triangle;
			graphicsPipelineCompileInfo.numRenderTargets = 1;
			graphicsPipelineCompileInfo.targetFormats = colorTargetFormats;
			graphicsPipelineCompileInfo.depthFormat = resources.getResourceDesc(data.textureDepth_Buffer).format;
			graphicsPipelineCompileInfo.inputLayout = &inputLayoutDesc;
			const GraphicsPipelineInfo& graphicsPipelineInfo = s_pipelineCache.getGraphicsPipeline(graphicsPipelineCompileInfo);
			commandRecorder.setPipeline(graphicsPipelineInfo.pipeline);

			commandRecorder.setStencilReference(0);

			// Vertex Buffers
			ResourceView vertexBufferViews[1];
			vertexBufferViews[0] = resources.getBuffer(bufferVBCopy)->getAsVbv(ViewDesc::buildAsVertexBufferView(0, data.vbStrideBytes, data.vbSizeBytes));
			commandRecorder.setVertexBuffers(1, vertexBufferViews);

			Rc2ShaderProgramBinder rc2ShaderProgramBinder(commandRecorder, graphicsPipelineInfo.shaderProgramInfo);

			rc2ShaderProgramBinder.vertexStage()
				.setConstantBuffer(sizeof(data.cb_VertexShaderCB), &data.cb_VertexShaderCB)
			;

			rc2ShaderProgramBinder.bind();

			// Draw
			commandRecorder.drawInstanced(PrimitiveTopology_TriangleList, data.vertexCountPerInstance, data.instanceCount, 0, 0);

			commandRecorder.endRenderPass();
		});
	}

	// bufferVertexBuffer  TODO: Put this buffer into the black board (Optional)
	// bufferColor_Buffer TODO: Put this texture into the black board (Optional)
	// bufferDepth_Buffer TODO: Put this texture into the black board (Optional)
}

void simpleRasterRenderPassModule::onInit(const InitParams& params)
{
	m_arena = params.memoryArena;

	// Register with framegraph
	renderPassBookmark::registerPass(RenderPassBookmark_AfterPost,
		[this](FrameGraph& frameGraph, FrameGraphBlackboard& blackboard) { addPass(frameGraph, blackboard); });
}

simpleRasterRenderPassModule::~simpleRasterRenderPassModule()
{
	s_pipelineCache.clear();
}

}
