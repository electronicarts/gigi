// (c) Electronic Arts.  All Rights Reserved.

#include "SubGraphTestShaderProgramDefinitions.h"
#include <Engine.World.Render/SubGraphTest/SubGraphTestShaderProgramsDefs.h>
#include <Engine.Pipeline/Driver/PipelineRegistrar.h>
#include <Engine.Render.Pipeline/Settings.h>
#include <Engine.Shaders.Pipeline/Compile/ShaderCompileTypes.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>
#include <Engine.Render/Core/CommonGeometryDeclarations.h>

namespace fb
{

FB_IMPLEMENT_SHADERPROGRAMSPROVIDER(SubGraphTestShaderProgramsProvider);

uint SubGraphTestShaderProgramsProvider::getShaderProgramDefinitionsVersion() const
{
	uint version = makeVersionNumber("SubGraphTest", 1338);

	return version;
}

void SubGraphTestShaderProgramsProvider::gatherShaderProgramDefinitions(
	ShaderRenderPath path,
	Platform platform,
	const ShaderProgramDatabasePipelineParams* params,
	eastl::vector<ShaderProgramDef>& outPrograms) const
{

	// Compute Shader: Swap_Colors
	{
		ShaderProgramDef program;
		program.shaderProgram = ShaderProgram_SubGraphTest_Swap_Colors_csmainCs;
		program.source = "Systems/WorldRender/SubGraphTest/SubGraphTest_SwapColors.hlsl";
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
