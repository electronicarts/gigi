// (c) Electronic Arts.  All Rights Reserved.

#include "profilingShaderProgramDefinitions.h"
#include <Engine.World.Render/profiling/profilingShaderProgramsDefs.h>
#include <Engine.Pipeline/Driver/PipelineRegistrar.h>
#include <Engine.Render.Pipeline/Settings.h>
#include <Engine.Shaders.Pipeline/Compile/ShaderCompileTypes.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>
#include <Engine.Render/Core/CommonGeometryDeclarations.h>

namespace fb
{

FB_IMPLEMENT_SHADERPROGRAMSPROVIDER(profilingShaderProgramsProvider);

uint profilingShaderProgramsProvider::getShaderProgramDefinitionsVersion() const
{
	uint version = makeVersionNumber("profiling", 1338);

	return version;
}

void profilingShaderProgramsProvider::gatherShaderProgramDefinitions(
	ShaderRenderPath path,
	Platform platform,
	const ShaderProgramDatabasePipelineParams* params,
	eastl::vector<ShaderProgramDef>& outPrograms) const
{

	// Compute Shader: DoSimpleCS
	// Runs the shader
	{
		ShaderProgramDef program;
		program.shaderProgram = ShaderProgram_profiling_DoSimpleCS_MainCs;
		program.source = "Systems/WorldRender/profiling/profiling.hlsl";
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
