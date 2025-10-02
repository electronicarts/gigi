// (c) Electronic Arts.  All Rights Reserved.

#include "BarrierTestShaderProgramDefinitions.h"
#include <Engine.World.Render/BarrierTest/BarrierTestShaderProgramsDefs.h>
#include <Engine.Pipeline/Driver/PipelineRegistrar.h>
#include <Engine.Render.Pipeline/Settings.h>
#include <Engine.Shaders.Pipeline/Compile/ShaderCompileTypes.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>
#include <Engine.Render/Core/CommonGeometryDeclarations.h>

namespace fb
{

FB_IMPLEMENT_SHADERPROGRAMSPROVIDER(BarrierTestShaderProgramsProvider);

uint BarrierTestShaderProgramsProvider::getShaderProgramDefinitionsVersion() const
{
	uint version = makeVersionNumber("BarrierTest", 1338);

	return version;
}

void BarrierTestShaderProgramsProvider::gatherShaderProgramDefinitions(
	ShaderRenderPath path,
	Platform platform,
	const ShaderProgramDatabasePipelineParams* params,
	eastl::vector<ShaderProgramDef>& outPrograms) const
{

	// Compute Shader: Draw_Left
	{
		ShaderProgramDef program;
		program.shaderProgram = ShaderProgram_BarrierTest_Draw_Left_csmainCs;
		program.source = "Systems/WorldRender/BarrierTest/BarrierTestLeft.hlsl";
		program.csName = "csmain";
		program.compileFlags = ShaderCompileFlag_None;
		program.macros.push_back(eastl::make_pair("__GigiDispatchMultiply", "uint3(1,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchDivide", "uint3(2,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPreAdd", "uint3(0,0,0)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPostAdd", "uint3(0,0,0)"));
		outPrograms.push_back(program);
	}

	// Compute Shader: Draw_Right
	{
		ShaderProgramDef program;
		program.shaderProgram = ShaderProgram_BarrierTest_Draw_Right_csmainCs;
		program.source = "Systems/WorldRender/BarrierTest/BarrierTestRight.hlsl";
		program.csName = "csmain";
		program.compileFlags = ShaderCompileFlag_None;
		program.macros.push_back(eastl::make_pair("__GigiDispatchMultiply", "uint3(1,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchDivide", "uint3(2,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPreAdd", "uint3(0,0,0)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPostAdd", "uint3(0,0,0)"));
		outPrograms.push_back(program);
	}

	// Compute Shader: After
	{
		ShaderProgramDef program;
		program.shaderProgram = ShaderProgram_BarrierTest_After_csmainCs;
		program.source = "Systems/WorldRender/BarrierTest/BarrierTestAfter.hlsl";
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
