// (c) Electronic Arts.  All Rights Reserved.

#include "ReadbackSequenceShaderProgramDefinitions.h"
#include <Engine.World.Render/ReadbackSequence/ReadbackSequenceShaderProgramsDefs.h>
#include <Engine.Pipeline/Driver/PipelineRegistrar.h>
#include <Engine.Render.Pipeline/Settings.h>
#include <Engine.Shaders.Pipeline/Compile/ShaderCompileTypes.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>
#include <Engine.Render/Core/CommonGeometryDeclarations.h>

namespace fb
{

FB_IMPLEMENT_SHADERPROGRAMSPROVIDER(ReadbackSequenceShaderProgramsProvider);

uint ReadbackSequenceShaderProgramsProvider::getShaderProgramDefinitionsVersion() const
{
	uint version = makeVersionNumber("ReadbackSequence", 1338);

	return version;
}

void ReadbackSequenceShaderProgramsProvider::gatherShaderProgramDefinitions(
	ShaderRenderPath path,
	Platform platform,
	const ShaderProgramDatabasePipelineParams* params,
	eastl::vector<ShaderProgramDef>& outPrograms) const
{

	// Compute Shader: Node_1
	{
		ShaderProgramDef program;
		program.shaderProgram = ShaderProgram_ReadbackSequence_Node_1_csmainCs;
		program.source = "Systems/WorldRender/ReadbackSequence/ReadbackSequence.hlsl";
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
