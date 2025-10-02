// (c) Electronic Arts.  All Rights Reserved.

#include "NoVertex_NoIndex_NoInstanceRenderPassModule.h"

#include <Engine.Base/Misc/Vars.h>
#include <Engine.Math/Math.h>
#include <Engine.Render/Core/GetTextureResource.h>
#include <Engine.Render/Core2/Rc2ShaderProgramBinder.h>
#include <Engine.Render/FrameGraph/FrameGraph.h>
#include <Engine.Render/FrameGraph/FrameGraphBookmark.h>
#include <Engine.Render/FrameGraph/FrameGraphRenderPassData.h>
#include <Engine.Render/Raytrace/RaytracingUtil.h>
#include <Engine.World.Render/FrameGraphWorldRenderer.h>
#include <Engine.World.Render/NoVertex_NoIndex_NoInstance/NoVertex_NoIndex_NoInstanceShaderProgramsDefs.h>
#include <Engine.World.Render/Raytrace/RaytraceRenderModule.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>

namespace fb
{
ShaderProgramPipelineCache NoVertex_NoIndex_NoInstanceRenderPassModule::s_pipelineCache;

template <typename T>
T Pow2GE(const T& A)
{
    float f = std::log2(float(A));
    f = std::ceilf(f);
    return (T)std::pow(2.0f, f);
}

void NoVertex_NoIndex_NoInstanceRenderPassModule::addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard)
{

	// Initialization - create resources internal to the technique
	FrameGraphMutableResource textureColor_Buffer;
	FrameGraphMutableResource textureDepth_Buffer;
	{
		struct PassData
		{
		};

		frameGraph.addRc2CallbackPass<PassData>("NoVertex_NoIndex_NoInstance.Initialization", [&](RenderPassBuilderBase& builder, PassData& data)
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
				desc.name = "NoVertex_NoIndex_NoInstance.Color_Buffer";
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
				desc.name = "NoVertex_NoIndex_NoInstance.Depth_Buffer";
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
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
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
			Struct__VertexShaderCB cb_VertexShaderCB;
		};

		frameGraph.addRc2CallbackPass<PassData>("NoVertex_NoIndex_NoInstance.Rasterize",
		[&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureColor_Buffer = builder.writeAndUpdate(textureColor_Buffer, FrameGraphResourceUsage::FrameGraphResourceUsage_RenderTarget);
			data.textureDepth_Buffer = builder.writeAndUpdate(textureDepth_Buffer, FrameGraphResourceUsage::FrameGraphResourceUsage_DepthWrite);
			data.cb_VertexShaderCB = m_cb__VertexShaderCB;

			data.vertexCountPerInstance = 6;

			data.instanceCount = 5;
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			RenderPassDesc renderPassDesc;
			renderPassDesc.name = "NoVertex_NoIndex_NoInstance.Rasterize";
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
			rasterizerStateDesc.cullMode = CullMode_Back;
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

			ResourceFormat colorTargetFormats[] =
			{
				resources.getResourceDesc(data.textureColor_Buffer).format,
			};

			GraphicsPipelineCompileInfo graphicsPipelineCompileInfo;
			graphicsPipelineCompileInfo.shaderProgram = ShaderProgram_NoVertex_NoIndex_NoInstance_Rasterize_VSPS;
			graphicsPipelineCompileInfo.primitiveTopologyType = PrimitiveTopologyType_Triangle;
			graphicsPipelineCompileInfo.numRenderTargets = 1;
			graphicsPipelineCompileInfo.targetFormats = colorTargetFormats;
			graphicsPipelineCompileInfo.depthFormat = resources.getResourceDesc(data.textureDepth_Buffer).format;
			graphicsPipelineCompileInfo.inputLayout = nullptr;
			const GraphicsPipelineInfo& graphicsPipelineInfo = s_pipelineCache.getGraphicsPipeline(graphicsPipelineCompileInfo);
			commandRecorder.setPipeline(graphicsPipelineInfo.pipeline);

			commandRecorder.setStencilReference(0);

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

	// bufferColor_Buffer TODO: Put this texture into the black board (Optional)
	// bufferDepth_Buffer TODO: Put this texture into the black board (Optional)
}

void NoVertex_NoIndex_NoInstanceRenderPassModule::onInit(const InitParams& params)
{
	m_arena = params.memoryArena;

	// Register with framegraph
	renderPassBookmark::registerPass(RenderPassBookmark_AfterPost,
		[this](FrameGraph& frameGraph, FrameGraphBlackboard& blackboard) { addPass(frameGraph, blackboard); });
}

NoVertex_NoIndex_NoInstanceRenderPassModule::~NoVertex_NoIndex_NoInstanceRenderPassModule()
{
	s_pipelineCache.clear();
}

}
