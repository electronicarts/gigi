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

class buffertestRenderPassModule : public RefCountImpl<IRenderPassModule>
{
public:
	buffertestRenderPassModule() = default;
	virtual ~buffertestRenderPassModule();

	void addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard);

	//IRenderPassModule interface
	void onInit(const InitParams& params) override;

public:
private:
	MemoryArena* m_arena = nullptr;

	static ShaderProgramPipelineCache s_pipelineCache;

	struct Struct_TestStruct
	{
		fb::math::Vector4Template<float> TheFloat4 = fb::math::Vector4Template<float>(0.f, 0.f, 0.f, 0.f);
		fb::math::Vector4Template<int> TheInt4 = fb::math::Vector4Template<int>(0, 0, 0, 0);
		unsigned int TheBool = false;
	};

	struct Struct__BufferTest_0CB
	{
		float alpha1 = 1.f;  // Adjusts the contribution of sample n-1
		float alpha2 = 0.f;  // Adjusts the contribution of sample n-2
		float gain = 0.5f;  // Overall Volume Adjustment
		float _padding0 = 0.f;  // Padding
	};

	Struct__BufferTest_0CB m_cb__BufferTest_0CB;
};

}
