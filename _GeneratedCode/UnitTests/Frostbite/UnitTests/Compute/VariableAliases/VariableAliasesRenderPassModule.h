// (c) Electronic Arts.  All Rights Reserved.
#pragma once

#include <Engine.Render/Core2/Rc2Helpers.h>
#include <Engine.Render/FrameGraph/FrameGraphRenderPassModule.h>
#include <Engine.Render/Raytrace/RaytraceRenderPassModule.h>
#include <Engine.RtFramework/RtFramework.h>

#include <FBMath/Vector2TemplateType.h>
#include <FBMath/Vector3TemplateType.h>
#include <FBMath/Vector4TemplateType.h>
#include <FBMath/Matrix44TemplateType.h>

namespace fb
{

class VariableAliasesRenderPassModule : public RefCountImpl<IRenderPassModule>
{
public:
	VariableAliasesRenderPassModule() = default;
	virtual ~VariableAliasesRenderPassModule();

	void addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard);

	//IRenderPassModule interface
	void onInit(const InitParams& params) override;

public:
private:
	MemoryArena* m_arena = nullptr;

	static ShaderProgramPipelineCache s_pipelineCache;

	struct Struct__SetChannel_0CB
	{
		int _alias_Channel = 0;
		float _alias_Value = 0.f;
		fb::math::Vector2Template<float> _padding0 = fb::math::Vector2Template<float>(0.f, 0.f);  // Padding
	};

	struct Struct__SetChannel_1CB
	{
		int _alias_Channel = 0;
		float _alias_Value = 0.f;
		fb::math::Vector2Template<float> _padding0 = fb::math::Vector2Template<float>(0.f, 0.f);  // Padding
	};

	Struct__SetChannel_0CB m_cb__SetChannel_0CB_0;

	Struct__SetChannel_0CB m_cb__SetChannel_0CB_1;

	Struct__SetChannel_1CB m_cb__SetChannel_1CB_0;
};

}
