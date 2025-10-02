// (c) Electronic Arts.  All Rights Reserved.

#include "simpleRTRenderPassModule.h"

#include <Engine.Base/Misc/Vars.h>
#include <Engine.Math/Math.h>
#include <Engine.Render/Core/GetTextureResource.h>
#include <Engine.Render/Core2/Rc2ShaderProgramBinder.h>
#include <Engine.Render/FrameGraph/FrameGraph.h>
#include <Engine.Render/FrameGraph/FrameGraphBookmark.h>
#include <Engine.Render/FrameGraph/FrameGraphRenderPassData.h>
#include <Engine.Render/Raytrace/RaytracingUtil.h>
#include <Engine.World.Render/FrameGraphWorldRenderer.h>
#include <Engine.World.Render/simpleRT/simpleRTShaderProgramsDefs.h>
#include <Engine.World.Render/Raytrace/RaytraceRenderModule.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>

// User Variables
namespace
{
	FB_DECLARE_VAR_BOOL(g_varenabled, "simpleRT.enabled", false);
	FB_DECLARE_VAR_VEC3(g_varhitColor, "simpleRT.hitColor", fb::math::Vector3(0.f, 1.f, 0.f));
	FB_DECLARE_VAR_VEC3(g_varmissColor, "simpleRT.missColor", fb::math::Vector3(1.f, 0.f, 0.f));
}

