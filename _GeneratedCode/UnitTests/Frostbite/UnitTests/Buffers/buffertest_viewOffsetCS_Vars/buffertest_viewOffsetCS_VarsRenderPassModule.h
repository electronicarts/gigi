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

class buffertest_viewOffsetCS_VarsRenderPassModule : public RefCountImpl<IRenderPassModule>
{
public:
	buffertest_viewOffsetCS_VarsRenderPassModule() = default;
	virtual ~buffertest_viewOffsetCS_VarsRenderPassModule();

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

	// Internal Variables
	int m_varInputTypedBufferBegin = 0;
	int m_varInputTypedBufferSize = 0;
	int m_varOutputTypedBufferBegin = 1;
	int m_varOutputTypedBufferSize = 0;
	int m_varInputTypedStructBufferBegin = 0;
	int m_varInputTypedStructBufferSize = 0;
	int m_varOutputTypedStructBufferBegin = 4;
	int m_varOutputTypedStructBufferSize = 0;
	int m_varInputStructuredBufferBegin = 0;
	int m_varInputStructuredBufferSize = 0;
	int m_varOutputStructuredBufferBegin = 1;
	int m_varOutputStructuredBufferSize = 0;
	int m_varInputTypedBufferRawBegin = 0;
	int m_varInputTypedBufferRawSize = 0;
	int m_varOutputTypedBufferRawBegin = 16;
	int m_varOutputTypedBufferRawSize = 0;

	Struct__BufferTest_0CB m_cb__BufferTest_0CB;
};

}
