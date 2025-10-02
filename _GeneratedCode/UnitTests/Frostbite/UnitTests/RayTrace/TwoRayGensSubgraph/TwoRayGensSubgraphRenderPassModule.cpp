// (c) Electronic Arts.  All Rights Reserved.

#include "TwoRayGensSubgraphRenderPassModule.h"

#include <Engine.Base/Misc/Vars.h>
#include <Engine.Math/Math.h>
#include <Engine.Render/Core/GetTextureResource.h>
#include <Engine.Render/Core2/Rc2ShaderProgramBinder.h>
#include <Engine.Render/FrameGraph/FrameGraph.h>
#include <Engine.Render/FrameGraph/FrameGraphBookmark.h>
#include <Engine.Render/FrameGraph/FrameGraphRenderPassData.h>
#include <Engine.Render/Raytrace/RaytracingUtil.h>
#include <Engine.World.Render/FrameGraphWorldRenderer.h>
#include <Engine.World.Render/TwoRayGensSubgraph/TwoRayGensSubgraphShaderProgramsDefs.h>
#include <Engine.World.Render/Raytrace/RaytraceRenderModule.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>

// User Variables
namespace
{
	FB_DECLARE_VAR_VEC3(g_varhitColor, "TwoRayGensSubgraph.hitColor", fb::math::Vector3(0.f, 1.f, 0.f));
	FB_DECLARE_VAR_VEC3(g_varmissColor, "TwoRayGensSubgraph.missColor", fb::math::Vector3(1.f, 0.f, 0.f));
}

