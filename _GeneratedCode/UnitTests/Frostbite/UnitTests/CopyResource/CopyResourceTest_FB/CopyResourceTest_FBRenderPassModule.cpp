// (c) Electronic Arts.  All Rights Reserved.

#include "CopyResourceTest_FBRenderPassModule.h"

#include <Engine.Base/Misc/Vars.h>
#include <Engine.Math/Math.h>
#include <Engine.Render/Core/GetTextureResource.h>
#include <Engine.Render/Core2/Rc2ShaderProgramBinder.h>
#include <Engine.Render/FrameGraph/FrameGraph.h>
#include <Engine.Render/FrameGraph/FrameGraphBookmark.h>
#include <Engine.Render/FrameGraph/FrameGraphRenderPassData.h>
#include <Engine.Render/Raytrace/RaytracingUtil.h>
#include <Engine.World.Render/FrameGraphWorldRenderer.h>
#include <Engine.World.Render/CopyResourceTest_FB/CopyResourceTest_FBShaderProgramsDefs.h>
#include <Engine.World.Render/Raytrace/RaytraceRenderModule.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>

namespace fb
{
ShaderProgramPipelineCache CopyResourceTest_FBRenderPassModule::s_pipelineCache;

template <typename T>
T Pow2GE(const T& A)
{
    float f = std::log2(float(A));
    f = std::ceilf(f);
    return (T)std::pow(2.0f, f);
}

void CopyResourceTest_FBRenderPassModule::addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard)
{
	// TODO: Provide this imported texture
	FrameGraphResource& textureSource_Texture = ;
	// TODO: Provide this imported buffer
	FrameGraphResource& bufferSource_Buffer = ;

	// Initialization - create resources internal to the technique
	FrameGraphMutableResource textureTexture_From_Texture;
	FrameGraphMutableResource bufferBuffer_From_Buffer;
	{
		struct PassData
		{
		};

		frameGraph.addRc2CallbackPass<PassData>("CopyResourceTest_FB.Initialization", [&](RenderPassBuilderBase& builder, PassData& data)
		{

			// Texture: Texture_From_Texture
			{
				TextureType type = TextureType_2d;
				u32 bindFlags = RenderBindFlag_ShaderResource;
				RenderFormat format = builder.getTextureDesc(textureSource_Texture).format;
				FrameGraphTextureDesc sizeSrcDesc = builder.getTextureDesc(textureSource_Texture);
				uint width = ((sizeSrcDesc.width + 0) * 1) / 1 + 0;
				uint height = ((sizeSrcDesc.height + 0) * 1) / 1 + 0;
				uint depth = ((sizeSrcDesc.depth + 0) * 1) / 1 + 0;

				FrameGraphTextureDesc desc;
				desc.name = "CopyResourceTest_FB.Texture_From_Texture";
				desc.usage = RenderResourceUsage_Static;
				desc.initialState = FrameGraphTextureDesc::InitialState_Undefined;
				desc.type = type;
				desc.bindFlags = bindFlags;
				desc.format = format;
				desc.width = width;
				desc.height = height;
				desc.depth = depth;
				textureTexture_From_Texture = builder.createTexture(desc, FrameGraphResourceUsage_CopyDestination);
			}

			// Buffer: Buffer_From_Buffer
			{
				uint32_t bindFlags = RenderBindFlag_ShaderResource;
				uint32_t memoryFlags = RenderResourceMemoryFlags_ZeroInitialize;
				FrameGraphBufferDesc formatSrcDesc = builder.getBufferDesc(bufferSource_Buffer);
				RenderFormat format = formatSrcDesc.format;
				uint32_t structByteStride = formatSrcDesc.structByteStride;
				if (structByteStride > 0)
					memoryFlags |= RenderResourceMemoryFlags_StructuredBuffer;
				FrameGraphBufferDesc sizeSrcDesc = builder.getBufferDesc(bufferSource_Buffer);
				uint32_t size = ((sizeSrcDesc.size + 0) * 1) / 1 + 0;

				FrameGraphBufferDesc desc;
				desc.name = "CopyResourceTest_FB.Buffer_From_Buffer";
				desc.usage = RenderResourceUsage_Static;
				desc.initialState = FrameGraphBufferDesc::InitialState_Undefined;
				desc.bindFlags = bindFlags;
				desc.memoryFlags = memoryFlags;
				desc.format = format;
				desc.structByteStride = structByteStride;
				desc.size = size;
				bufferBuffer_From_Buffer = builder.createBuffer(desc, FrameGraphResourceUsage_CopyDestination);
			}
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
		});
	}

	// Copy Resource: Copy_Texture_To_Texture
	{
		struct PassData
		{
			FrameGraphResource source;
			FrameGraphMutableResource dest;
		};

		frameGraph.addRc2CallbackPass<PassData>("CopyResourceTest_FB.Copy_Texture_To_Texture", [&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.source = builder.read(textureSource_Texture);
			data.dest = builder.writeAndUpdate(textureTexture_From_Texture, FrameGraphResourceUsage_CopyDestination);
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

	// Copy Resource: Copy_Buffer_To_Buffer
	{
		struct PassData
		{
			FrameGraphResource source;
			FrameGraphMutableResource dest;
		};

		frameGraph.addRc2CallbackPass<PassData>("CopyResourceTest_FB.Copy_Buffer_To_Buffer", [&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.source = builder.read(bufferSource_Buffer);
			data.dest = builder.writeAndUpdate(bufferBuffer_From_Buffer, FrameGraphResourceUsage_CopyDestination);
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

	// bufferSource_Texture TODO: Put this texture into the black board (Optional)
	// bufferTexture_From_Texture TODO: Put this texture into the black board (Optional)
	// bufferSource_Buffer  TODO: Put this buffer into the black board (Optional)
	// bufferBuffer_From_Buffer TODO: Put this buffer into the black board (Optional)
}

void CopyResourceTest_FBRenderPassModule::onInit(const InitParams& params)
{
	m_arena = params.memoryArena;

	// Register with framegraph
	renderPassBookmark::registerPass(RenderPassBookmark_AfterPost,
		[this](FrameGraph& frameGraph, FrameGraphBlackboard& blackboard) { addPass(frameGraph, blackboard); });
}

CopyResourceTest_FBRenderPassModule::~CopyResourceTest_FBRenderPassModule()
{
	s_pipelineCache.clear();
}

}
