// (c) Electronic Arts.  All Rights Reserved.

#include "buffertest_viewOffsetCS_VarsRenderPassModule.h"

#include <Engine.Base/Misc/Vars.h>
#include <Engine.Math/Math.h>
#include <Engine.Render/Core/GetTextureResource.h>
#include <Engine.Render/Core2/Rc2ShaderProgramBinder.h>
#include <Engine.Render/FrameGraph/FrameGraph.h>
#include <Engine.Render/FrameGraph/FrameGraphBookmark.h>
#include <Engine.Render/FrameGraph/FrameGraphRenderPassData.h>
#include <Engine.Render/Raytrace/RaytracingUtil.h>
#include <Engine.World.Render/FrameGraphWorldRenderer.h>
#include <Engine.World.Render/buffertest_viewOffsetCS_Vars/buffertest_viewOffsetCS_VarsShaderProgramsDefs.h>
#include <Engine.World.Render/Raytrace/RaytraceRenderModule.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>

// User Variables
namespace
{
	FB_DECLARE_VAR_FLOAT(g_vargain, "buffertest_viewOffsetCS_Vars.gain", 0.5f);  // Overall Volume Adjustment
	FB_DECLARE_VAR_FLOAT(g_varalpha1, "buffertest_viewOffsetCS_Vars.alpha1", 1.f);  // Adjusts the contribution of sample n-1
	FB_DECLARE_VAR_FLOAT(g_varalpha2, "buffertest_viewOffsetCS_Vars.alpha2", 0.f);  // Adjusts the contribution of sample n-2
}

