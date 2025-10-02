// (c) Electronic Arts.  All Rights Reserved.

#include "VariableAliasesRenderPassModule.h"

#include <Engine.Base/Misc/Vars.h>
#include <Engine.Math/Math.h>
#include <Engine.Render/Core/GetTextureResource.h>
#include <Engine.Render/Core2/Rc2ShaderProgramBinder.h>
#include <Engine.Render/FrameGraph/FrameGraph.h>
#include <Engine.Render/FrameGraph/FrameGraphBookmark.h>
#include <Engine.Render/FrameGraph/FrameGraphRenderPassData.h>
#include <Engine.Render/Raytrace/RaytracingUtil.h>
#include <Engine.World.Render/FrameGraphWorldRenderer.h>
#include <Engine.World.Render/VariableAliases/VariableAliasesShaderProgramsDefs.h>
#include <Engine.World.Render/Raytrace/RaytraceRenderModule.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>

// User Variables
namespace
{
	FB_DECLARE_VAR_VEC3(g_varRenderSize, "VariableAliases.RenderSize", fb::math::Vector3(32, 32, 0.0f));
	FB_DECLARE_VAR_INT(g_varNode1Channel, "VariableAliases.Node1Channel", 0);
	FB_DECLARE_VAR_FLOAT(g_varNode1Value, "VariableAliases.Node1Value", 0.25f);
	FB_DECLARE_VAR_INT(g_varNode2Channel, "VariableAliases.Node2Channel", 1);
	FB_DECLARE_VAR_FLOAT(g_varNode2Value, "VariableAliases.Node2Value", 0.5f);
	FB_DECLARE_VAR_INT(g_varNode3Channel, "VariableAliases.Node3Channel", 2);
	FB_DECLARE_VAR_FLOAT(g_varNode3Value, "VariableAliases.Node3Value", 0.75f);
	FB_DECLARE_VAR_FLOAT(g_varUnusedFloat, "VariableAliases.UnusedFloat", 0.f);  // This is for the unused alias in the shader
}

