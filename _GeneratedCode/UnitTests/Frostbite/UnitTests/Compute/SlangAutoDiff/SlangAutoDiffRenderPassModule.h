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

class SlangAutoDiffRenderPassModule : public RefCountImpl<IRenderPassModule>
{
public:
	SlangAutoDiffRenderPassModule() = default;
	virtual ~SlangAutoDiffRenderPassModule();

	void addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard);

	//IRenderPassModule interface
	void onInit(const InitParams& params) override;

public:

	// Host Variables
	bool m_varinitialized = false;
	int m_varFrameIndex = 0;
	fb::math::Vector4Template<float> m_varMouseState = fb::math::Vector4Template<float>(0.f, 0.f, 0.f, 0.f);
	fb::math::Vector4Template<float> m_varMouseStateLastFrame = fb::math::Vector4Template<float>(0.f, 0.f, 0.f, 0.f);
	fb::math::Vector3Template<float> m_variResolution = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);
private:
	MemoryArena* m_arena = nullptr;

	static ShaderProgramPipelineCache s_pipelineCache;

	struct Struct__Init_0CB
	{
		int FrameIndex = 0;
		fb::math::Vector3Template<float> _padding0 = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);  // Padding
		fb::math::Vector4Template<float> MouseState = fb::math::Vector4Template<float>(0.f, 0.f, 0.f, 0.f);
		fb::math::Vector4Template<float> MouseStateLastFrame = fb::math::Vector4Template<float>(0.f, 0.f, 0.f, 0.f);
		int NumGaussians = 10;
		fb::math::Vector3Template<float> iResolution = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);
		unsigned int initialized = false;
		fb::math::Vector3Template<float> _padding1 = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);  // Padding
	};

	struct Struct__Render_0CB
	{
		int NumGaussians = 10;
		unsigned int QuantizeDisplay = false;
		fb::math::Vector2Template<float> _padding0 = fb::math::Vector2Template<float>(0.f, 0.f);  // Padding
	};

	struct Struct__Descend_0CB
	{
		float LearningRate = 0.100000001f;
		float MaximumStepSize = 0.00999999978f;
		int NumGaussians = 10;
		unsigned int UseBackwardAD = true;
	};

	// first 2 floats is position, nect 2 floats are the gradient. Then 5 floats per Gaussian.
	SmartRef<IRenderBuffer> m_bufferData;
	RenderFormat m_bufferData_format = RenderFormat::RenderFormat_Unknown;
	uint32_t m_bufferData_size = 0;

	Struct__Init_0CB m_cb__Init_0CB;

	Struct__Render_0CB m_cb__Render_0CB;

	Struct__Descend_0CB m_cb__Descend_0CB;
};

}
