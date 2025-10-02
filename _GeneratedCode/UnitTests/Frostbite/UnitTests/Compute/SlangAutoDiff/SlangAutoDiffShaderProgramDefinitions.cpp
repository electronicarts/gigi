// (c) Electronic Arts.  All Rights Reserved.

#include "SlangAutoDiffShaderProgramDefinitions.h"
#include <Engine.World.Render/SlangAutoDiff/SlangAutoDiffShaderProgramsDefs.h>
#include <Engine.Pipeline/Driver/PipelineRegistrar.h>
#include <Engine.Render.Pipeline/Settings.h>
#include <Engine.Shaders.Pipeline/Compile/ShaderCompileTypes.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>
#include <Engine.Render/Core/CommonGeometryDeclarations.h>

namespace fb
{

FB_IMPLEMENT_SHADERPROGRAMSPROVIDER(SlangAutoDiffShaderProgramsProvider);

uint SlangAutoDiffShaderProgramsProvider::getShaderProgramDefinitionsVersion() const
{
	uint version = makeVersionNumber("SlangAutoDiff", 1338);

	return version;
}

void SlangAutoDiffShaderProgramsProvider::gatherShaderProgramDefinitions(
	ShaderRenderPath path,
	Platform platform,
	const ShaderProgramDatabasePipelineParams* params,
	eastl::vector<ShaderProgramDef>& outPrograms) const
{

	// Compute Shader: Initialize
	{
		ShaderProgramDef program;
		program.shaderProgram = ShaderProgram_SlangAutoDiff_Initialize_csmainCs;
		program.source = "Systems/WorldRender/SlangAutoDiff/SlangAutoDiff_Initialize.hlsl";
		program.csName = "csmain";
		program.compileFlags = ShaderCompileFlag_None;
		program.macros.push_back(eastl::make_pair("__GigiDispatchMultiply", "uint3(1,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchDivide", "uint3(1,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPreAdd", "uint3(0,0,0)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPostAdd", "uint3(0,0,0)"));
		outPrograms.push_back(program);
	}

	// Compute Shader: GradientDescend
	{
		ShaderProgramDef program;
		program.shaderProgram = ShaderProgram_SlangAutoDiff_GradientDescend_csmainCs;
		program.source = "Systems/WorldRender/SlangAutoDiff/SlangAutoDiff_Descend_0.hlsl";
		program.csName = "csmain";
		program.compileFlags = ShaderCompileFlag_None;
		program.macros.push_back(eastl::make_pair("SCREEN_MIN", "0.001f"));
		program.macros.push_back(eastl::make_pair("SCREEN_MAX", "0.99f"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchMultiply", "uint3(1,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchDivide", "uint3(1,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPreAdd", "uint3(0,0,0)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPostAdd", "uint3(0,0,0)"));
		outPrograms.push_back(program);
	}

	// Compute Shader: Render
	{
		ShaderProgramDef program;
		program.shaderProgram = ShaderProgram_SlangAutoDiff_Render_csmainCs;
		program.source = "Systems/WorldRender/SlangAutoDiff/SlangAutoDiff_Render.hlsl";
		program.csName = "csmain";
		program.compileFlags = ShaderCompileFlag_None;
		program.macros.push_back(eastl::make_pair("__GigiDispatchMultiply", "uint3(1,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchDivide", "uint3(1,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPreAdd", "uint3(0,0,0)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPostAdd", "uint3(0,0,0)"));
		outPrograms.push_back(program);
	}
}

}
