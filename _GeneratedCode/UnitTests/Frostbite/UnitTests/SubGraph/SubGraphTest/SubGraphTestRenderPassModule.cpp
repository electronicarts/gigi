// (c) Electronic Arts.  All Rights Reserved.

#include "SubGraphTestRenderPassModule.h"

#include <Engine.Base/Misc/Vars.h>
#include <Engine.Math/Math.h>
#include <Engine.Render/Core/GetTextureResource.h>
#include <Engine.Render/Core2/Rc2ShaderProgramBinder.h>
#include <Engine.Render/FrameGraph/FrameGraph.h>
#include <Engine.Render/FrameGraph/FrameGraphBookmark.h>
#include <Engine.Render/FrameGraph/FrameGraphRenderPassData.h>
#include <Engine.Render/Raytrace/RaytracingUtil.h>
#include <Engine.World.Render/FrameGraphWorldRenderer.h>
#include <Engine.World.Render/SubGraphTest/SubGraphTestShaderProgramsDefs.h>
#include <Engine.World.Render/Raytrace/RaytraceRenderModule.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>

namespace fb
{
ShaderProgramPipelineCache SubGraphTestRenderPassModule::s_pipelineCache;

template <typename T>
T Pow2GE(const T& A)
{
    float f = std::log2(float(A));
    f = std::ceilf(f);
    return (T)std::pow(2.0f, f);
}

void SubGraphTestRenderPassModule::addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard)
{
	// TODO: Provide this imported texture
	FrameGraphResource& textureTest = ;

	// Initialization - create resources internal to the technique
	FrameGraphMutableResource textureInner_Exported_Tex;
	{
		struct PassData
		{
		};

		frameGraph.addRc2CallbackPass<PassData>("SubGraphTest.Initialization", [&](RenderPassBuilderBase& builder, PassData& data)
		{

			// Texture: Inner_Exported_Tex
			{
				TextureType type = TextureType_2d;
				u32 bindFlags = RenderBindFlag_ShaderResource | RenderBindFlag_UnorderedAccess;
				RenderFormat format = builder.getTextureDesc(textureTest).format;
				FrameGraphTextureDesc sizeSrcDesc = builder.getTextureDesc(textureTest);
				uint width = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
				uint height = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
				uint depth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;

				FrameGraphTextureDesc desc;
				desc.name = "SubGraphTest.Inner_Exported_Tex";
				desc.usage = RenderResourceUsage_Static;
				desc.initialState = FrameGraphTextureDesc::InitialState_Undefined;
				desc.type = type;
				desc.bindFlags = bindFlags;
				desc.format = format;
				desc.width = width;
				desc.height = height;
				desc.depth = depth;
				textureInner_Exported_Tex = builder.createTexture(desc, FrameGraphResourceUsage_CopyDestination);
			}
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
		});
	}

	// Copy Resource: Inner_Copy_Import_To_Export
	{
		struct PassData
		{
			FrameGraphResource source;
			FrameGraphMutableResource dest;
		};

		frameGraph.addRc2CallbackPass<PassData>("SubGraphTest.Inner_Copy_Import_To_Export", [&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.source = builder.read(textureTest);
			data.dest = builder.writeAndUpdate(textureInner_Exported_Tex, FrameGraphResourceUsage_CopyDestination);
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

	// Compute Shader: Swap_Colors
	{
		struct PassData
		{
			uint dispatchX;
			uint dispatchY;
			uint dispatchZ;
			FrameGraphMutableResource textureInner_Exported_Tex;
		};

		frameGraph.addRc2CallbackPass<PassData>("SubGraphTest.Swap_Colors",
		[&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureInner_Exported_Tex = builder.writeAndUpdate(textureInner_Exported_Tex, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			const FrameGraphTextureDesc& desc = builder.getTextureDesc(textureTest);
			data.dispatchX = ((desc.width + 0) * 1) / 1 + 0;
			data.dispatchY = ((desc.height + 0) * 1) / 1 + 0;
			data.dispatchZ = ((desc.depth + 0) * 1) / 1 + 0;
			data.dispatchX = (data.dispatchX + 8 - 1) / 8;
			data.dispatchY = (data.dispatchY + 8 - 1) / 8;
			data.dispatchZ = (data.dispatchZ + 1 - 1) / 1;
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			const ComputePipelineInfo& info = s_pipelineCache.getComputePipeline(ShaderProgram_SubGraphTest_Swap_Colors_csmainCs, 0);
			commandRecorder.setPipeline(info.pipeline);

			Rc2ComputeShaderProgramBinder binder(commandRecorder, info.computeShaderInfo);

			binder.computeStage()
				.setUav(0, resources.getUav(data.textureInner_Exported_Tex));
			binder.bind();

			commandRecorder.dispatch(data.dispatchX, data.dispatchY, data.dispatchZ);
		});
	}

	// bufferTest TODO: Put this texture into the black board (Optional)
	// bufferInner_Exported_Tex TODO: Put this texture into the black board (Optional)
}

void SubGraphTestRenderPassModule::onInit(const InitParams& params)
{
	m_arena = params.memoryArena;

	// Register with framegraph
	renderPassBookmark::registerPass(RenderPassBookmark_AfterPost,
		[this](FrameGraph& frameGraph, FrameGraphBlackboard& blackboard) { addPass(frameGraph, blackboard); });
}

SubGraphTestRenderPassModule::~SubGraphTestRenderPassModule()
{
	s_pipelineCache.clear();
}

}
