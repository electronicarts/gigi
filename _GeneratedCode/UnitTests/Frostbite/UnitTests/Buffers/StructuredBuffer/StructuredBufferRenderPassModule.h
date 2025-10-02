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

class StructuredBufferRenderPassModule : public RefCountImpl<IRenderPassModule>
{
public:
	StructuredBufferRenderPassModule() = default;
	virtual ~StructuredBufferRenderPassModule();

	void addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard);

	//IRenderPassModule interface
	void onInit(const InitParams& params) override;

public:

	enum class Trinary: int
	{
		True,
		False,
		Maybe,
	};

	// Host Variables
	int m_varframeIndex = 0;
	float m_varframeDeltaTime = 0.f;
private:
	MemoryArena* m_arena = nullptr;

	static ShaderProgramPipelineCache s_pipelineCache;

	struct Struct_TheStructure
	{
		int TheInt = 0;
		float TheFloat = 0.f;
		int TheEnum = (int)Trinary::True;
		unsigned int TheBool = true;
		unsigned int TheUINT = 0;
		fb::math::Vector2Template<float> TheFloat2 = fb::math::Vector2Template<float>(0.f, 0.f);
	};

	struct Struct__csmain_0CB
	{
		float frameDeltaTime = 0.f;
		int frameIndex = 0;
		fb::math::Vector2Template<float> _padding0 = fb::math::Vector2Template<float>(0.f, 0.f);  // Padding
	};

	Struct__csmain_0CB m_cb__csmain_0CB;
};

}
