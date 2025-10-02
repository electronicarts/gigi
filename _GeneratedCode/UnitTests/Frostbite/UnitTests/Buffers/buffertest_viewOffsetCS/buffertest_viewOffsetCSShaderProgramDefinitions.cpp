// (c) Electronic Arts.  All Rights Reserved.

#include "buffertest_viewOffsetCSShaderProgramDefinitions.h"
#include <Engine.World.Render/buffertest_viewOffsetCS/buffertest_viewOffsetCSShaderProgramsDefs.h>
#include <Engine.Pipeline/Driver/PipelineRegistrar.h>
#include <Engine.Render.Pipeline/Settings.h>
#include <Engine.Shaders.Pipeline/Compile/ShaderCompileTypes.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>
#include <Engine.Render/Core/CommonGeometryDeclarations.h>

namespace fb
{

FB_IMPLEMENT_SHADERPROGRAMSPROVIDER(buffertest_viewOffsetCSShaderProgramsProvider);

uint buffertest_viewOffsetCSShaderProgramsProvider::getShaderProgramDefinitionsVersion() const
{
	uint version = makeVersionNumber("buffertest_viewOffsetCS", 1338);

	return version;
}

void buffertest_viewOffsetCSShaderProgramsProvider::gatherShaderProgramDefinitions(
	ShaderRenderPath path,
	Platform platform,
	const ShaderProgramDatabasePipelineParams* params,
	eastl::vector<ShaderProgramDef>& outPrograms) const
{

	// Compute Shader: BufferTest
	// BufferTest compute shader
	{
		ShaderProgramDef program;
		program.shaderProgram = ShaderProgram_buffertest_viewOffsetCS_BufferTest_MainCs;
		program.source = "Systems/WorldRender/buffertest_viewOffsetCS/BufferTest.hlsl";
		program.csName = "Main";
		program.compileFlags = ShaderCompileFlag_None;
		program.macros.push_back(eastl::make_pair("__GigiDispatchMultiply", "uint3(1,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchDivide", "uint3(1,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPreAdd", "uint3(0,0,0)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPostAdd", "uint3(0,0,0)"));
		outPrograms.push_back(program);
	}
}

}