namespace fb
{
ShaderProgramPipelineCache buffertest_viewOffsetCS_VarsRenderPassModule::s_pipelineCache;

template <typename T>
T Pow2GE(const T& A)
{
    float f = std::log2(float(A));
    f = std::ceilf(f);
    return (T)std::pow(2.0f, f);
}

void buffertest_viewOffsetCS_VarsRenderPassModule::addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard)
{
	// TODO: Provide this imported buffer
	FrameGraphResource& bufferInputTypedBuffer = ;  // This is the buffer to be filtered.
	// TODO: Provide this imported buffer
	FrameGraphResource& bufferInputStructuredBuffer = ;
	// TODO: Provide this imported buffer
	FrameGraphResource& bufferInputTypedBufferRaw = ;  // This is the buffer to be filtered.
	// TODO: Provide this imported buffer
	FrameGraphResource& bufferInputTypedStructBuffer = ;  // This is the buffer to be filtered.

	// Initialization - create resources internal to the technique
	FrameGraphMutableResource bufferOutputTypedBuffer;  // An internal buffer used during the filtering process.
	FrameGraphMutableResource bufferOutputStructuredBuffer;
	FrameGraphMutableResource bufferOutputTypedBufferRaw;  // An internal buffer used during the filtering process.
	FrameGraphMutableResource bufferOutputTypedStructBuffer;  // An internal buffer used during the filtering process.
	{
		struct PassData
		{
		};

		frameGraph.addRc2CallbackPass<PassData>("buffertest_viewOffsetCS_Vars.Initialization", [&](RenderPassBuilderBase& builder, PassData& data)
		{

			// Buffer: OutputTypedBuffer
			// An internal buffer used during the filtering process.
			{
				uint32_t bindFlags = RenderBindFlag_ShaderResource | RenderBindFlag_UnorderedAccess;
				uint32_t memoryFlags = RenderResourceMemoryFlags_ZeroInitialize;
				FrameGraphBufferDesc formatSrcDesc = builder.getBufferDesc(bufferInputTypedBuffer);
				RenderFormat format = formatSrcDesc.format;
				uint32_t structByteStride = formatSrcDesc.structByteStride;
				if (structByteStride > 0)
					memoryFlags |= RenderResourceMemoryFlags_StructuredBuffer;
				FrameGraphBufferDesc sizeSrcDesc = builder.getBufferDesc(bufferInputTypedBuffer);
				uint32_t size = ((sizeSrcDesc.size + 0) * 1) / 1 + 0;

				FrameGraphBufferDesc desc;
				desc.name = "buffertest_viewOffsetCS_Vars.OutputTypedBuffer";
				desc.usage = RenderResourceUsage_Static;
				desc.initialState = FrameGraphBufferDesc::InitialState_Undefined;
				desc.bindFlags = bindFlags;
				desc.memoryFlags = memoryFlags;
				desc.format = format;
				desc.structByteStride = structByteStride;
				desc.size = size;
				bufferOutputTypedBuffer = builder.createBuffer(desc, FrameGraphResourceUsage_UnorderedAccess);
			}

			// Buffer: OutputStructuredBuffer
			{
				uint32_t bindFlags = RenderBindFlag_ShaderResource | RenderBindFlag_UnorderedAccess;
				uint32_t memoryFlags = RenderResourceMemoryFlags_StructuredBuffer | RenderResourceMemoryFlags_ZeroInitialize;
				FrameGraphBufferDesc formatSrcDesc = builder.getBufferDesc(bufferInputStructuredBuffer);
				RenderFormat format = formatSrcDesc.format;
				uint32_t structByteStride = formatSrcDesc.structByteStride;
				if (structByteStride > 0)
					memoryFlags |= RenderResourceMemoryFlags_StructuredBuffer;
				FrameGraphBufferDesc sizeSrcDesc = builder.getBufferDesc(bufferInputStructuredBuffer);
				uint32_t size = ((sizeSrcDesc.size + 0) * 2) / 1 + 0;

				FrameGraphBufferDesc desc;
				desc.name = "buffertest_viewOffsetCS_Vars.OutputStructuredBuffer";
				desc.usage = RenderResourceUsage_Static;
				desc.initialState = FrameGraphBufferDesc::InitialState_Undefined;
				desc.bindFlags = bindFlags;
				desc.memoryFlags = memoryFlags;
				desc.format = format;
				desc.structByteStride = structByteStride;
				desc.size = size;
				bufferOutputStructuredBuffer = builder.createBuffer(desc, FrameGraphResourceUsage_UnorderedAccess);
			}

			// Buffer: OutputTypedBufferRaw
			// An internal buffer used during the filtering process.
			{
				uint32_t bindFlags = RenderBindFlag_ShaderResource | RenderBindFlag_UnorderedAccess;
				uint32_t memoryFlags = RenderResourceMemoryFlags_BufferAllowRawViews | RenderResourceMemoryFlags_ZeroInitialize;
				FrameGraphBufferDesc formatSrcDesc = builder.getBufferDesc(bufferInputTypedBufferRaw);
				RenderFormat format = formatSrcDesc.format;
				uint32_t structByteStride = formatSrcDesc.structByteStride;
				if (structByteStride > 0)
					memoryFlags |= RenderResourceMemoryFlags_StructuredBuffer;
				FrameGraphBufferDesc sizeSrcDesc = builder.getBufferDesc(bufferInputTypedBufferRaw);
				uint32_t size = ((sizeSrcDesc.size + 0) * 1) / 1 + 0;

				FrameGraphBufferDesc desc;
				desc.name = "buffertest_viewOffsetCS_Vars.OutputTypedBufferRaw";
				desc.usage = RenderResourceUsage_Static;
				desc.initialState = FrameGraphBufferDesc::InitialState_Undefined;
				desc.bindFlags = bindFlags;
				desc.memoryFlags = memoryFlags;
				desc.format = format;
				desc.structByteStride = structByteStride;
				desc.size = size;
				bufferOutputTypedBufferRaw = builder.createBuffer(desc, FrameGraphResourceUsage_UnorderedAccess);
			}

			// Buffer: OutputTypedStructBuffer
			// An internal buffer used during the filtering process.
			{
				uint32_t bindFlags = RenderBindFlag_ShaderResource | RenderBindFlag_UnorderedAccess;
				uint32_t memoryFlags = RenderResourceMemoryFlags_StructuredBuffer | RenderResourceMemoryFlags_ZeroInitialize;
				FrameGraphBufferDesc formatSrcDesc = builder.getBufferDesc(bufferInputTypedBuffer);
				RenderFormat format = formatSrcDesc.format;
				uint32_t structByteStride = formatSrcDesc.structByteStride;
				if (structByteStride > 0)
					memoryFlags |= RenderResourceMemoryFlags_StructuredBuffer;
				FrameGraphBufferDesc sizeSrcDesc = builder.getBufferDesc(bufferInputTypedBuffer);
				uint32_t size = ((sizeSrcDesc.size + 0) * 1) / 1 + 0;

				FrameGraphBufferDesc desc;
				desc.name = "buffertest_viewOffsetCS_Vars.OutputTypedStructBuffer";
				desc.usage = RenderResourceUsage_Static;
				desc.initialState = FrameGraphBufferDesc::InitialState_Undefined;
				desc.bindFlags = bindFlags;
				desc.memoryFlags = memoryFlags;
				desc.format = format;
				desc.structByteStride = structByteStride;
				desc.size = size;
				bufferOutputTypedStructBuffer = builder.createBuffer(desc, FrameGraphResourceUsage_UnorderedAccess);
			}
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
		});
	}

	// Shader Constants: _BufferTest_0CB
	{
		m_cb__BufferTest_0CB.alpha1 = g_varalpha1;
		m_cb__BufferTest_0CB.alpha2 = g_varalpha2;
		m_cb__BufferTest_0CB.gain = g_vargain;
	}

	// Transition resources for the next action
	{
		struct PassData
		{
			FrameGraphMutableResource bufferOutputTypedBuffer;
			FrameGraphMutableResource bufferOutputStructuredBuffer;
			FrameGraphMutableResource bufferOutputTypedBufferRaw;
			FrameGraphMutableResource bufferOutputTypedStructBuffer;
		};

		frameGraph.addRc2CallbackPass<PassData>("buffertest_viewOffsetCS_Vars.Transitions", [&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.bufferOutputTypedBuffer = builder.writeAndUpdate(bufferOutputTypedBuffer, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.bufferOutputStructuredBuffer = builder.writeAndUpdate(bufferOutputStructuredBuffer, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.bufferOutputTypedBufferRaw = builder.writeAndUpdate(bufferOutputTypedBufferRaw, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.bufferOutputTypedStructBuffer = builder.writeAndUpdate(bufferOutputTypedStructBuffer, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			commandRecorder.computeBarrier(resources.getRenderResource(data.bufferOutputTypedBuffer));
			commandRecorder.computeBarrier(resources.getRenderResource(data.bufferOutputStructuredBuffer));
			commandRecorder.computeBarrier(resources.getRenderResource(data.bufferOutputTypedBufferRaw));
			commandRecorder.computeBarrier(resources.getRenderResource(data.bufferOutputTypedStructBuffer));
		});
	}

	// Compute Shader: BufferTest
	// BufferTest compute shader
	{
		struct PassData
		{
			uint dispatchX;
			uint dispatchY;
			uint dispatchZ;
			FrameGraphResource bufferInputTypedBuffer;
			FrameGraphMutableResource bufferOutputTypedBuffer;
			FrameGraphResource bufferInputTypedStructBuffer;
			FrameGraphMutableResource bufferOutputTypedStructBuffer;
			FrameGraphResource bufferInputStructuredBuffer;
			FrameGraphMutableResource bufferOutputStructuredBuffer;
			FrameGraphResource bufferInputTypedBufferRaw;
			FrameGraphMutableResource bufferOutputTypedBufferRaw;
			Struct__BufferTest_0CB cb_BufferTest_0CB;
		};

		frameGraph.addRc2CallbackPass<PassData>("buffertest_viewOffsetCS_Vars.BufferTest",
		[&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.bufferInputTypedBuffer = builder.read(bufferInputTypedBuffer);
			data.bufferOutputTypedBuffer = builder.writeAndUpdate(bufferOutputTypedBuffer, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.bufferInputTypedStructBuffer = builder.read(bufferInputTypedStructBuffer);
			data.bufferOutputTypedStructBuffer = builder.writeAndUpdate(bufferOutputTypedStructBuffer, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.bufferInputStructuredBuffer = builder.read(bufferInputStructuredBuffer);
			data.bufferOutputStructuredBuffer = builder.writeAndUpdate(bufferOutputStructuredBuffer, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.bufferInputTypedBufferRaw = builder.read(bufferInputTypedBufferRaw);
			data.bufferOutputTypedBufferRaw = builder.writeAndUpdate(bufferOutputTypedBufferRaw, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.cb_BufferTest_0CB = m_cb__BufferTest_0CB;
			const FrameGraphBufferDesc& desc = builder.getBufferDesc(bufferInputTypedBuffer);
			data.dispatchX = ((desc.size + 0) * 1) / 1 + 0;
			data.dispatchY = ((1 + 0) * 1) / 1 + 0;
			data.dispatchZ = ((1 + 0) * 1) / 1 + 0;
			data.dispatchX = (data.dispatchX + 64 - 1) / 64;
			data.dispatchY = (data.dispatchY + 1 - 1) / 1;
			data.dispatchZ = (data.dispatchZ + 1 - 1) / 1;
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			const ViewDesc viewInputTypedBuffer = ViewDesc::buildAsBufferView(ResourceBindFlags_ShaderResource, ResourceFormat_R32_FLOAT, m_varInputTypedBufferBegin, m_varInputTypedBufferSize);
			const ViewDesc viewOutputTypedBuffer = ViewDesc::buildAsBufferView(ResourceBindFlags_UnorderedAccess, ResourceFormat_R32_FLOAT, m_varOutputTypedBufferBegin, m_varOutputTypedBufferSize);
			const ViewDesc viewInputTypedStructBuffer = ViewDesc::buildAsStructuredBufferView(ResourceBindFlags_ShaderResource, m_varInputTypedStructBufferBegin, 4, (m_varInputTypedStructBufferSize/4));
			const ViewDesc viewOutputTypedStructBuffer = ViewDesc::buildAsStructuredBufferView(ResourceBindFlags_UnorderedAccess, m_varOutputTypedStructBufferBegin, 4, (m_varOutputTypedStructBufferSize/4));
			const ViewDesc viewInputStructuredBuffer = ViewDesc::buildAsStructuredBufferView(ResourceBindFlags_ShaderResource, m_varInputStructuredBufferBegin, 36, (m_varInputStructuredBufferSize/36));
			const ViewDesc viewOutputStructuredBuffer = ViewDesc::buildAsStructuredBufferView(ResourceBindFlags_UnorderedAccess, m_varOutputStructuredBufferBegin, 36, (m_varOutputStructuredBufferSize/36));
			const ViewDesc viewInputTypedBufferRaw = ViewDesc::buildAsRawBufferView(ResourceBindFlags_ShaderResource, m_varInputTypedBufferRawBegin, m_varInputTypedBufferRawSize);
			const ViewDesc viewOutputTypedBufferRaw = ViewDesc::buildAsRawBufferView(ResourceBindFlags_UnorderedAccess, m_varOutputTypedBufferRawBegin, m_varOutputTypedBufferRawSize);

			const ComputePipelineInfo& info = s_pipelineCache.getComputePipeline(ShaderProgram_buffertest_viewOffsetCS_Vars_BufferTest_MainCs, 0);
			commandRecorder.setPipeline(info.pipeline);

			Rc2ComputeShaderProgramBinder binder(commandRecorder, info.computeShaderInfo);

			binder.computeStage()
				.setSrv(0, resources.getBuffer(data.bufferInputTypedBuffer)->getAsSrv(viewInputTypedBuffer))
				.setUav(0, resources.getBuffer(data.bufferOutputTypedBuffer)->getAsUav(viewOutputTypedBuffer))
				.setSrv(1, resources.getBuffer(data.bufferInputTypedStructBuffer)->getAsSrv(viewInputTypedStructBuffer))
				.setUav(1, resources.getBuffer(data.bufferOutputTypedStructBuffer)->getAsUav(viewOutputTypedStructBuffer))
				.setSrv(2, resources.getBuffer(data.bufferInputStructuredBuffer)->getAsSrv(viewInputStructuredBuffer))
				.setUav(2, resources.getBuffer(data.bufferOutputStructuredBuffer)->getAsUav(viewOutputStructuredBuffer))
				.setSrv(3, resources.getBuffer(data.bufferInputTypedBufferRaw)->getAsSrv(viewInputTypedBufferRaw))
				.setUav(3, resources.getBuffer(data.bufferOutputTypedBufferRaw)->getAsUav(viewOutputTypedBufferRaw))
				.setConstantBuffer(sizeof(data.cb_BufferTest_0CB), &data.cb_BufferTest_0CB);
			binder.bind();

			commandRecorder.dispatch(data.dispatchX, data.dispatchY, data.dispatchZ);
		});
	}

	// bufferInputTypedBuffer  TODO: Put this buffer into the black board (Optional)
	// bufferOutputTypedBuffer TODO: Put this buffer into the black board (Optional)
	// bufferInputStructuredBuffer  TODO: Put this buffer into the black board (Optional)
	// bufferOutputStructuredBuffer TODO: Put this buffer into the black board (Optional)
	// bufferInputTypedBufferRaw  TODO: Put this buffer into the black board (Optional)
	// bufferOutputTypedBufferRaw TODO: Put this buffer into the black board (Optional)
	// bufferInputTypedStructBuffer  TODO: Put this buffer into the black board (Optional)
	// bufferOutputTypedStructBuffer TODO: Put this buffer into the black board (Optional)
}

void buffertest_viewOffsetCS_VarsRenderPassModule::onInit(const InitParams& params)
{
	m_arena = params.memoryArena;

	// Register with framegraph
	renderPassBookmark::registerPass(RenderPassBookmark_AfterPost,
		[this](FrameGraph& frameGraph, FrameGraphBlackboard& blackboard) { addPass(frameGraph, blackboard); });
}

buffertest_viewOffsetCS_VarsRenderPassModule::~buffertest_viewOffsetCS_VarsRenderPassModule()
{
	s_pipelineCache.clear();
}

}
