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

class GPUWriteRenderPassModule : public RefCountImpl<IRenderPassModule>
{
public:
	GPUWriteRenderPassModule() = default;
	virtual ~GPUWriteRenderPassModule();

	void addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard);

	//IRenderPassModule interface
	void onInit(const InitParams& params) override;

public:
private:
	MemoryArena* m_arena = nullptr;

	static ShaderProgramPipelineCache s_pipelineCache;

	struct Struct_BufferStruct
	{
		int theInt = 0;
		float theFloat = 0.f;
	};

	SmartRef<ITexture> m_textureTexture2D;
	RenderFormat m_textureTexture2D_format = RenderFormat_Unknown;
	uint m_textureTexture2D_width = 0;
	uint m_textureTexture2D_height = 0;
	uint m_textureTexture2D_depth = 0;

	SmartRef<ITexture> m_textureTexture2DArray;
	RenderFormat m_textureTexture2DArray_format = RenderFormat_Unknown;
	uint m_textureTexture2DArray_width = 0;
	uint m_textureTexture2DArray_height = 0;
	uint m_textureTexture2DArray_depth = 0;

	SmartRef<ITexture> m_textureTexture3D;
	RenderFormat m_textureTexture3D_format = RenderFormat_Unknown;
	uint m_textureTexture3D_width = 0;
	uint m_textureTexture3D_height = 0;
	uint m_textureTexture3D_depth = 0;

	SmartRef<IRenderBuffer> m_bufferFloatBuffer;
	RenderFormat m_bufferFloatBuffer_format = RenderFormat::RenderFormat_Unknown;
	uint32_t m_bufferFloatBuffer_size = 0;

	SmartRef<IRenderBuffer> m_bufferStructBuffer;
	RenderFormat m_bufferStructBuffer_format = RenderFormat::RenderFormat_Unknown;
	uint32_t m_bufferStructBuffer_size = 0;
};

}
