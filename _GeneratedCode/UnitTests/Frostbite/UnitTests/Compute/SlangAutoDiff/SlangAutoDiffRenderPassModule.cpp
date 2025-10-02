// (c) Electronic Arts.  All Rights Reserved.

#include "SlangAutoDiffRenderPassModule.h"

#include <Engine.Base/Misc/Vars.h>
#include <Engine.Math/Math.h>
#include <Engine.Render/Core/GetTextureResource.h>
#include <Engine.Render/Core2/Rc2ShaderProgramBinder.h>
#include <Engine.Render/FrameGraph/FrameGraph.h>
#include <Engine.Render/FrameGraph/FrameGraphBookmark.h>
#include <Engine.Render/FrameGraph/FrameGraphRenderPassData.h>
#include <Engine.Render/Raytrace/RaytracingUtil.h>
#include <Engine.World.Render/FrameGraphWorldRenderer.h>
#include <Engine.World.Render/SlangAutoDiff/SlangAutoDiffShaderProgramsDefs.h>
#include <Engine.World.Render/Raytrace/RaytraceRenderModule.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>

// User Variables
namespace
{
	FB_DECLARE_VAR_INT(g_varNumGaussians, "SlangAutoDiff.NumGaussians", 10);
	FB_DECLARE_VAR_BOOL(g_varReset, "SlangAutoDiff.Reset", false);
	FB_DECLARE_VAR_FLOAT(g_varLearningRate, "SlangAutoDiff.LearningRate", 0.100000001f);
	FB_DECLARE_VAR_FLOAT(g_varMaximumStepSize, "SlangAutoDiff.MaximumStepSize", 0.00999999978f);
	FB_DECLARE_VAR_BOOL(g_varUseBackwardAD, "SlangAutoDiff.UseBackwardAD", true);
	FB_DECLARE_VAR_BOOL(g_varQuantizeDisplay, "SlangAutoDiff.QuantizeDisplay", false);
}

