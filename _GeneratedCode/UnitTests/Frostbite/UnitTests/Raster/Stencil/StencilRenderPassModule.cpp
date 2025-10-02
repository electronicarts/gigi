// (c) Electronic Arts.  All Rights Reserved.

#include "StencilRenderPassModule.h"

#include <Engine.Base/Misc/Vars.h>
#include <Engine.Math/Math.h>
#include <Engine.Render/Core/GetTextureResource.h>
#include <Engine.Render/Core2/Rc2ShaderProgramBinder.h>
#include <Engine.Render/FrameGraph/FrameGraph.h>
#include <Engine.Render/FrameGraph/FrameGraphBookmark.h>
#include <Engine.Render/FrameGraph/FrameGraphRenderPassData.h>
#include <Engine.Render/Raytrace/RaytracingUtil.h>
#include <Engine.World.Render/FrameGraphWorldRenderer.h>
#include <Engine.World.Render/Stencil/StencilShaderProgramsDefs.h>
#include <Engine.World.Render/Raytrace/RaytraceRenderModule.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>

namespace fb
{
ShaderProgramPipelineCache StencilRenderPassModule::s_pipelineCache;

template <typename T>
T Pow2GE(const T& A)
{
    float f = std::log2(float(A));
    f = std::ceilf(f);
    return (T)std::pow(2.0f, f);
}

void StencilRenderPassModule::addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard)
{
	// TODO: Provide this imported buffer
	FrameGraphResource& bufferVertex_Buffer = ;
	// TODO: Describe the vertex buffer format
	eastl::vector<InputElementDesc> bufferVertex_Buffer_vbDesc;

	// Initialization - create resources internal to the technique
	FrameGraphMutableResource textureColor;
	FrameGraphMutableResource textureDepth_Stencil;
	FrameGraphResource texture_loadedTexture_0;
	{
		struct PassData
		{
		};

		frameGraph.addRc2CallbackPass<PassData>("Stencil.Initialization", [&](RenderPassBuilderBase& builder, PassData& data)
		{

			// Texture: Color
			{
				TextureType type = TextureType_2d;
				u32 bindFlags = RenderBindFlag_ShaderResource | RenderBindFlag_RenderTarget;
				RenderFormat format = RenderFormat_R8G8B8A8_SRGB;
				uint width = ((1 + 0) * 128) / 1 + 0;
				uint height = ((1 + 0) * 128) / 1 + 0;
				uint depth = ((1 + 0) * 1) / 1 + 0;

				FrameGraphTextureDesc desc;
				desc.name = "Stencil.Color";
				desc.usage = RenderResourceUsage_Static;
				desc.initialState = FrameGraphTextureDesc::InitialState_Undefined;
				desc.type = type;
				desc.bindFlags = bindFlags;
				desc.format = format;
				desc.width = width;
				desc.height = height;
				desc.depth = depth;
				textureColor = builder.createTexture(desc, FrameGraphResourceUsage_RenderTarget);
			}

			// Texture: Depth_Stencil
			{
				TextureType type = TextureType_2d;
				u32 bindFlags = RenderBindFlag_ShaderResource | RenderBindFlag_DepthStencil;
				RenderFormat format = RenderFormat_D24_UNORM_S8_UINT;
				FrameGraphTextureDesc sizeSrcDesc = builder.getTextureDesc(textureColor);
				uint width = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
				uint height = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
				uint depth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;

				// (Re)create the texture as needed
				if (!m_textureDepth_Stencil || m_textureDepth_Stencil_format != format || m_textureDepth_Stencil_width != width  || m_textureDepth_Stencil_height != height || m_textureDepth_Stencil_depth != depth)
				{
					m_textureDepth_Stencil_format = format;
					m_textureDepth_Stencil_width = width;
					m_textureDepth_Stencil_height = height;
					m_textureDepth_Stencil_depth = depth;

					TextureCreateDesc desc;
					desc.name = "Stencil.Depth_Stencil";
					desc.type = type;
					desc.bindFlags = bindFlags;
					desc.format = format;
					desc.width = width;
					desc.height = height;
					desc.depth = depth;
					m_textureDepth_Stencil = g_renderer->createTexture(*m_arena, desc);
				}

				textureDepth_Stencil = frameGraph.importResource(m_textureDepth_Stencil.get());
			}

			texture_loadedTexture_0 = frameGraph.importResource(GetTextureResource::getTextureResource("Stencil/cabinsmall"));
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
		});
	}

	// Shader Constants: _Draw1VSCB
	{
		m_cb__Draw1VSCB.ViewProjMtx = m_varViewProjMtx;
	}

	// Draw Call: Draw_1
	{
		struct PassData
		{
			FrameGraphMutableResource textureColor;
			FrameGraphMutableResource textureDepth_Stencil;
			size_t vertexCountPerInstance;
			size_t instanceCount;
			FrameGraphResource vertexBuffer;
			size_t vbStrideBytes;
			size_t vbSizeBytes;
			Struct__Draw1VSCB cb_Draw1VSCB;
		};

		frameGraph.addRc2CallbackPass<PassData>("Stencil.Draw_1",
		[&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureColor = builder.writeAndUpdate(textureColor, FrameGraphResourceUsage::FrameGraphResourceUsage_RenderTarget);
			data.textureDepth_Stencil = builder.writeAndUpdate(textureDepth_Stencil, FrameGraphResourceUsage::FrameGraphResourceUsage_DepthWrite);
			data.cb_Draw1VSCB = m_cb__Draw1VSCB;

			FrameGraphBufferDesc vbDesc = builder.getBufferDesc(bufferVertex_Buffer);
			data.vertexBuffer = builder.read(bufferVertex_Buffer);
			data.vbStrideBytes = vbDesc.structByteStride > 0 ? vbDesc.structByteStride : getFormatBytesPerElement(convertRenderFormat(vbDesc.format));
			data.vbSizeBytes = vbDesc.size;

			uint32_t vbCount = vbDesc.size / (uint32_t)data.vbStrideBytes;
			data.vertexCountPerInstance = vbCount;

			data.instanceCount = 1;
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			commandRecorder.transitionResource(resources.getBuffer(bufferVertex_Buffer), ResourceBindFlags_VertexBuffer);

			RenderPassDesc renderPassDesc;
			renderPassDesc.name = "Stencil.Draw_1";
			renderPassDesc.renderTargetCount = 1;
			renderPassDesc.renderTargets[0] = resources.getRtvInfo(data.textureColor).view;
			renderPassDesc.renderTargetUsage[0] = AttachmentUsage_LoadAction_Clear | AttachmentUsage_StoreAction_Write;
			renderPassDesc.renderTargetClearValue[0] = { 0.000000f, 0.000000f, 0.000000f, 1.000000f };
			renderPassDesc.depthTarget = resources.getDsvInfo(data.textureDepth_Stencil).view;
			renderPassDesc.depthTargetUsage = AttachmentUsage_LoadAction_Clear | AttachmentUsage_StoreAction_Write;
			renderPassDesc.depthClearValue = 0.000000f;
			if (isFormatDepthStencilCompatible(resources.getResourceDesc(data.textureDepth_Stencil).format))
			{
				renderPassDesc.stencilTargetUsage = AttachmentUsage_LoadAction_Clear | AttachmentUsage_StoreAction_Write;
				renderPassDesc.stencilClearValue = 0;
			}
			else
			{
				renderPassDesc.stencilTargetUsage = AttachmentUsage_Invalid;
			}
			commandRecorder.beginRenderPass(renderPassDesc);

			RasterizerStateDesc rasterizerStateDesc;
			rasterizerStateDesc.cullMode = CullMode_None;
			rasterizerStateDesc.frontCounterClockwise = true;
			rasterizerStateDesc.conservativeRaster = false;
			commandRecorder.setPipelineRasterizerState(rasterizerStateDesc);

			DepthStencilStateDesc depthStencilStateDesc;
			depthStencilStateDesc.depthTestEnable = true;
			depthStencilStateDesc.depthWriteEnable = true;
			depthStencilStateDesc.stencilTestEnable = isFormatDepthStencilCompatible(resources.getResourceDesc(data.textureDepth_Stencil).format);
			depthStencilStateDesc.depthFunc = RenderCompareFunction_Greater;
			depthStencilStateDesc.stencilReadMask = 255;
			depthStencilStateDesc.stencilWriteMask = 255;
			depthStencilStateDesc.frontFaceOp.stencilFailOp = StencilOp_Keep;
			depthStencilStateDesc.frontFaceOp.stencilDepthFailOp = StencilOp_Keep;
			depthStencilStateDesc.frontFaceOp.stencilPassOp = StencilOp_Replace;
			depthStencilStateDesc.frontFaceOp.stencilFunc = RenderCompareFunction_Always;
			depthStencilStateDesc.backFaceOp.stencilFailOp = StencilOp_Keep;
			depthStencilStateDesc.backFaceOp.stencilDepthFailOp = StencilOp_Keep;
			depthStencilStateDesc.backFaceOp.stencilPassOp = StencilOp_Replace;
			depthStencilStateDesc.backFaceOp.stencilFunc = RenderCompareFunction_Always;
			commandRecorder.setPipelineDepthStencilState(depthStencilStateDesc);

			BlendStateDesc blendStateDesc;
			blendStateDesc.targetBlend.blendEnable = false;
			blendStateDesc.targetBlend.renderTargetWriteMask = ColorWriteEnable_All;
			commandRecorder.setPipelineBlendState(blendStateDesc);

			eastl::vector<InputElementDesc> inputElementDescs;
			inputElementDescs.insert(inputElementDescs.end(), bufferVertex_Buffer_vbDesc.begin(), bufferVertex_Buffer_vbDesc.end());

			InputLayoutDesc inputLayoutDesc = InputLayoutDesc::build(inputElementDescs);

			ResourceFormat colorTargetFormats[] =
			{
				resources.getResourceDesc(data.textureColor).format,
			};

			GraphicsPipelineCompileInfo graphicsPipelineCompileInfo;
			graphicsPipelineCompileInfo.shaderProgram = ShaderProgram_Stencil_Draw_1_VSPS;
			graphicsPipelineCompileInfo.primitiveTopologyType = PrimitiveTopologyType_Triangle;
			graphicsPipelineCompileInfo.numRenderTargets = 1;
			graphicsPipelineCompileInfo.targetFormats = colorTargetFormats;
			graphicsPipelineCompileInfo.depthFormat = resources.getResourceDesc(data.textureDepth_Stencil).format;
			graphicsPipelineCompileInfo.inputLayout = &inputLayoutDesc;
			const GraphicsPipelineInfo& graphicsPipelineInfo = s_pipelineCache.getGraphicsPipeline(graphicsPipelineCompileInfo);
			commandRecorder.setPipeline(graphicsPipelineInfo.pipeline);

			commandRecorder.setStencilReference(128);

			// Vertex Buffers
			ResourceView vertexBufferViews[1];
			vertexBufferViews[0] = resources.getBuffer(bufferVertex_Buffer)->getAsVbv(ViewDesc::buildAsVertexBufferView(0, data.vbStrideBytes, data.vbSizeBytes));
			commandRecorder.setVertexBuffers(1, vertexBufferViews);

			Rc2ShaderProgramBinder rc2ShaderProgramBinder(commandRecorder, graphicsPipelineInfo.shaderProgramInfo);

			rc2ShaderProgramBinder.vertexStage()
				.setConstantBuffer(sizeof(data.cb_Draw1VSCB), &data.cb_Draw1VSCB)
			;

			rc2ShaderProgramBinder.bind();

			// Draw
			commandRecorder.drawInstanced(PrimitiveTopology_TriangleList, data.vertexCountPerInstance, data.instanceCount, 0, 0);

			commandRecorder.endRenderPass();
		});
	}

	// Draw Call: Draw_2
	{
		struct PassData
		{
			FrameGraphMutableResource textureColor;
			FrameGraphMutableResource textureDepth_Stencil;
			size_t vertexCountPerInstance;
			size_t instanceCount;
			FrameGraphResource texture_loadedTexture_0;
		};

		frameGraph.addRc2CallbackPass<PassData>("Stencil.Draw_2",
		[&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureColor = builder.writeAndUpdate(textureColor, FrameGraphResourceUsage::FrameGraphResourceUsage_RenderTarget);
			data.textureDepth_Stencil = builder.writeAndUpdate(textureDepth_Stencil, FrameGraphResourceUsage::FrameGraphResourceUsage_DepthWrite);
			data.texture_loadedTexture_0 = builder.read(texture_loadedTexture_0);

			data.vertexCountPerInstance = 4;

			data.instanceCount = 1;
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			RenderPassDesc renderPassDesc;
			renderPassDesc.name = "Stencil.Draw_2";
			renderPassDesc.renderTargetCount = 1;
			renderPassDesc.renderTargets[0] = resources.getRtvInfo(data.textureColor).view;
			renderPassDesc.renderTargetUsage[0] = AttachmentUsage_LoadAction_Load | AttachmentUsage_StoreAction_Write;
			renderPassDesc.depthTarget = resources.getDsvInfo(data.textureDepth_Stencil).view;
			renderPassDesc.depthTargetUsage = AttachmentUsage_LoadAction_Load | AttachmentUsage_StoreAction_Write;
			if (isFormatDepthStencilCompatible(resources.getResourceDesc(data.textureDepth_Stencil).format))
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
			rasterizerStateDesc.frontCounterClockwise = true;
			rasterizerStateDesc.conservativeRaster = false;
			commandRecorder.setPipelineRasterizerState(rasterizerStateDesc);

			DepthStencilStateDesc depthStencilStateDesc;
			depthStencilStateDesc.depthTestEnable = true;
			depthStencilStateDesc.depthWriteEnable = false;
			depthStencilStateDesc.stencilTestEnable = isFormatDepthStencilCompatible(resources.getResourceDesc(data.textureDepth_Stencil).format);
			depthStencilStateDesc.depthFunc = RenderCompareFunction_Always;
			depthStencilStateDesc.stencilReadMask = 255;
			depthStencilStateDesc.stencilWriteMask = 255;
			depthStencilStateDesc.frontFaceOp.stencilFailOp = StencilOp_Keep;
			depthStencilStateDesc.frontFaceOp.stencilDepthFailOp = StencilOp_Keep;
			depthStencilStateDesc.frontFaceOp.stencilPassOp = StencilOp_Keep;
			depthStencilStateDesc.frontFaceOp.stencilFunc = RenderCompareFunction_Equal;
			depthStencilStateDesc.backFaceOp.stencilFailOp = StencilOp_Keep;
			depthStencilStateDesc.backFaceOp.stencilDepthFailOp = StencilOp_Keep;
			depthStencilStateDesc.backFaceOp.stencilPassOp = StencilOp_Keep;
			depthStencilStateDesc.backFaceOp.stencilFunc = RenderCompareFunction_Equal;
			commandRecorder.setPipelineDepthStencilState(depthStencilStateDesc);

			BlendStateDesc blendStateDesc;
			blendStateDesc.targetBlend.blendEnable = false;
			blendStateDesc.targetBlend.renderTargetWriteMask = ColorWriteEnable_All;
			commandRecorder.setPipelineBlendState(blendStateDesc);

			ResourceFormat colorTargetFormats[] =
			{
				resources.getResourceDesc(data.textureColor).format,
			};

			GraphicsPipelineCompileInfo graphicsPipelineCompileInfo;
			graphicsPipelineCompileInfo.shaderProgram = ShaderProgram_Stencil_Draw_2_VSPS;
			graphicsPipelineCompileInfo.primitiveTopologyType = PrimitiveTopologyType_Triangle;
			graphicsPipelineCompileInfo.numRenderTargets = 1;
			graphicsPipelineCompileInfo.targetFormats = colorTargetFormats;
			graphicsPipelineCompileInfo.depthFormat = resources.getResourceDesc(data.textureDepth_Stencil).format;
			graphicsPipelineCompileInfo.inputLayout = nullptr;
			const GraphicsPipelineInfo& graphicsPipelineInfo = s_pipelineCache.getGraphicsPipeline(graphicsPipelineCompileInfo);
			commandRecorder.setPipeline(graphicsPipelineInfo.pipeline);

			commandRecorder.setStencilReference(128);

			Rc2ShaderProgramBinder rc2ShaderProgramBinder(commandRecorder, graphicsPipelineInfo.shaderProgramInfo);

			rc2ShaderProgramBinder.pixelStage()
				.setSrv(0, resources.getSrv(data.texture_loadedTexture_0))
			;

			rc2ShaderProgramBinder.bind();

			// Draw
			commandRecorder.drawInstanced(PrimitiveTopology_TriangleList, data.vertexCountPerInstance, data.instanceCount, 0, 0);

			commandRecorder.endRenderPass();
		});
	}

	// bufferVertex_Buffer  TODO: Put this buffer into the black board (Optional)
	// bufferColor TODO: Put this texture into the black board (Optional)
	// bufferDepth_Stencil TODO: Put this texture into the black board (Optional)
}

void StencilRenderPassModule::onInit(const InitParams& params)
{
	m_arena = params.memoryArena;

	// Register with framegraph
	renderPassBookmark::registerPass(RenderPassBookmark_AfterPost,
		[this](FrameGraph& frameGraph, FrameGraphBlackboard& blackboard) { addPass(frameGraph, blackboard); });
}

StencilRenderPassModule::~StencilRenderPassModule()
{
	s_pipelineCache.clear();
}

}