namespace fb
{
ShaderProgramPipelineCache VariableAliasesRenderPassModule::s_pipelineCache;

template <typename T>
T Pow2GE(const T& A)
{
    float f = std::log2(float(A));
    f = std::ceilf(f);
    return (T)std::pow(2.0f, f);
}

void VariableAliasesRenderPassModule::addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard)
{

	// Initialization - create resources internal to the technique
	FrameGraphMutableResource textureColor;
	{
		struct PassData
		{
		};

		frameGraph.addRc2CallbackPass<PassData>("VariableAliases.Initialization", [&](RenderPassBuilderBase& builder, PassData& data)
		{

			// Texture: Color
			{
				TextureType type = TextureType_2d;
				u32 bindFlags = RenderBindFlag_ShaderResource | RenderBindFlag_UnorderedAccess;
				RenderFormat format = RenderFormat_R8G8B8A8_UNORM;
				uint width = ((g_varRenderSize[0] + 0) * 1) / 1 + 0;
				uint height = ((g_varRenderSize[1] + 0) * 1) / 1 + 0;
				uint depth = ((1 + 0) * 1) / 1 + 0;

				FrameGraphTextureDesc desc;
				desc.name = "VariableAliases.Color";
				desc.usage = RenderResourceUsage_Static;
				desc.initialState = FrameGraphTextureDesc::InitialState_Undefined;
				desc.type = type;
				desc.bindFlags = bindFlags;
				desc.format = format;
				desc.width = width;
				desc.height = height;
				desc.depth = depth;
				textureColor = builder.createTexture(desc, FrameGraphResourceUsage_UnorderedAccess);
			}
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
		});
	}

	// Shader Constants: _SetChannel_0CB_0
	{
		m_cb__SetChannel_0CB_0._alias_Channel = g_varNode1Channel;
		m_cb__SetChannel_0CB_0._alias_Value = g_varNode1Value;
	}

	// Transition resources for the next action
	{
		struct PassData
		{
			FrameGraphMutableResource textureColor;
		};

		frameGraph.addRc2CallbackPass<PassData>("VariableAliases.Transitions", [&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureColor = builder.writeAndUpdate(textureColor, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			commandRecorder.computeBarrier(resources.getRenderResource(data.textureColor));
		});
	}

	// Compute Shader: Set_Red
	{
		struct PassData
		{
			uint dispatchX;
			uint dispatchY;
			uint dispatchZ;
			FrameGraphMutableResource textureColor;
			Struct__SetChannel_0CB cb_SetChannel_0CB_0;
		};

		frameGraph.addRc2CallbackPass<PassData>("VariableAliases.Set_Red",
		[&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureColor = builder.writeAndUpdate(textureColor, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.cb_SetChannel_0CB_0 = m_cb__SetChannel_0CB_0;
			const FrameGraphTextureDesc& desc = builder.getTextureDesc(textureColor);
			data.dispatchX = ((desc.width + 0) * 1) / 1 + 0;
			data.dispatchY = ((desc.height + 0) * 1) / 1 + 0;
			data.dispatchZ = ((desc.depth + 0) * 1) / 1 + 0;
			data.dispatchX = (data.dispatchX + 8 - 1) / 8;
			data.dispatchY = (data.dispatchY + 8 - 1) / 8;
			data.dispatchZ = (data.dispatchZ + 1 - 1) / 1;
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			const ComputePipelineInfo& info = s_pipelineCache.getComputePipeline(ShaderProgram_VariableAliases_Set_Red_mainCs, 0);
			commandRecorder.setPipeline(info.pipeline);

			Rc2ComputeShaderProgramBinder binder(commandRecorder, info.computeShaderInfo);

			binder.computeStage()
				.setUav(0, resources.getUav(data.textureColor))
				.setConstantBuffer(sizeof(data.cb_SetChannel_0CB_0), &data.cb_SetChannel_0CB_0);
			binder.bind();

			commandRecorder.dispatch(data.dispatchX, data.dispatchY, data.dispatchZ);
		});
	}

	// Shader Constants: _SetChannel_0CB_1
	{
		m_cb__SetChannel_0CB_1._alias_Channel = g_varNode2Channel;
		m_cb__SetChannel_0CB_1._alias_Value = g_varNode2Value;
	}

	// Transition resources for the next action
	{
		struct PassData
		{
			FrameGraphMutableResource textureColor;
		};

		frameGraph.addRc2CallbackPass<PassData>("VariableAliases.Transitions", [&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureColor = builder.writeAndUpdate(textureColor, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			commandRecorder.computeBarrier(resources.getRenderResource(data.textureColor));
		});
	}

	// Compute Shader: Set_Green
	{
		struct PassData
		{
			uint dispatchX;
			uint dispatchY;
			uint dispatchZ;
			FrameGraphMutableResource textureColor;
			Struct__SetChannel_0CB cb_SetChannel_0CB_1;
		};

		frameGraph.addRc2CallbackPass<PassData>("VariableAliases.Set_Green",
		[&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureColor = builder.writeAndUpdate(textureColor, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.cb_SetChannel_0CB_1 = m_cb__SetChannel_0CB_1;
			const FrameGraphTextureDesc& desc = builder.getTextureDesc(textureColor);
			data.dispatchX = ((desc.width + 0) * 1) / 1 + 0;
			data.dispatchY = ((desc.height + 0) * 1) / 1 + 0;
			data.dispatchZ = ((desc.depth + 0) * 1) / 1 + 0;
			data.dispatchX = (data.dispatchX + 8 - 1) / 8;
			data.dispatchY = (data.dispatchY + 8 - 1) / 8;
			data.dispatchZ = (data.dispatchZ + 1 - 1) / 1;
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			const ComputePipelineInfo& info = s_pipelineCache.getComputePipeline(ShaderProgram_VariableAliases_Set_Green_mainCs, 0);
			commandRecorder.setPipeline(info.pipeline);

			Rc2ComputeShaderProgramBinder binder(commandRecorder, info.computeShaderInfo);

			binder.computeStage()
				.setUav(0, resources.getUav(data.textureColor))
				.setConstantBuffer(sizeof(data.cb_SetChannel_0CB_1), &data.cb_SetChannel_0CB_1);
			binder.bind();

			commandRecorder.dispatch(data.dispatchX, data.dispatchY, data.dispatchZ);
		});
	}

	// Shader Constants: _SetChannel_1CB_0
	{
		m_cb__SetChannel_1CB_0._alias_Channel = g_varNode3Channel;
		m_cb__SetChannel_1CB_0._alias_Value = g_varNode3Value;
	}

	// Transition resources for the next action
	{
		struct PassData
		{
			FrameGraphMutableResource textureColor;
		};

		frameGraph.addRc2CallbackPass<PassData>("VariableAliases.Transitions", [&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureColor = builder.writeAndUpdate(textureColor, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			commandRecorder.computeBarrier(resources.getRenderResource(data.textureColor));
		});
	}

	// Compute Shader: Set_Blue
	{
		struct PassData
		{
			uint dispatchX;
			uint dispatchY;
			uint dispatchZ;
			FrameGraphMutableResource textureColor;
			Struct__SetChannel_1CB cb_SetChannel_1CB_0;
		};

		frameGraph.addRc2CallbackPass<PassData>("VariableAliases.Set_Blue",
		[&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureColor = builder.writeAndUpdate(textureColor, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.cb_SetChannel_1CB_0 = m_cb__SetChannel_1CB_0;
			const FrameGraphTextureDesc& desc = builder.getTextureDesc(textureColor);
			data.dispatchX = ((desc.width + 0) * 1) / 1 + 0;
			data.dispatchY = ((desc.height + 0) * 1) / 1 + 0;
			data.dispatchZ = ((desc.depth + 0) * 1) / 1 + 0;
			data.dispatchX = (data.dispatchX + 8 - 1) / 8;
			data.dispatchY = (data.dispatchY + 8 - 1) / 8;
			data.dispatchZ = (data.dispatchZ + 1 - 1) / 1;
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			const ComputePipelineInfo& info = s_pipelineCache.getComputePipeline(ShaderProgram_VariableAliases_Set_Blue_mainCs, 0);
			commandRecorder.setPipeline(info.pipeline);

			Rc2ComputeShaderProgramBinder binder(commandRecorder, info.computeShaderInfo);

			binder.computeStage()
				.setUav(0, resources.getUav(data.textureColor))
				.setConstantBuffer(sizeof(data.cb_SetChannel_1CB_0), &data.cb_SetChannel_1CB_0);
			binder.bind();

			commandRecorder.dispatch(data.dispatchX, data.dispatchY, data.dispatchZ);
		});
	}

	// bufferColor TODO: Put this texture into the black board (Optional)
}

void VariableAliasesRenderPassModule::onInit(const InitParams& params)
{
	m_arena = params.memoryArena;

	// Register with framegraph
	renderPassBookmark::registerPass(RenderPassBookmark_AfterPost,
		[this](FrameGraph& frameGraph, FrameGraphBlackboard& blackboard) { addPass(frameGraph, blackboard); });
}

VariableAliasesRenderPassModule::~VariableAliasesRenderPassModule()
{
	s_pipelineCache.clear();
}

}