namespace fb
{
ShaderProgramPipelineCache SlangAutoDiffRenderPassModule::s_pipelineCache;

template <typename T>
T Pow2GE(const T& A)
{
    float f = std::log2(float(A));
    f = std::ceilf(f);
    return (T)std::pow(2.0f, f);
}

void SlangAutoDiffRenderPassModule::addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard)
{
	// TODO: Provide this imported texture
	FrameGraphMutableResource& textureOutput = ;

	// Initialization - create resources internal to the technique
	FrameGraphMutableResource bufferData;  // first 2 floats is position, nect 2 floats are the gradient. Then 5 floats per Gaussian.
	{
		struct PassData
		{
		};

		frameGraph.addRc2CallbackPass<PassData>("SlangAutoDiff.Initialization", [&](RenderPassBuilderBase& builder, PassData& data)
		{

			// Buffer: Data
			// first 2 floats is position, nect 2 floats are the gradient. Then 5 floats per Gaussian.
			{
				uint32_t bindFlags = RenderBindFlag_ShaderResource | RenderBindFlag_UnorderedAccess;
				uint32_t memoryFlags = RenderResourceMemoryFlags_ZeroInitialize;
				RenderFormat format = RenderFormat::RenderFormat_R32_FLOAT;
				uint32_t structByteStride = 0;
				uint32_t count = ((g_varNumGaussians + 0) * 5) / 1 + 4;
				uint32_t size = count * ((structByteStride > 0) ? structByteStride : (uint32_t)getFormatBytesPerElement(convertRenderFormat(format)));

				// (Re)create the buffer as needed
				if (!m_bufferData || m_bufferData_format != format || m_bufferData_size != size)
				{
					m_bufferData_format = format;
					m_bufferData_size = size;

					RenderBufferCreateDesc desc;
					desc.name = "SlangAutoDiff.Data";
					desc.usage = RenderResourceUsage_Static;
					desc.bindFlags = bindFlags;
					desc.memoryFlags = memoryFlags;
					desc.format = format;
					desc.structByteStride = structByteStride;
					desc.size = size;
					m_bufferData = g_renderer->createBuffer(*m_arena, desc);
				}

				// import the resource
				bufferData = frameGraph.importResource(m_bufferData.get());
			}
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
		});
	}

	// Shader Constants: _Init_0CB
	{
		m_cb__Init_0CB.FrameIndex = m_varFrameIndex;
		m_cb__Init_0CB.MouseState = m_varMouseState;
		m_cb__Init_0CB.MouseStateLastFrame = m_varMouseStateLastFrame;
		m_cb__Init_0CB.NumGaussians = g_varNumGaussians;
		m_cb__Init_0CB.iResolution = { m_variResolution.x, m_variResolution.y, m_variResolution.z };
		m_cb__Init_0CB.initialized = m_varinitialized;
	}

	// Compute Shader: Initialize
	{
		struct PassData
		{
			uint dispatchX;
			uint dispatchY;
			uint dispatchZ;
			FrameGraphMutableResource bufferData;
			Struct__Init_0CB cb_Init_0CB;
		};

		frameGraph.addRc2CallbackPass<PassData>("SlangAutoDiff.Initialize",
		[&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.bufferData = builder.writeAndUpdate(bufferData, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.cb_Init_0CB = m_cb__Init_0CB;
			data.dispatchX = ((1 + 0) * 1) / 1 + 0;
			data.dispatchY = ((1 + 0) * 1) / 1 + 0;
			data.dispatchZ = ((1 + 0) * 1) / 1 + 0;
			data.dispatchX = (data.dispatchX + 1 - 1) / 1;
			data.dispatchY = (data.dispatchY + 1 - 1) / 1;
			data.dispatchZ = (data.dispatchZ + 1 - 1) / 1;
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			const ComputePipelineInfo& info = s_pipelineCache.getComputePipeline(ShaderProgram_SlangAutoDiff_Initialize_csmainCs, 0);
			commandRecorder.setPipeline(info.pipeline);

			Rc2ComputeShaderProgramBinder binder(commandRecorder, info.computeShaderInfo);

			binder.computeStage()
				.setUav(0, resources.getUav(data.bufferData))
				.setConstantBuffer(sizeof(data.cb_Init_0CB), &data.cb_Init_0CB);
			binder.bind();

			commandRecorder.dispatch(data.dispatchX, data.dispatchY, data.dispatchZ);
		});
	}

	// Shader Constants: _Render_0CB
	{
		m_cb__Render_0CB.NumGaussians = g_varNumGaussians;
		m_cb__Render_0CB.QuantizeDisplay = g_varQuantizeDisplay;
	}

	// Shader Constants: _Descend_0CB
	{
		m_cb__Descend_0CB.LearningRate = g_varLearningRate;
		m_cb__Descend_0CB.MaximumStepSize = g_varMaximumStepSize;
		m_cb__Descend_0CB.NumGaussians = g_varNumGaussians;
		m_cb__Descend_0CB.UseBackwardAD = g_varUseBackwardAD;
	}

	// Transition resources for the next action
	{
		struct PassData
		{
			FrameGraphMutableResource bufferData;
		};

		frameGraph.addRc2CallbackPass<PassData>("SlangAutoDiff.Transitions", [&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.bufferData = builder.writeAndUpdate(bufferData, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			commandRecorder.computeBarrier(resources.getRenderResource(data.bufferData));
		});
	}

	// Compute Shader: GradientDescend
	{
		struct PassData
		{
			uint dispatchX;
			uint dispatchY;
			uint dispatchZ;
			FrameGraphMutableResource bufferData;
			Struct__Descend_0CB cb_Descend_0CB;
		};

		frameGraph.addRc2CallbackPass<PassData>("SlangAutoDiff.GradientDescend",
		[&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.bufferData = builder.writeAndUpdate(bufferData, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.cb_Descend_0CB = m_cb__Descend_0CB;
			data.dispatchX = ((1 + 0) * 1) / 1 + 0;
			data.dispatchY = ((1 + 0) * 1) / 1 + 0;
			data.dispatchZ = ((1 + 0) * 1) / 1 + 0;
			data.dispatchX = (data.dispatchX + 1 - 1) / 1;
			data.dispatchY = (data.dispatchY + 1 - 1) / 1;
			data.dispatchZ = (data.dispatchZ + 1 - 1) / 1;
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			const ComputePipelineInfo& info = s_pipelineCache.getComputePipeline(ShaderProgram_SlangAutoDiff_GradientDescend_csmainCs, 0);
			commandRecorder.setPipeline(info.pipeline);

			Rc2ComputeShaderProgramBinder binder(commandRecorder, info.computeShaderInfo);

			binder.computeStage()
				.setUav(0, resources.getUav(data.bufferData))
				.setConstantBuffer(sizeof(data.cb_Descend_0CB), &data.cb_Descend_0CB);
			binder.bind();

			commandRecorder.dispatch(data.dispatchX, data.dispatchY, data.dispatchZ);
		});
	}

	// Compute Shader: Render
	{
		struct PassData
		{
			uint dispatchX;
			uint dispatchY;
			uint dispatchZ;
			FrameGraphResource bufferData;
			FrameGraphMutableResource textureOutput;
			Struct__Render_0CB cb_Render_0CB;
		};

		frameGraph.addRc2CallbackPass<PassData>("SlangAutoDiff.Render",
		[&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.bufferData = builder.read(bufferData);
			data.textureOutput = builder.writeAndUpdate(textureOutput, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.cb_Render_0CB = m_cb__Render_0CB;
			const FrameGraphTextureDesc& desc = builder.getTextureDesc(textureOutput);
			data.dispatchX = ((desc.width + 0) * 1) / 1 + 0;
			data.dispatchY = ((desc.height + 0) * 1) / 1 + 0;
			data.dispatchZ = ((desc.depth + 0) * 1) / 1 + 0;
			data.dispatchX = (data.dispatchX + 8 - 1) / 8;
			data.dispatchY = (data.dispatchY + 8 - 1) / 8;
			data.dispatchZ = (data.dispatchZ + 1 - 1) / 1;
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			const ComputePipelineInfo& info = s_pipelineCache.getComputePipeline(ShaderProgram_SlangAutoDiff_Render_csmainCs, 0);
			commandRecorder.setPipeline(info.pipeline);

			Rc2ComputeShaderProgramBinder binder(commandRecorder, info.computeShaderInfo);

			binder.computeStage()
				.setSrv(0, resources.getSrv(data.bufferData))
				.setUav(0, resources.getUav(data.textureOutput))
				.setConstantBuffer(sizeof(data.cb_Render_0CB), &data.cb_Render_0CB);
			binder.bind();

			commandRecorder.dispatch(data.dispatchX, data.dispatchY, data.dispatchZ);
		});
	}

	// Set variables
	m_varinitialized = !g_varReset;
	if(!m_varinitialized)
		Vars::setValue("SlangAutoDiff.Reset", "false");

	// bufferOutput TODO: Put this texture into the black board (Optional)
}

void SlangAutoDiffRenderPassModule::onInit(const InitParams& params)
{
	m_arena = params.memoryArena;

	// Register with framegraph
	renderPassBookmark::registerPass(RenderPassBookmark_AfterPost,
		[this](FrameGraph& frameGraph, FrameGraphBlackboard& blackboard) { addPass(frameGraph, blackboard); });
}

SlangAutoDiffRenderPassModule::~SlangAutoDiffRenderPassModule()
{
	s_pipelineCache.clear();
}

}
