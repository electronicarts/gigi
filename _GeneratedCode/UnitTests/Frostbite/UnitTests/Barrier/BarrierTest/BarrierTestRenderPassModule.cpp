// (c) Electronic Arts.  All Rights Reserved.

#include "BarrierTestRenderPassModule.h"

#include <Engine.Base/Misc/Vars.h>
#include <Engine.Math/Math.h>
#include <Engine.Render/Core/GetTextureResource.h>
#include <Engine.Render/Core2/Rc2ShaderProgramBinder.h>
#include <Engine.Render/FrameGraph/FrameGraph.h>
#include <Engine.Render/FrameGraph/FrameGraphBookmark.h>
#include <Engine.Render/FrameGraph/FrameGraphRenderPassData.h>
#include <Engine.Render/Raytrace/RaytracingUtil.h>
#include <Engine.World.Render/FrameGraphWorldRenderer.h>
#include <Engine.World.Render/BarrierTest/BarrierTestShaderProgramsDefs.h>
#include <Engine.World.Render/Raytrace/RaytraceRenderModule.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>

namespace fb
{
ShaderProgramPipelineCache BarrierTestRenderPassModule::s_pipelineCache;

template <typename T>
T Pow2GE(const T& A)
{
    float f = std::log2(float(A));
    f = std::ceilf(f);
    return (T)std::pow(2.0f, f);
}

void BarrierTestRenderPassModule::addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard)
{

	// Initialization - create resources internal to the technique
	FrameGraphMutableResource textureOutput;
	{
		struct PassData
		{
		};

		frameGraph.addRc2CallbackPass<PassData>("BarrierTest.Initialization", [&](RenderPassBuilderBase& builder, PassData& data)
		{

			// Texture: Output
			{
				TextureType type = TextureType_2d;
				u32 bindFlags = RenderBindFlag_ShaderResource | RenderBindFlag_UnorderedAccess;
				RenderFormat format = RenderFormat_R8G8B8A8_UNORM;
				uint width = ((1 + 0) * 512) / 1 + 0;
				uint height = ((1 + 0) * 512) / 1 + 0;
				uint depth = ((1 + 0) * 1) / 1 + 0;

				FrameGraphTextureDesc desc;
				desc.name = "BarrierTest.Output";
				desc.usage = RenderResourceUsage_Static;
				desc.initialState = FrameGraphTextureDesc::InitialState_Undefined;
				desc.type = type;
				desc.bindFlags = bindFlags;
				desc.format = format;
				desc.width = width;
				desc.height = height;
				desc.depth = depth;
				textureOutput = builder.createTexture(desc, FrameGraphResourceUsage_UnorderedAccess);
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
			FrameGraphMutableResource textureOutput;
		};

		frameGraph.addRc2CallbackPass<PassData>("BarrierTest.Transitions", [&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureOutput = builder.writeAndUpdate(textureOutput, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			commandRecorder.computeBarrier(resources.getRenderResource(data.textureOutput));
		});
	}

	// Compute Shader: Draw_Left
	{
		struct PassData
		{
			uint dispatchX;
			uint dispatchY;
			uint dispatchZ;
			FrameGraphMutableResource textureOutput;
		};

		frameGraph.addRc2CallbackPass<PassData>("BarrierTest.Draw_Left",
		[&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureOutput = builder.writeAndUpdate(textureOutput, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			const FrameGraphTextureDesc& desc = builder.getTextureDesc(textureOutput);
			data.dispatchX = ((desc.width + 0) * 1) / 2 + 0;
			data.dispatchY = ((desc.height + 0) * 1) / 1 + 0;
			data.dispatchZ = ((desc.depth + 0) * 1) / 1 + 0;
			data.dispatchX = (data.dispatchX + 8 - 1) / 8;
			data.dispatchY = (data.dispatchY + 8 - 1) / 8;
			data.dispatchZ = (data.dispatchZ + 1 - 1) / 1;
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			const ComputePipelineInfo& info = s_pipelineCache.getComputePipeline(ShaderProgram_BarrierTest_Draw_Left_csmainCs, 0);
			commandRecorder.setPipeline(info.pipeline);

			Rc2ComputeShaderProgramBinder binder(commandRecorder, info.computeShaderInfo);

			binder.computeStage()
				.setUav(0, resources.getUav(data.textureOutput));
			binder.bind();

			commandRecorder.dispatch(data.dispatchX, data.dispatchY, data.dispatchZ);
		});
	}

	// Transition resources for the next action
	{
		struct PassData
		{
			FrameGraphMutableResource textureOutput;
		};

		frameGraph.addRc2CallbackPass<PassData>("BarrierTest.Transitions", [&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureOutput = builder.writeAndUpdate(textureOutput, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			commandRecorder.computeBarrier(resources.getRenderResource(data.textureOutput));
		});
	}

	// Compute Shader: Draw_Right
	{
		struct PassData
		{
			uint dispatchX;
			uint dispatchY;
			uint dispatchZ;
			FrameGraphMutableResource textureOutput;
		};

		frameGraph.addRc2CallbackPass<PassData>("BarrierTest.Draw_Right",
		[&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureOutput = builder.writeAndUpdate(textureOutput, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			const FrameGraphTextureDesc& desc = builder.getTextureDesc(textureOutput);
			data.dispatchX = ((desc.width + 0) * 1) / 2 + 0;
			data.dispatchY = ((desc.height + 0) * 1) / 1 + 0;
			data.dispatchZ = ((desc.depth + 0) * 1) / 1 + 0;
			data.dispatchX = (data.dispatchX + 8 - 1) / 8;
			data.dispatchY = (data.dispatchY + 8 - 1) / 8;
			data.dispatchZ = (data.dispatchZ + 1 - 1) / 1;
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			const ComputePipelineInfo& info = s_pipelineCache.getComputePipeline(ShaderProgram_BarrierTest_Draw_Right_csmainCs, 0);
			commandRecorder.setPipeline(info.pipeline);

			Rc2ComputeShaderProgramBinder binder(commandRecorder, info.computeShaderInfo);

			binder.computeStage()
				.setUav(0, resources.getUav(data.textureOutput));
			binder.bind();

			commandRecorder.dispatch(data.dispatchX, data.dispatchY, data.dispatchZ);
		});
	}

	// Transition resources for the next action
	{
		struct PassData
		{
			FrameGraphMutableResource textureOutput;
		};

		frameGraph.addRc2CallbackPass<PassData>("BarrierTest.Transitions", [&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureOutput = builder.writeAndUpdate(textureOutput, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			commandRecorder.computeBarrier(resources.getRenderResource(data.textureOutput));
		});
	}

	// Compute Shader: After
	{
		struct PassData
		{
			uint dispatchX;
			uint dispatchY;
			uint dispatchZ;
			FrameGraphMutableResource textureOutput;
		};

		frameGraph.addRc2CallbackPass<PassData>("BarrierTest.After",
		[&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureOutput = builder.writeAndUpdate(textureOutput, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
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
			const ComputePipelineInfo& info = s_pipelineCache.getComputePipeline(ShaderProgram_BarrierTest_After_csmainCs, 0);
			commandRecorder.setPipeline(info.pipeline);

			Rc2ComputeShaderProgramBinder binder(commandRecorder, info.computeShaderInfo);

			binder.computeStage()
				.setUav(0, resources.getUav(data.textureOutput));
			binder.bind();

			commandRecorder.dispatch(data.dispatchX, data.dispatchY, data.dispatchZ);
		});
	}

	// bufferOutput TODO: Put this texture into the black board (Optional)
}

void BarrierTestRenderPassModule::onInit(const InitParams& params)
{
	m_arena = params.memoryArena;

	// Register with framegraph
	renderPassBookmark::registerPass(RenderPassBookmark_AfterPost,
		[this](FrameGraph& frameGraph, FrameGraphBlackboard& blackboard) { addPass(frameGraph, blackboard); });
}

BarrierTestRenderPassModule::~BarrierTestRenderPassModule()
{
	s_pipelineCache.clear();
}

}
