// (c) Electronic Arts.  All Rights Reserved.

#include "boxblurRenderPassModule.h"

#include <Engine.Base/Misc/Vars.h>
#include <Engine.Math/Math.h>
#include <Engine.Render/Core/GetTextureResource.h>
#include <Engine.Render/Core2/Rc2ShaderProgramBinder.h>
#include <Engine.Render/FrameGraph/FrameGraph.h>
#include <Engine.Render/FrameGraph/FrameGraphBookmark.h>
#include <Engine.Render/FrameGraph/FrameGraphRenderPassData.h>
#include <Engine.Render/Raytrace/RaytracingUtil.h>
#include <Engine.World.Render/FrameGraphWorldRenderer.h>
#include <Engine.World.Render/boxblur/boxblurShaderProgramsDefs.h>
#include <Engine.World.Render/Raytrace/RaytraceRenderModule.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>

// User Variables
namespace
{
	FB_DECLARE_VAR_BOOL(g_varenabled, "boxblur.enabled", true);  // Enables or disables the blurring effect.
	FB_DECLARE_VAR_INT(g_varradius, "boxblur.radius", 2);  // The radius of the blur.  Actual size in pixles of the blur is (radius*2+1)^2
	FB_DECLARE_VAR_BOOL(g_varsRGB, "boxblur.sRGB", true);
}