namespace fb
{

void simpleRTRenderPassModule::onRegisterActiveWorlds(FrameGraphBlackboard& blackboard)
{
	registerActiveWorld(RaytraceWorldType::RaytraceWorldType_simpleRT_DoRT_World);
}

ShaderProgramPipelineCache simpleRTRenderPassModule::s_pipelineCache;

template <typename T>
T Pow2GE(const T& A)
{
    float f = std::log2(float(A));
    f = std::ceilf(f);
    return (T)std::pow(2.0f, f);
}

void simpleRTRenderPassModule::addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard)
{
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

		frameGraph.addRc2CallbackPass<PassData>("simpleRT.Initialization", [&](RenderPassBuilderBase& builder, PassData& data)
		{

			// Texture: Texture
			// The texture that is rendered to
			{
				TextureType type = TextureType_2d;
				u32 bindFlags = RenderBindFlag_ShaderResource | RenderBindFlag_UnorderedAccess;
				RenderFormat format = RenderFormat_R8G8B8A8_UNORM;
				uint width = ((1 + 0) * 1280) / 1 + 0;
				uint height = ((1 + 0) * 720) / 1 + 0;
				uint depth = ((1 + 0) * 1) / 1 + 0;

				FrameGraphTextureDesc desc;
				desc.name = "simpleRT.Texture";
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

	// Shader Constants: _SimpleRTRayGenCB
	{
		m_cb__SimpleRTRayGenCB.cameraPos = { m_varcameraPos.x, m_varcameraPos.y, m_varcameraPos.z };
		m_cb__SimpleRTRayGenCB.clipToWorld = m_varclipToWorld;
		m_cb__SimpleRTRayGenCB.depthNearPlane = m_vardepthNearPlane;
		m_cb__SimpleRTRayGenCB.hitColor = { (float)g_varhitColor.x, (float)g_varhitColor.y, (float)g_varhitColor.z };
		m_cb__SimpleRTRayGenCB.missColor = { (float)g_varmissColor.x, (float)g_varmissColor.y, (float)g_varmissColor.z };
	}

	// Transition resources for the next action
	{
		struct PassData
		{
			FrameGraphMutableResource textureTexture;
		};

		frameGraph.addRc2CallbackPass<PassData>("simpleRT.Transitions", [&](RenderPassBuilderBase& builder, PassData& data)
		{
			data.textureTexture = builder.writeAndUpdate(textureTexture, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
		},
		[=](const PassData& data, const Rc2RenderPassResources& resources, CommandRecorder& commandRecorder)
		{
			commandRecorder.computeBarrier(resources.getRenderResource(data.textureTexture));
		});
	}

	// Ray Shader: DoRT
	if(g_varenabled)
	{
		FB_FATAL_ASSERT(getActiveWorldHandle(RaytraceWorldType_simpleRT_DoRT_World) != getInvalidWorldHandle());

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
			Struct__SimpleRTRayGenCB cb_SimpleRTRayGenCB;
		};

		frameGraph.addRc2CallbackDispatchPass<PassData>("simpleRT.DoRT", [&](RenderPassBuilderBase& builder, PassData& data)
		{

			data.textureTexture = builder.writeAndUpdate(textureTexture, FrameGraphResourceUsage::FrameGraphResourceUsage_UnorderedAccess);
			data.cb_SimpleRTRayGenCB = m_cb__SimpleRTRayGenCB;
			const FrameGraphTextureDesc& desc = builder.getTextureDesc(textureTexture);
			data.dispatchX = ((desc.width + 0) * 1) / 1 + 0;
			data.dispatchY = ((desc.height + 0) * 1) / 1 + 0;
			data.dispatchZ = ((desc.depth + 0) * 1) / 1 + 0;

			data.rtSystemUpdateDependency = builder.read(rtBlackboardData.systemUpdateDependency);

			data.trace = m_rtTrace_DoRT;
			data.table = m_rayGenTable_DoRT;
		},
		[=](PassData& data, const Rc2RenderPassResources& resources, IFrameGraphPrepareContext* prepareContext)
		{
			CommandListDesc clDesc;
			RtSystem::getCommandListDesc("simpleRT.DoRT", clDesc);
			IProcessQueue* processQueue = g_renderer->getCore2Process();
			data.rtClHandle = processQueue->reserveCommandList(clDesc);

			m_rayGenCB_DoRT->update(sizeof(data.cb_SimpleRTRayGenCB), &data.cb_SimpleRTRayGenCB);

			m_rayGenTable_DoRT->setRwResources({
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
}

void simpleRTRenderPassModule::onInit(const InitParams& params)
{
	m_arena = params.memoryArena;

	registerRaytraceRenderPassModule(this);

	RaytraceData& rtData = getRaytraceRenderModule()->getRaytraceData();

	// Ray Shader: DoRT
	{
		if (!rtData.worldInfos[RaytraceWorldType_simpleRT_DoRT_World]->world->hasDefaultHitGroup())
		{
			RtBindingDesc hitGroupDesc;
			hitGroupDesc.shader = RtUtil::createShaderFromProgram(rtData.system, ShaderProgram_simpleRT_SimpleRTClosestHit_HG);
			rtData.worldInfos[RaytraceWorldType_simpleRT_DoRT_World]->world->setDefaultHitGroup(RtBinding::create(rtData.system, hitGroupDesc));
		}

		m_rayGenTable_DoRT = RtTable::create(rtData.system, rtData.system->getRayGenTableDesc(0));
		m_rayGenCB_DoRT = RtBuffer::create(rtData.system, RtBufferDesc{});

		RtBindingDesc rayGenDesc;
		rayGenDesc.shader = RtUtil::createShaderFromProgram(rtData.system, ShaderProgram_simpleRT_SimpleRTRayGen_RG);
		rayGenDesc.constantBuffers.push_back(m_rayGenCB_DoRT);
		rayGenDesc.tables.push_back(m_rayGenTable_DoRT);
		RtHandle<RtBinding> rayGen = RtBinding::create(rtData.system, rayGenDesc);

		RtTraceDesc traceDesc;
		traceDesc.worlds.emplace_back(rtData.worldInfos[RaytraceWorldType_simpleRT_DoRT_World]->world, 0);
		traceDesc.raygenBinding = rayGen;

		RtBindingDesc missDesc;
		missDesc.shader = RtUtil::createShaderFromProgram(rtData.system, ShaderProgram_simpleRT_SimpleRTMiss_MS);
		traceDesc.missBindings.push_back(RtBinding::create(rtData.system, missDesc));

		m_rtTrace_DoRT = RtTrace::create(rtData.system, traceDesc);
	}

	// Register with framegraph
	renderPassBookmark::registerPass(RenderPassBookmark_AfterPost,
		[this](FrameGraph& frameGraph, FrameGraphBlackboard& blackboard) { addPass(frameGraph, blackboard); });
}

simpleRTRenderPassModule::~simpleRTRenderPassModule()
{
	s_pipelineCache.clear();
}

}
