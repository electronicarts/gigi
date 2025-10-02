// (c) Electronic Arts.  All Rights Reserved.

#include "VariableAliasesShaderProgramDefinitions.h"
#include <Engine.World.Render/VariableAliases/VariableAliasesShaderProgramsDefs.h>
#include <Engine.Pipeline/Driver/PipelineRegistrar.h>
#include <Engine.Render.Pipeline/Settings.h>
#include <Engine.Shaders.Pipeline/Compile/ShaderCompileTypes.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>
#include <Engine.Render/Core/CommonGeometryDeclarations.h>

namespace fb
{

FB_IMPLEMENT_SHADERPROGRAMSPROVIDER(VariableAliasesShaderProgramsProvider);

uint VariableAliasesShaderProgramsProvider::getShaderProgramDefinitionsVersion() const
{
	uint version = makeVersionNumber("VariableAliases", 1338);

	return version;
}

void VariableAliasesShaderProgramsProvider::gatherShaderProgramDefinitions(
	ShaderRenderPath path,
	Platform platform,
	const ShaderProgramDatabasePipelineParams* params,
	eastl::vector<ShaderProgramDef>& outPrograms) const
{

	// Compute Shader: Set_Red
	{
		ShaderProgramDef program;
		program.shaderProgram = ShaderProgram_VariableAliases_Set_Red_mainCs;
		program.source = "Systems/WorldRender/VariableAliases/VariableAliases_SetChannel.hlsl";
		program.csName = "main";
		program.compileFlags = ShaderCompileFlag_None;
		program.macros.push_back(eastl::make_pair("__GigiDispatchMultiply", "uint3(1,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchDivide", "uint3(1,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPreAdd", "uint3(0,0,0)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPostAdd", "uint3(0,0,0)"));
		outPrograms.push_back(program);
	}

	// Compute Shader: Set_Green
	{
		ShaderProgramDef program;
		program.shaderProgram = ShaderProgram_VariableAliases_Set_Green_mainCs;
		program.source = "Systems/WorldRender/VariableAliases/VariableAliases_SetChannel.hlsl";
		program.csName = "main";
		program.compileFlags = ShaderCompileFlag_None;
		program.macros.push_back(eastl::make_pair("__GigiDispatchMultiply", "uint3(1,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchDivide", "uint3(1,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPreAdd", "uint3(0,0,0)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPostAdd", "uint3(0,0,0)"));
		outPrograms.push_back(program);
	}

	// Compute Shader: Set_Blue
	{
		ShaderProgramDef program;
		program.shaderProgram = ShaderProgram_VariableAliases_Set_Blue_mainCs;
		program.source = "Systems/WorldRender/VariableAliases/VariableAliases_SetChannel.hlsl";
		program.csName = "main";
		program.compileFlags = ShaderCompileFlag_None;
		program.macros.push_back(eastl::make_pair("__GigiDispatchMultiply", "uint3(1,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchDivide", "uint3(1,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPreAdd", "uint3(0,0,0)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPostAdd", "uint3(0,0,0)"));
		program.macros.push_back(eastl::make_pair("__GIGI_AlIAS_VARIABLE_CONST_Value", "0.75"));
		outPrograms.push_back(program);
	}
}

}
