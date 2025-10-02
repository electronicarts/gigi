// (c) Electronic Arts.  All Rights Reserved.

#include "TwoRayGensShaderProgramDefinitions.h"
#include <Engine.World.Render/TwoRayGens/TwoRayGensShaderProgramsDefs.h>
#include <Engine.Pipeline/Driver/PipelineRegistrar.h>
#include <Engine.Render.Pipeline/Settings.h>
#include <Engine.Shaders.Pipeline/Compile/ShaderCompileTypes.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>
#include <Engine.Render/Core/CommonGeometryDeclarations.h>

namespace fb
{

FB_IMPLEMENT_SHADERPROGRAMSPROVIDER(TwoRayGensShaderProgramsProvider);

uint TwoRayGensShaderProgramsProvider::getShaderProgramDefinitionsVersion() const
{
	uint version = makeVersionNumber("TwoRayGens", 1340);

	return version;
}

void TwoRayGensShaderProgramsProvider::gatherShaderProgramDefinitions(
	ShaderRenderPath path,
	Platform platform,
	const ShaderProgramDatabasePipelineParams* params,
	eastl::vector<ShaderProgramDef>& outPrograms) const
{
	// Ray Shader: TwoRayGensMiss1
	{
		ShaderProgramDef program;
		program.source = "Systems/WorldRender/TwoRayGens/TwoRayGens1.hlsl";
		program.shaderProgram = ShaderProgram_TwoRayGens_TwoRayGensMiss1_MS;
		program.msName = "Miss1";
		program.compileFlags = ShaderCompileFlag_KeepDXBC | ShaderCompileFlag_EnableScratch | ShaderCompileFlag_Hlsl2021;
		program.isRtHitGroup = false;
		program.rtMaxPayloadSizeInBytes = 12;
		outPrograms.push_back(program);
	}

	// Ray Shader: TwoRayGensClosestHit1
	{
		ShaderProgramDef program;
		program.source = "Systems/WorldRender/TwoRayGens/TwoRayGens1.hlsl";
		program.shaderProgram = ShaderProgram_TwoRayGens_TwoRayGensClosestHit1_HG;
		program.chName = "ClosestHit1";
		program.compileFlags = ShaderCompileFlag_KeepDXBC | ShaderCompileFlag_EnableScratch | ShaderCompileFlag_Hlsl2021;
		program.isRtHitGroup = true;
		program.rtMaxPayloadSizeInBytes = 12;
		outPrograms.push_back(program);
	}

	// Ray Shader: TwoRayGensMiss2A
	{
		ShaderProgramDef program;
		program.source = "Systems/WorldRender/TwoRayGens/TwoRayGens2.hlsl";
		program.shaderProgram = ShaderProgram_TwoRayGens_TwoRayGensMiss2A_MS;
		program.msName = "Miss2A";
		program.compileFlags = ShaderCompileFlag_KeepDXBC | ShaderCompileFlag_EnableScratch | ShaderCompileFlag_Hlsl2021;
		program.isRtHitGroup = false;
		program.rtMaxPayloadSizeInBytes = 12;
		outPrograms.push_back(program);
	}

	// Ray Shader: TwoRayGensMiss2B
	{
		ShaderProgramDef program;
		program.source = "Systems/WorldRender/TwoRayGens/TwoRayGens2.hlsl";
		program.shaderProgram = ShaderProgram_TwoRayGens_TwoRayGensMiss2B_MS;
		program.msName = "Miss2B";
		program.compileFlags = ShaderCompileFlag_KeepDXBC | ShaderCompileFlag_EnableScratch | ShaderCompileFlag_Hlsl2021;
		program.isRtHitGroup = false;
		program.rtMaxPayloadSizeInBytes = 12;
		outPrograms.push_back(program);
	}

	// Ray Shader: TwoRayGensClosestHit2
	{
		ShaderProgramDef program;
		program.source = "Systems/WorldRender/TwoRayGens/TwoRayGens2.hlsl";
		program.shaderProgram = ShaderProgram_TwoRayGens_TwoRayGensClosestHit2_HG;
		program.chName = "ClosestHit2";
		program.compileFlags = ShaderCompileFlag_KeepDXBC | ShaderCompileFlag_EnableScratch | ShaderCompileFlag_Hlsl2021;
		program.isRtHitGroup = true;
		program.rtMaxPayloadSizeInBytes = 12;
		outPrograms.push_back(program);
	}

	// Ray Shader: DoRT1
	{
		ShaderProgramDef program;
		program.source = "Systems/WorldRender/TwoRayGens/TwoRayGens1.hlsl";
		program.shaderProgram = ShaderProgram_TwoRayGens_TwoRayGens1_RG;
		program.rgName = "RayGen1";
		program.compileFlags = ShaderCompileFlag_KeepDXBC | ShaderCompileFlag_EnableScratch | ShaderCompileFlag_Hlsl2021;
		program.isRtHitGroup = false;
		program.rtMaxPayloadSizeInBytes = 12;
		outPrograms.push_back(program);
	}
	// Ray Shader: DoRT2
	{
		ShaderProgramDef program;
		program.source = "Systems/WorldRender/TwoRayGens/TwoRayGens2.hlsl";
		program.shaderProgram = ShaderProgram_TwoRayGens_TwoRayGens2_RG;
		program.rgName = "RayGen2";
		program.compileFlags = ShaderCompileFlag_KeepDXBC | ShaderCompileFlag_EnableScratch | ShaderCompileFlag_Hlsl2021;
		program.isRtHitGroup = false;
		program.rtMaxPayloadSizeInBytes = 12;
		outPrograms.push_back(program);
	}
}

}
