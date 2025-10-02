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

class boxblurRenderPassModule : public RefCountImpl<IRenderPassModule>
{
public:
	boxblurRenderPassModule() = default;
	virtual ~boxblurRenderPassModule();

	void addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard);

	//IRenderPassModule interface
	void onInit(const InitParams& params) override;

public:

	// Host Variables
	fb::math::Vector3Template<float> m_variResolution = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);
	float m_variTime = 0.f;
	float m_variTimeDelta = 0.f;
	float m_variFrameRate = 0.f;
	int m_variFrame = 0;
	fb::math::Vector4Template<float> m_variMouse = fb::math::Vector4Template<float>(0.f, 0.f, 0.f, 0.f);
	fb::math::Vector4Template<float> m_varMouseState = fb::math::Vector4Template<float>(0.f, 0.f, 0.f, 0.f);
	fb::math::Vector4Template<float> m_varMouseStateLastFrame = fb::math::Vector4Template<float>(0.f, 0.f, 0.f, 0.f);
private:
	MemoryArena* m_arena = nullptr;

	static ShaderProgramPipelineCache s_pipelineCache;

	struct Struct__BoxBlur_0CB
	{
		int radius = 2;  // The radius of the blur.  Actual size in pixles of the blur is (radius*2+1)^2
		unsigned int sRGB = true;
		fb::math::Vector2Template<float> _padding0 = fb::math::Vector2Template<float>(0.f, 0.f);  // Padding
	};

	struct Struct__BoxBlur_1CB
	{
		int radius = 2;  // The radius of the blur.  Actual size in pixles of the blur is (radius*2+1)^2
		unsigned int sRGB = true;
		fb::math::Vector2Template<float> _padding0 = fb::math::Vector2Template<float>(0.f, 0.f);  // Padding
	};

	Struct__BoxBlur_0CB m_cb__BoxBlur_0CB;

	Struct__BoxBlur_1CB m_cb__BoxBlur_1CB;
};

}
