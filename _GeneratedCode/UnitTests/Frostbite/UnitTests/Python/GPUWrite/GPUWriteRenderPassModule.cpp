// (c) Electronic Arts.  All Rights Reserved.

#include "GPUWriteRenderPassModule.h"

#include <Engine.Base/Misc/Vars.h>
#include <Engine.Math/Math.h>
#include <Engine.Render/Core/GetTextureResource.h>
#include <Engine.Render/Core2/Rc2ShaderProgramBinder.h>
#include <Engine.Render/FrameGraph/FrameGraph.h>
#include <Engine.Render/FrameGraph/FrameGraphBookmark.h>
#include <Engine.Render/FrameGraph/FrameGraphRenderPassData.h>
#include <Engine.Render/Raytrace/RaytracingUtil.h>
#include <Engine.World.Render/FrameGraphWorldRenderer.h>
#include <Engine.World.Render/GPUWrite/GPUWriteShaderProgramsDefs.h>
#include <Engine.World.Render/Raytrace/RaytraceRenderModule.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>

namespace fb
{
ShaderProgramPipelineCache GPUWriteRenderPassModule::s_pipelineCache;

template <typename T>
T Pow2GE(const T& A)
{
    float f = std::log2(float(A));
    f = std::ceilf(f);
    return (T)std::pow(2.0f, f);
}

void GPUWriteRenderPassModule::addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard)
{

	// Initialization - create resources internal to the technique
	FrameGraphMutableResource textureTexture2D;
	FrameGraphMutableResource textureTexture2DArray;
	FrameGraphMutableResource textureTexture3D;
	FrameGraphMutableResource bufferFloatBuffer;
	FrameGraphMutableResource bufferStructBuffer;
	{
		struct PassData
		{
		};

		frameGraph.addRc2CallbackPass<PassData>("GPUWrite.Initialization", [&](RenderPassBuilderBase& builder, PassData& data)
		{

			// Texture: Texture2D
			{
				TextureType type = TextureType_2d;
				u32 bindFlags = RenderBindFlag_ShaderResource | RenderBindFlag_UnorderedAccess;
				RenderFormat format = RenderFormat_R8G8B8A8_UNORM;
				uint width = ((1 + 0) * 32) / 1 + 0;
				uint height = ((1 + 0) * 32) / 1 + 0;
				uint depth = ((1 + 0) * 1) / 1 + 0;

				// (Re)create the texture as needed
				if (!m_textureTexture2D || m_textureTexture2D_format != format || m_textureTexture2D_width != width  || m_textureTexture2D_height != height || m_textureTexture2D_depth != depth)
				{
					m_textureTexture2D_format = format;
					m_textureTexture2D_width = width;
					m_textureTexture2D_height = height;
					m_textureTexture2D_depth = depth;

					TextureCreateDesc desc;
					desc.name = "GPUWrite.Texture2D";
					desc.type = type;
					desc.bindFlags = bindFlags;
					desc.format = format;
					desc.width = width;
					desc.height = height;
					desc.depth = depth;
					m_textureTexture2D = g_renderer->createTexture(*m_arena, desc);
				}

				textureTexture2D = frameGraph.importResource(m_textureTexture2D.get());
			}

			// Texture: Texture2DArray
			{
				TextureType type = TextureType_2dArray;
				u32 bindFlags = RenderBindFlag_ShaderResource | RenderBindFlag_UnorderedAccess;
				RenderFormat format = builder.getTextureDesc(textureTexture2D).format;
				uint width = ((1 + 0) * 32) / 1 + 0;
				uint height = ((1 + 0) * 32) / 1 + 0;
				uint depth = ((1 + 0) * 3) / 1 + 0;

				// (Re)create the texture as needed
				if (!m_textureTexture2DArray || m_textureTexture2DArray_format != format || m_textureTexture2DArray_width != width  || m_textureTexture2DArray_height != height || m_textureTexture2DArray_depth != depth)
				{
					m_textureTexture2DArray_format = format;
					m_textureTexture2DArray_width = width;
					m_textureTexture2DArray_height = height;
					m_textureTexture2DArray_depth = depth;

					TextureCreateDesc desc;
					desc.name = "GPUWrite.Texture2DArray";
					desc.type = type;
					desc.bindFlags = bindFlags;
					desc.format = format;
					desc.width = width;
					desc.height = height;
					desc.depth = depth;
					m_textureTexture2DArray = g_renderer->createTexture(*m_arena, desc);
				}

				textureTexture2DArray = frameGraph.importResource(m_textureTexture2DArray.get());
			}

			// Texture: Texture3D
			{
				TextureType type = TextureType_3d;
				u32 bindFlags = RenderBindFlag_ShaderResource | RenderBindFlag_UnorderedAccess;
				RenderFormat format = builder.getTextureDesc(textureTexture2D).format;
				uint width = ((1 + 0) * 32) / 1 + 0;
				uint height = ((1 + 0) * 32) / 1 + 0;
				uint depth = ((1 + 0) * 3) / 1 + 0;

				// (Re)create the texture as needed
				if (!m_textureTexture3D || m_textureTexture3D_format != format || m_textureTexture3D_width != width  || m_textureTexture3D_height != height || m_textureTexture3D_depth != depth)
				{
					m_textureTexture3D_format = format;
					m_textureTexture3D_width = width;
					m_textureTexture3D_height = height;
					m_textureTexture3D_depth = depth;

					TextureCreateDesc desc;
					desc.name = "GPUWrite.Texture3D";
					desc.type = type;
					desc.bindFlags = bindFlags;
					desc.format = format;
					desc.width = width;
					desc.height = height;
					desc.depth = depth;
					m_textureTexture3D = g_renderer->createTexture(*m_arena, desc);
				}

				textureTexture3D = frameGraph.importResource(m_textureTexture3D.get());
			}

			// Buffer: FloatBuffer
			{
				uint32_t bindFlags = RenderBindFlag_ShaderResource | RenderBindFlag_UnorderedAccess;
				uint32_t memoryFlags = RenderResourceMemoryFlags_ZeroInitialize;
				RenderFormat format = RenderFormat::RenderFormat_R32_FLOAT;
				uint32_t structByteStride = 0;
				uint32_t count = ((1 + 0) * 8) / 1 + 0;
				uint32_t size = count * ((structByteStride > 0) ? structByteStride : (uint32_t)getFormatBytesPerElement(convertRenderFormat(format)));

				// (Re)create the buffer as needed
				if (!m_bufferFloatBuffer || m_bufferFloatBuffer_format != format || m_bufferFloatBuffer_size != size)
				{
					m_bufferFloatBuffer_format = format;
					m_bufferFloatBuffer_size = size;

					RenderBufferCreateDesc desc;
					desc.name = "GPUWrite.FloatBuffer";
					desc.usage = RenderResourceUsage_Static;
					desc.bindFlags = bindFlags;
					desc.memoryFlags = memoryFlags;
					desc.format = format;
					desc.structByteStride = structByteStride;
					desc.size = size;
					m_bufferFloatBuffer = g_renderer->createBuffer(*m_arena, desc);
				}

				// import the resource
				bufferFloatBuffer = frameGraph.importResource(m_bufferFloatBuffer.get());
			}

			// Buffer: StructBuffer
			{
				uint32_t bindFlags = RenderBindFlag_ShaderResource | RenderBindFlag_UnorderedAccess;
				uint32_t memoryFlags = RenderResourceMemoryFlags_StructuredBuffer | RenderResourceMemoryFlags_ZeroInitialize;
				RenderFormat format = RenderFormat::RenderFormat_Unknown;
				uint32_t structByteStride = 8;
				uint32_t count = ((1 + 0) * 1) / 1 + 0;
				uint32_t size = count * ((structByteStride > 0) ? structByteStride : (uint32_t)getFormatBytesPerElement(convertRenderFormat(format)));

				// (Re)create the buffer as needed
				if (!m_bufferStructBuffer || m_bufferStructBuffer_format != format || m_bufferStructBuffer_size != size)
				{
					m_bufferStructBuffer_format = format;
					m_bufferStructBuffer_size = size;

					RenderBufferCreateDesc desc;
					desc.name = "GPUWrite.StructBuffer";
					desc.usage = RenderResourceUsage_Static;
					desc.bindFlags = bindFlags;
					desc.memoryFlags = memoryFlags;
					desc.format = format;
					desc.structByteStride = structByteStride;
					desc.size = size;
					m_bufferStructBuffer = g_renderer->createBuffer(*m_arena, desc);
				}

				// import the resource
				bufferStructBuffer = frameGraph.importResource(m_bufferStructBuffer.get());
			}
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
		});
	}

	// Transition resources for the next action
	{
		struct PassData
		{
			FrameGraphMutableResource textureTexture2D;
			FrameGraphMutableResource textureTexture2DArray;
			FrameGraphMutableResource textureTexture3D;
			FrameGraphMutableResource bufferFloatBuffer;
			FrameGraphMutableResource bufferStructBuffer;
		};

		frameGraph.addRc2CallbackPass<PassData>("GPUWrite.Transitions", [&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureTexture2D = builder.writeAndUpdate(textureTexture2D, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.textureTexture2DArray = builder.writeAndUpdate(textureTexture2DArray, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.textureTexture3D = builder.writeAndUpdate(textureTexture3D, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.bufferFloatBuffer = builder.writeAndUpdate(bufferFloatBuffer, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.bufferStructBuffer = builder.writeAndUpdate(bufferStructBuffer, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			commandRecorder.computeBarrier(resources.getRenderResource(data.textureTexture2D));
			commandRecorder.computeBarrier(resources.getRenderResource(data.textureTexture2DArray));
			commandRecorder.computeBarrier(resources.getRenderResource(data.textureTexture3D));
			commandRecorder.computeBarrier(resources.getRenderResource(data.bufferFloatBuffer));
			commandRecorder.computeBarrier(resources.getRenderResource(data.bufferStructBuffer));
		});
	}

	// Compute Shader: ComputeShader
	{
		struct PassData
		{
			uint dispatchX;
			uint dispatchY;
			uint dispatchZ;
			FrameGraphMutableResource textureTexture2D;
			FrameGraphMutableResource textureTexture2DArray;
			FrameGraphMutableResource textureTexture3D;
			FrameGraphMutableResource bufferFloatBuffer;
			FrameGraphMutableResource bufferStructBuffer;
		};

		frameGraph.addRc2CallbackPass<PassData>("GPUWrite.ComputeShader",
		[&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureTexture2D = builder.writeAndUpdate(textureTexture2D, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.textureTexture2DArray = builder.writeAndUpdate(textureTexture2DArray, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.textureTexture3D = builder.writeAndUpdate(textureTexture3D, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.bufferFloatBuffer = builder.writeAndUpdate(bufferFloatBuffer, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.bufferStructBuffer = builder.writeAndUpdate(bufferStructBuffer, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.dispatchX = ((1 + 0) * 1) / 1 + 0;
			data.dispatchY = ((1 + 0) * 1) / 1 + 0;
			data.dispatchZ = ((1 + 0) * 1) / 1 + 0;
			data.dispatchX = (data.dispatchX + 8 - 1) / 8;
			data.dispatchY = (data.dispatchY + 8 - 1) / 8;
			data.dispatchZ = (data.dispatchZ + 1 - 1) / 1;
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			const ComputePipelineInfo& info = s_pipelineCache.getComputePipeline(ShaderProgram_GPUWrite_ComputeShader_csmainCs, 0);
			commandRecorder.setPipeline(info.pipeline);

			Rc2ComputeShaderProgramBinder binder(commandRecorder, info.computeShaderInfo);

			binder.computeStage()
				.setUav(0, resources.getUav(data.textureTexture2D))
				.setUav(1, resources.getUav(data.textureTexture2DArray))
				.setUav(2, resources.getUav(data.textureTexture3D))
				.setUav(3, resources.getUav(data.bufferFloatBuffer))
				.setUav(4, resources.getUav(data.bufferStructBuffer));
			binder.bind();

			commandRecorder.dispatch(data.dispatchX, data.dispatchY, data.dispatchZ);
		});
	}

}

void GPUWriteRenderPassModule::onInit(const InitParams& params)
{
	m_arena = params.memoryArena;

	// Register with framegraph
	renderPassBookmark::registerPass(RenderPassBookmark_AfterPost,
		[this](FrameGraph& frameGraph, FrameGraphBlackboard& blackboard) { addPass(frameGraph, blackboard); });
}

GPUWriteRenderPassModule::~GPUWriteRenderPassModule()
{
	s_pipelineCache.clear();
}

}