namespace fb
{
ShaderProgramPipelineCache boxblurRenderPassModule::s_pipelineCache;

template <typename T>
T Pow2GE(const T& A)
{
    float f = std::log2(float(A));
    f = std::ceilf(f);
    return (T)std::pow(2.0f, f);
}

void boxblurRenderPassModule::addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard)
{
	// TODO: Provide this imported texture
	FrameGraphMutableResource& textureInputTexture = ;  // This is the texture to be blurred

	// Initialization - create resources internal to the technique
	FrameGraphMutableResource texturePingPongTexture;  // An internal texture used during the blurring process
	{
		struct PassData
		{
		};

		frameGraph.addRc2CallbackPass<PassData>("boxblur.Initialization", [&](RenderPassBuilderBase& builder, PassData& data)
		{

			// Texture: PingPongTexture
			// An internal texture used during the blurring process
			{
				TextureType type = TextureType_2d;
				u32 bindFlags = RenderBindFlag_ShaderResource | RenderBindFlag_UnorderedAccess;
				RenderFormat format = builder.getTextureDesc(textureInputTexture).format;
				FrameGraphTextureDesc sizeSrcDesc = builder.getTextureDesc(textureInputTexture);
				uint width = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
				uint height = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
				uint depth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;

				FrameGraphTextureDesc desc;
				desc.name = "boxblur.PingPongTexture";
				desc.usage = RenderResourceUsage_Static;
				desc.initialState = FrameGraphTextureDesc::InitialState_Undefined;
				desc.type = type;
				desc.bindFlags = bindFlags;
				desc.format = format;
				desc.width = width;
				desc.height = height;
				desc.depth = depth;
				texturePingPongTexture = builder.createTexture(desc, FrameGraphResourceUsage_UnorderedAccess);
			}
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
		});
	}

	// Shader Constants: _BoxBlur_0CB
	{
		m_cb__BoxBlur_0CB.radius = g_varradius;
		m_cb__BoxBlur_0CB.sRGB = g_varsRGB;
	}

	// Compute Shader: BlurH
	// Horizontal blur pass
	if(g_varenabled)
	{
		struct PassData
		{
			uint dispatchX;
			uint dispatchY;
			uint dispatchZ;
			FrameGraphResource textureInputTexture;
			FrameGraphMutableResource texturePingPongTexture;
			Struct__BoxBlur_0CB cb_BoxBlur_0CB;
		};

		frameGraph.addRc2CallbackPass<PassData>("boxblur.BlurH",
		[&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureInputTexture = builder.read(textureInputTexture);
			data.texturePingPongTexture = builder.writeAndUpdate(texturePingPongTexture, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.cb_BoxBlur_0CB = m_cb__BoxBlur_0CB;
			const FrameGraphTextureDesc& desc = builder.getTextureDesc(textureInputTexture);
			data.dispatchX = ((desc.width + 0) * 1) / 1 + 0;
			data.dispatchY = ((desc.height + 0) * 1) / 1 + 0;
			data.dispatchZ = ((desc.depth + 0) * 1) / 1 + 0;
			data.dispatchX = (data.dispatchX + 8 - 1) / 8;
			data.dispatchY = (data.dispatchY + 8 - 1) / 8;
			data.dispatchZ = (data.dispatchZ + 1 - 1) / 1;
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			const ComputePipelineInfo& info = s_pipelineCache.getComputePipeline(ShaderProgram_boxblur_BlurH_BlurHCs, 0);
			commandRecorder.setPipeline(info.pipeline);

			Rc2ComputeShaderProgramBinder binder(commandRecorder, info.computeShaderInfo);

			binder.computeStage()
				.setSrv(0, resources.getSrv(data.textureInputTexture))
				.setUav(0, resources.getUav(data.texturePingPongTexture))
				.setConstantBuffer(sizeof(data.cb_BoxBlur_0CB), &data.cb_BoxBlur_0CB);
			binder.bind();

			commandRecorder.dispatch(data.dispatchX, data.dispatchY, data.dispatchZ);
		});
	}

	// Shader Constants: _BoxBlur_1CB
	{
		m_cb__BoxBlur_1CB.radius = g_varradius;
		m_cb__BoxBlur_1CB.sRGB = g_varsRGB;
	}

	// Compute Shader: BlurV
	// Vertical blur pass
	if(g_varenabled)
	{
		struct PassData
		{
			uint dispatchX;
			uint dispatchY;
			uint dispatchZ;
			FrameGraphResource texturePingPongTexture;
			FrameGraphMutableResource textureInputTexture;
			Struct__BoxBlur_1CB cb_BoxBlur_1CB;
		};

		frameGraph.addRc2CallbackPass<PassData>("boxblur.BlurV",
		[&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.texturePingPongTexture = builder.read(texturePingPongTexture);
			data.textureInputTexture = builder.writeAndUpdate(textureInputTexture, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.cb_BoxBlur_1CB = m_cb__BoxBlur_1CB;
			const FrameGraphTextureDesc& desc = builder.getTextureDesc(textureInputTexture);
			data.dispatchX = ((desc.width + 0) * 1) / 1 + 0;
			data.dispatchY = ((desc.height + 0) * 1) / 1 + 0;
			data.dispatchZ = ((desc.depth + 0) * 1) / 1 + 0;
			data.dispatchX = (data.dispatchX + 8 - 1) / 8;
			data.dispatchY = (data.dispatchY + 8 - 1) / 8;
			data.dispatchZ = (data.dispatchZ + 1 - 1) / 1;
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			const ComputePipelineInfo& info = s_pipelineCache.getComputePipeline(ShaderProgram_boxblur_BlurV_BlurVCs, 0);
			commandRecorder.setPipeline(info.pipeline);

			Rc2ComputeShaderProgramBinder binder(commandRecorder, info.computeShaderInfo);

			binder.computeStage()
				.setSrv(0, resources.getSrv(data.texturePingPongTexture))
				.setUav(0, resources.getUav(data.textureInputTexture))
				.setConstantBuffer(sizeof(data.cb_BoxBlur_1CB), &data.cb_BoxBlur_1CB);
			binder.bind();

			commandRecorder.dispatch(data.dispatchX, data.dispatchY, data.dispatchZ);
		});
	}

	// bufferInputTexture TODO: Put this texture into the black board (Optional)
}

void boxblurRenderPassModule::onInit(const InitParams& params)
{
	m_arena = params.memoryArena;

	// Register with framegraph
	renderPassBookmark::registerPass(RenderPassBookmark_AfterPost,
		[this](FrameGraph& frameGraph, FrameGraphBlackboard& blackboard) { addPass(frameGraph, blackboard); });
}

boxblurRenderPassModule::~boxblurRenderPassModule()
{
	s_pipelineCache.clear();
}

}
