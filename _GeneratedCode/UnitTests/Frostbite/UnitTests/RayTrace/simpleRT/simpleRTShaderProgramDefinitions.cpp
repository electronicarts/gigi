// (c) Electronic Arts.  All Rights Reserved.

#include "simpleRTShaderProgramDefinitions.h"
#include <Engine.World.Render/simpleRT/simpleRTShaderProgramsDefs.h>
#include <Engine.Pipeline/Driver/PipelineRegistrar.h>
#include <Engine.Render.Pipeline/Settings.h>
#include <Engine.Shaders.Pipeline/Compile/ShaderCompileTypes.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>
#include <Engine.Render/Core/CommonGeometryDeclarations.h>

namespace fb
{

FB_IMPLEMENT_SHADERPROGRAMSPROVIDER(simpleRTShaderProgramsProvider);

uint simpleRTShaderProgramsProvider::getShaderProgramDefinitionsVersion() const
{
	uint version = makeVersionNumber("simpleRT", 1340);

	return version;
}

void simpleRTShaderProgramsProvider::gatherShaderProgramDefinitions(
	ShaderRenderPath path,
	Platform platform,
	const ShaderProgramDatabasePipelineParams* params,
	eastl::vector<ShaderProgramDef>& outPrograms) const
{
	// Ray Shader: SimpleRTMiss
	{
		ShaderProgramDef program;
		program.source = "Systems/WorldRender/simpleRT/simpleRT.hlsl";
		program.shaderProgram = ShaderProgram_simpleRT_SimpleRTMiss_MS;
		program.msName = "Miss";
		program.compileFlags = ShaderCompileFlag_KeepDXBC | ShaderCompileFlag_EnableScratch | ShaderCompileFlag_Hlsl2021;
		program.isRtHitGroup = false;
		program.rtMaxPayloadSizeInBytes = 12;
		outPrograms.push_back(program);
	}

	// Ray Shader: SimpleRTClosestHit
	{
		ShaderProgramDef program;
		program.source = "Systems/WorldRender/simpleRT/simpleRT.hlsl";
		program.shaderProgram = ShaderProgram_simpleRT_SimpleRTClosestHit_HG;
		program.chName = "ClosestHit";
		program.compileFlags = ShaderCompileFlag_KeepDXBC | ShaderCompileFlag_EnableScratch | ShaderCompileFlag_Hlsl2021;
		program.isRtHitGroup = true;
		program.rtMaxPayloadSizeInBytes = 12;
		outPrograms.push_back(program);
	}

	// Ray Shader: DoRT
	{
		ShaderProgramDef program;
		program.source = "Systems/WorldRender/simpleRT/simpleRT.hlsl";
		program.shaderProgram = ShaderProgram_simpleRT_SimpleRTRayGen_RG;
		program.rgName = "RayGen";
		program.compileFlags = ShaderCompileFlag_KeepDXBC | ShaderCompileFlag_EnableScratch | ShaderCompileFlag_Hlsl2021;
		program.isRtHitGroup = false;
		program.rtMaxPayloadSizeInBytes = 12;
		outPrograms.push_back(program);
	}
}

}