namespace fb
{

void TwoRayGensSubgraphRenderPassModule::onRegisterActiveWorlds(FrameGraphBlackboard& blackboard)
{
	registerActiveWorld(RaytraceWorldType::RaytraceWorldType_TwoRayGensSubgraph_A_DoRT1_World);
	registerActiveWorld(RaytraceWorldType::RaytraceWorldType_TwoRayGensSubgraph_B_DoRT2_World);
}

ShaderProgramPipelineCache TwoRayGensSubgraphRenderPassModule::s_pipelineCache;

template <typename T>
T Pow2GE(const T& A)
{
    float f = std::log2(float(A));
    f = std::ceilf(f);
    return (T)std::pow(2.0f, f);
}

void TwoRayGensSubgraphRenderPassModule::addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard)
{
	// TODO: Provide this imported texture
	FrameGraphResource& textureBlueChannel = ;
	// RtTrace objects can only be submitted once per frame, but this function is called for multiple
	// viewports each frame. For now we deal with it by only running it for root view index 0 but can
	// improve it later.
	auto& paramIO = blackboard.get<WorldRendererParamIO>();
	const WorldRendererRootView* rootView = paramIO.rootView;
	if (!rootView->viewInfo->isMainView || rootView->rootView.viewType != WorldViewType_Root)
		return;

	auto& rtBlackboardData = blackboard.get<RaytraceSetupBlackboardData>();


	// Initialization - create resources internal to the technique
	FrameGraphMutableResource textureTexture;  // The texture that is rendered to
	{
		struct PassData
		{
		};

		frameGraph.addRc2CallbackPass<PassData>("TwoRayGensSubgraph.Initialization", [&](RenderPassBuilderBase& builder, PassData& data)
		{

			// Texture: Texture
			// The texture that is rendered to
			{
				TextureType type = TextureType_2d;
				u32 bindFlags = RenderBindFlag_ShaderResource | RenderBindFlag_UnorderedAccess;
				RenderFormat format = RenderFormat_R8G8B8A8_UNORM;
				uint width = ((1 + 0) * 256) / 1 + 0;
				uint height = ((1 + 0) * 256) / 1 + 0;
				uint depth = ((1 + 0) * 1) / 1 + 0;

				FrameGraphTextureDesc desc;
				desc.name = "TwoRayGensSubgraph.Texture";
				desc.usage = RenderResourceUsage_Static;
				desc.initialState = FrameGraphTextureDesc::InitialState_Undefined;
				desc.type = type;
				desc.bindFlags = bindFlags;
				desc.format = format;
				desc.width = width;
				desc.height = height;
				desc.depth = depth;
				textureTexture = builder.createTexture(desc, FrameGraphResourceUsage_UnorderedAccess);
			}
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
		});
	}

	// Shader Constants: _A_TwoRayGens1CB
	{
		m_cb__A_TwoRayGens1CB.cameraPos = { m_varcameraPos.x, m_varcameraPos.y, m_varcameraPos.z };
		m_cb__A_TwoRayGens1CB.clipToWorld = m_varclipToWorld;
		m_cb__A_TwoRayGens1CB.depthNearPlane = m_vardepthNearPlane;
	}

	// Transition resources for the next action
	{
		struct PassData
		{
			FrameGraphMutableResource textureTexture;
		};

		frameGraph.addRc2CallbackPass<PassData>("TwoRayGensSubgraph.Transitions", [&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureTexture = builder.writeAndUpdate(textureTexture, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			commandRecorder.computeBarrier(resources.getRenderResource(data.textureTexture));
		});
	}

	// Ray Shader: A_DoRT1
	{
		FB_FATAL_ASSERT(getActiveWorldHandle(RaytraceWorldType_TwoRayGensSubgraph_A_DoRT1_World) != getInvalidWorldHandle());

		struct PassData
		{
			RtRef<RtTrace> trace;
			RtHandle<RtTable> table;
			CommandListHandle rtClHandle;
			FrameGraphResource rtSystemUpdateDependency;
			uint dispatchX;
			uint dispatchY;
			uint dispatchZ;
			FrameGraphMutableResource textureTexture;
			Struct__A_TwoRayGens1CB cb_A_TwoRayGens1CB;
		};

		frameGraph.addRc2CallbackDispatchPass<PassData>("TwoRayGensSubgraph.A_DoRT1", [&](RenderPassBuilderBase& builder, PassData& data)
		{

			data.textureTexture = builder.writeAndUpdate(textureTexture, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.cb_A_TwoRayGens1CB = m_cb__A_TwoRayGens1CB;
			const FrameGraphTextureDesc& desc = builder.getTextureDesc(textureTexture);
			data.dispatchX = ((desc.width + 0) * 1) / 1 + 0;
			data.dispatchY = ((desc.height + 0) * 1) / 1 + 0;
			data.dispatchZ = ((desc.depth + 0) * 1) / 1 + 0;

			data.rtSystemUpdateDependency = builder.read(rtBlackboardData.systemUpdateDependency);

			data.trace = m_rtTrace_A_DoRT1;
			data.table = m_rayGenTable_A_DoRT1;
		},
		[=](PassData& data, const Rc2RenderPassResources& resources, IFrameGraphPrepareContext* prepareContext)
		{
			CommandListDesc clDesc;
			RtSystem::getCommandListDesc("TwoRayGensSubgraph.A_DoRT1", clDesc);
			IProcessQueue* processQueue = g_renderer->getCore2Process();
			data.rtClHandle = processQueue->reserveCommandList(clDesc);

			m_rayGenCB_A_DoRT1->update(sizeof(data.cb_A_TwoRayGens1CB), &data.cb_A_TwoRayGens1CB);

			m_rayGenTable_A_DoRT1->setRwResources({
				resources.getIRenderResource(data.textureTexture)->getAsUav()
			});

			data.trace->enqueue(data.rtClHandle, {data.dispatchX, data.dispatchY, data.dispatchZ});
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			commandRecorder.callCommandList(data.rtClHandle);
		});
	}

	// Shader Constants: _B_TwoRayGens2CB
	{
		m_cb__B_TwoRayGens2CB.cameraPos = { m_varcameraPos.x, m_varcameraPos.y, m_varcameraPos.z };
		m_cb__B_TwoRayGens2CB.clipToWorld = m_varclipToWorld;
		m_cb__B_TwoRayGens2CB.depthNearPlane = m_vardepthNearPlane;
	}

	// Transition resources for the next action
	{
		struct PassData
		{
			FrameGraphMutableResource textureTexture;
		};

		frameGraph.addRc2CallbackPass<PassData>("TwoRayGensSubgraph.Transitions", [&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureTexture = builder.writeAndUpdate(textureTexture, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			commandRecorder.computeBarrier(resources.getRenderResource(data.textureTexture));
		});
	}

	// Ray Shader: B_DoRT2
	{
		FB_FATAL_ASSERT(getActiveWorldHandle(RaytraceWorldType_TwoRayGensSubgraph_B_DoRT2_World) != getInvalidWorldHandle());

		struct PassData
		{
			RtRef<RtTrace> trace;
			RtHandle<RtTable> table;
			CommandListHandle rtClHandle;
			FrameGraphResource rtSystemUpdateDependency;
			uint dispatchX;
			uint dispatchY;
			uint dispatchZ;
			FrameGraphMutableResource textureTexture;
			FrameGraphResource textureBlueChannel;
			Struct__B_TwoRayGens2CB cb_B_TwoRayGens2CB;
		};

		frameGraph.addRc2CallbackDispatchPass<PassData>("TwoRayGensSubgraph.B_DoRT2", [&](RenderPassBuilderBase& builder, PassData& data)
		{

			data.textureTexture = builder.writeAndUpdate(textureTexture, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.textureBlueChannel = builder.read(textureBlueChannel);
			data.cb_B_TwoRayGens2CB = m_cb__B_TwoRayGens2CB;
			const FrameGraphTextureDesc& desc = builder.getTextureDesc(textureTexture);
			data.dispatchX = ((desc.width + 0) * 1) / 1 + 0;
			data.dispatchY = ((desc.height + 0) * 1) / 1 + 0;
			data.dispatchZ = ((desc.depth + 0) * 1) / 1 + 0;

			data.rtSystemUpdateDependency = builder.read(rtBlackboardData.systemUpdateDependency);

			data.trace = m_rtTrace_B_DoRT2;
			data.table = m_rayGenTable_B_DoRT2;
		},
		[=](PassData& data, const Rc2RenderPassResources& resources, IFrameGraphPrepareContext* prepareContext)
		{
			CommandListDesc clDesc;
			RtSystem::getCommandListDesc("TwoRayGensSubgraph.B_DoRT2", clDesc);
			IProcessQueue* processQueue = g_renderer->getCore2Process();
			data.rtClHandle = processQueue->reserveCommandList(clDesc);

			m_rayGenCB_B_DoRT2->update(sizeof(data.cb_B_TwoRayGens2CB), &data.cb_B_TwoRayGens2CB);

			m_rayGenTable_B_DoRT2->setResources({
				resources.getIRenderResource(data.textureBlueChannel)->getAsSrv()
			});

			m_rayGenTable_B_DoRT2->setRwResources({
				resources.getIRenderResource(data.textureTexture)->getAsUav()
			});

			data.trace->enqueue(data.rtClHandle, {data.dispatchX, data.dispatchY, data.dispatchZ});
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			commandRecorder.callCommandList(data.rtClHandle);
		});
	}

	// bufferTexture TODO: Put this texture into the black board (Optional)
	// bufferBlueChannel TODO: Put this texture into the black board (Optional)
}

void TwoRayGensSubgraphRenderPassModule::onInit(const InitParams& params)
{
	m_arena = params.memoryArena;

	registerRaytraceRenderPassModule(this);

	RaytraceData& rtData = getRaytraceRenderModule()->getRaytraceData();

	// Ray Shader: A_DoRT1
	{
		if (!rtData.worldInfos[RaytraceWorldType_TwoRayGensSubgraph_A_DoRT1_World]->world->hasDefaultHitGroup())
		{
			RtBindingDesc hitGroupDesc;
			hitGroupDesc.shader = RtUtil::createShaderFromProgram(rtData.system, ShaderProgram_TwoRayGensSubgraph_A_TwoRayGensClosestHit1_HG);
			rtData.worldInfos[RaytraceWorldType_TwoRayGensSubgraph_A_DoRT1_World]->world->setDefaultHitGroup(RtBinding::create(rtData.system, hitGroupDesc));
		}

		m_rayGenTable_A_DoRT1 = RtTable::create(rtData.system, rtData.system->getRayGenTableDesc(0));
		m_rayGenCB_A_DoRT1 = RtBuffer::create(rtData.system, RtBufferDesc{});

		RtBindingDesc rayGenDesc;
		rayGenDesc.shader = RtUtil::createShaderFromProgram(rtData.system, ShaderProgram_TwoRayGensSubgraph_A_TwoRayGens1_RG);
		rayGenDesc.constantBuffers.push_back(m_rayGenCB_A_DoRT1);
		rayGenDesc.tables.push_back(m_rayGenTable_A_DoRT1);
		RtHandle<RtBinding> rayGen = RtBinding::create(rtData.system, rayGenDesc);

		RtTraceDesc traceDesc;
		traceDesc.worlds.emplace_back(rtData.worldInfos[RaytraceWorldType_TwoRayGensSubgraph_A_DoRT1_World]->world, 0);
		traceDesc.raygenBinding = rayGen;

		RtBindingDesc missDesc;
		missDesc.shader = RtUtil::createShaderFromProgram(rtData.system, ShaderProgram_TwoRayGensSubgraph_A_TwoRayGensMiss1_MS);
		traceDesc.missBindings.push_back(RtBinding::create(rtData.system, missDesc));

		missDesc.shader = RtUtil::createShaderFromProgram(rtData.system, ShaderProgram_TwoRayGensSubgraph_B_TwoRayGensMiss2A_MS);
		traceDesc.missBindings.push_back(RtBinding::create(rtData.system, missDesc));

		missDesc.shader = RtUtil::createShaderFromProgram(rtData.system, ShaderProgram_TwoRayGensSubgraph_B_TwoRayGensMiss2B_MS);
		traceDesc.missBindings.push_back(RtBinding::create(rtData.system, missDesc));

		m_rtTrace_A_DoRT1 = RtTrace::create(rtData.system, traceDesc);
	}

	// Ray Shader: B_DoRT2
	{
		if (!rtData.worldInfos[RaytraceWorldType_TwoRayGensSubgraph_B_DoRT2_World]->world->hasDefaultHitGroup())
		{
			RtBindingDesc hitGroupDesc;
			hitGroupDesc.shader = RtUtil::createShaderFromProgram(rtData.system, ShaderProgram_TwoRayGensSubgraph_B_TwoRayGensClosestHit2_HG);
			rtData.worldInfos[RaytraceWorldType_TwoRayGensSubgraph_B_DoRT2_World]->world->setDefaultHitGroup(RtBinding::create(rtData.system, hitGroupDesc));
		}

		m_rayGenTable_B_DoRT2 = RtTable::create(rtData.system, rtData.system->getRayGenTableDesc(0));
		m_rayGenCB_B_DoRT2 = RtBuffer::create(rtData.system, RtBufferDesc{});

		RtBindingDesc rayGenDesc;
		rayGenDesc.shader = RtUtil::createShaderFromProgram(rtData.system, ShaderProgram_TwoRayGensSubgraph_B_TwoRayGens2_RG);
		rayGenDesc.constantBuffers.push_back(m_rayGenCB_B_DoRT2);
		rayGenDesc.tables.push_back(m_rayGenTable_B_DoRT2);
		RtHandle<RtBinding> rayGen = RtBinding::create(rtData.system, rayGenDesc);

		RtTraceDesc traceDesc;
		traceDesc.worlds.emplace_back(rtData.worldInfos[RaytraceWorldType_TwoRayGensSubgraph_B_DoRT2_World]->world, 0);
		traceDesc.raygenBinding = rayGen;

		RtBindingDesc missDesc;
		missDesc.shader = RtUtil::createShaderFromProgram(rtData.system, ShaderProgram_TwoRayGensSubgraph_A_TwoRayGensMiss1_MS);
		traceDesc.missBindings.push_back(RtBinding::create(rtData.system, missDesc));

		missDesc.shader = RtUtil::createShaderFromProgram(rtData.system, ShaderProgram_TwoRayGensSubgraph_B_TwoRayGensMiss2A_MS);
		traceDesc.missBindings.push_back(RtBinding::create(rtData.system, missDesc));

		missDesc.shader = RtUtil::createShaderFromProgram(rtData.system, ShaderProgram_TwoRayGensSubgraph_B_TwoRayGensMiss2B_MS);
		traceDesc.missBindings.push_back(RtBinding::create(rtData.system, missDesc));

		m_rtTrace_B_DoRT2 = RtTrace::create(rtData.system, traceDesc);
	}

	// Register with framegraph
	renderPassBookmark::registerPass(RenderPassBookmark_AfterPost,
		[this](FrameGraph& frameGraph, FrameGraphBlackboard& blackboard) { addPass(frameGraph, blackboard); });
}

TwoRayGensSubgraphRenderPassModule::~TwoRayGensSubgraphRenderPassModule()
{
	s_pipelineCache.clear();
}

}
