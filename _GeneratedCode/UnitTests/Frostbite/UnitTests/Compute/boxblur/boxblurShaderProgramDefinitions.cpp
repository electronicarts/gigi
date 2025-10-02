// (c) Electronic Arts.  All Rights Reserved.

#include "boxblurShaderProgramDefinitions.h"
#include <Engine.World.Render/boxblur/boxblurShaderProgramsDefs.h>
#include <Engine.Pipeline/Driver/PipelineRegistrar.h>
#include <Engine.Render.Pipeline/Settings.h>
#include <Engine.Shaders.Pipeline/Compile/ShaderCompileTypes.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>
#include <Engine.Render/Core/CommonGeometryDeclarations.h>

namespace fb
{

FB_IMPLEMENT_SHADERPROGRAMSPROVIDER(boxblurShaderProgramsProvider);

uint boxblurShaderProgramsProvider::getShaderProgramDefinitionsVersion() const
{
	uint version = makeVersionNumber("boxblur", 1338);

	return version;
}

void boxblurShaderProgramsProvider::gatherShaderProgramDefinitions(
	ShaderRenderPath path,
	Platform platform,
	const ShaderProgramDatabasePipelineParams* params,
	eastl::vector<ShaderProgramDef>& outPrograms) const
{

	// Compute Shader: BlurH
	// Horizontal blur pass
	{
		ShaderProgramDef program;
		program.shaderProgram = ShaderProgram_boxblur_BlurH_BlurHCs;
		program.source = "Systems/WorldRender/boxblur/boxblur_0.hlsl";
		program.csName = "BlurH";
		program.compileFlags = ShaderCompileFlag_None;
		program.macros.push_back(eastl::make_pair("__GigiDispatchMultiply", "uint3(1,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchDivide", "uint3(1,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPreAdd", "uint3(0,0,0)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPostAdd", "uint3(0,0,0)"));
		outPrograms.push_back(program);
	}

	// Compute Shader: BlurV
	// Vertical blur pass
	{
		ShaderProgramDef program;
		program.shaderProgram = ShaderProgram_boxblur_BlurV_BlurVCs;
		program.source = "Systems/WorldRender/boxblur/boxblur_1.hlsl";
		program.csName = "BlurV";
		program.compileFlags = ShaderCompileFlag_None;
		program.macros.push_back(eastl::make_pair("__GigiDispatchMultiply", "uint3(1,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchDivide", "uint3(1,1,1)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPreAdd", "uint3(0,0,0)"));
		program.macros.push_back(eastl::make_pair("__GigiDispatchPostAdd", "uint3(0,0,0)"));
		outPrograms.push_back(program);
	}
}

}
