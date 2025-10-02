// (c) Electronic Arts.  All Rights Reserved.

#include "TwoRayGensSubgraphShaderProgramDefinitions.h"
#include <Engine.World.Render/TwoRayGensSubgraph/TwoRayGensSubgraphShaderProgramsDefs.h>
#include <Engine.Pipeline/Driver/PipelineRegistrar.h>
#include <Engine.Render.Pipeline/Settings.h>
#include <Engine.Shaders.Pipeline/Compile/ShaderCompileTypes.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>
#include <Engine.Render/Core/CommonGeometryDeclarations.h>

namespace fb
{

FB_IMPLEMENT_SHADERPROGRAMSPROVIDER(TwoRayGensSubgraphShaderProgramsProvider);

uint TwoRayGensSubgraphShaderProgramsProvider::getShaderProgramDefinitionsVersion() const
{
	uint version = makeVersionNumber("TwoRayGensSubgraph", 1340);

	return version;
}

void TwoRayGensSubgraphShaderProgramsProvider::gatherShaderProgramDefinitions(
	ShaderRenderPath path,
	Platform platform,
	const ShaderProgramDatabasePipelineParams* params,
	eastl::vector<ShaderProgramDef>& outPrograms) const
{
	// Ray Shader: A_TwoRayGensMiss1
	{
		ShaderProgramDef program;
		program.source = "Systems/WorldRender/TwoRayGensSubgraph/TwoRayGensSubgraphA_A/TwoRayGens1.hlsl";
		program.shaderProgram = ShaderProgram_TwoRayGensSubgraph_A_TwoRayGensMiss1_MS;
		program.msName = "Miss1";
		program.compileFlags = ShaderCompileFlag_KeepDXBC | ShaderCompileFlag_EnableScratch | ShaderCompileFlag_Hlsl2021;
		program.isRtHitGroup = false;
		program.rtMaxPayloadSizeInBytes = 12;
		outPrograms.push_back(program);
	}

	// Ray Shader: A_TwoRayGensClosestHit1
	{
		ShaderProgramDef program;
		program.source = "Systems/WorldRender/TwoRayGensSubgraph/TwoRayGensSubgraphA_A/TwoRayGens1.hlsl";
		program.shaderProgram = ShaderProgram_TwoRayGensSubgraph_A_TwoRayGensClosestHit1_HG;
		program.chName = "ClosestHit1";
		program.compileFlags = ShaderCompileFlag_KeepDXBC | ShaderCompileFlag_EnableScratch | ShaderCompileFlag_Hlsl2021;
		program.isRtHitGroup = true;
		program.rtMaxPayloadSizeInBytes = 12;
		outPrograms.push_back(program);
	}

	// Ray Shader: B_TwoRayGensMiss2A
	{
		ShaderProgramDef program;
		program.source = "Systems/WorldRender/TwoRayGensSubgraph/TwoRayGensSubgraphB_B/TwoRayGens2.hlsl";
		program.shaderProgram = ShaderProgram_TwoRayGensSubgraph_B_TwoRayGensMiss2A_MS;
		program.msName = "Miss2A";
		program.compileFlags = ShaderCompileFlag_KeepDXBC | ShaderCompileFlag_EnableScratch | ShaderCompileFlag_Hlsl2021;
		program.isRtHitGroup = false;
		program.rtMaxPayloadSizeInBytes = 12;
		outPrograms.push_back(program);
	}

	// Ray Shader: B_TwoRayGensMiss2B
	{
		ShaderProgramDef program;
		program.source = "Systems/WorldRender/TwoRayGensSubgraph/TwoRayGensSubgraphB_B/TwoRayGens2.hlsl";
		program.shaderProgram = ShaderProgram_TwoRayGensSubgraph_B_TwoRayGensMiss2B_MS;
		program.msName = "Miss2B";
		program.compileFlags = ShaderCompileFlag_KeepDXBC | ShaderCompileFlag_EnableScratch | ShaderCompileFlag_Hlsl2021;
		program.isRtHitGroup = false;
		program.rtMaxPayloadSizeInBytes = 12;
		outPrograms.push_back(program);
	}

	// Ray Shader: B_TwoRayGensClosestHit2
	{
		ShaderProgramDef program;
		program.source = "Systems/WorldRender/TwoRayGensSubgraph/TwoRayGensSubgraphB_B/TwoRayGens2.hlsl";
		program.shaderProgram = ShaderProgram_TwoRayGensSubgraph_B_TwoRayGensClosestHit2_HG;
		program.chName = "ClosestHit2";
		program.compileFlags = ShaderCompileFlag_KeepDXBC | ShaderCompileFlag_EnableScratch | ShaderCompileFlag_Hlsl2021;
		program.isRtHitGroup = true;
		program.rtMaxPayloadSizeInBytes = 12;
		outPrograms.push_back(program);
	}

	// Ray Shader: A_DoRT1
	{
		ShaderProgramDef program;
		program.source = "Systems/WorldRender/TwoRayGensSubgraph/TwoRayGensSubgraphA_A/TwoRayGens1.hlsl";
		program.shaderProgram = ShaderProgram_TwoRayGensSubgraph_A_TwoRayGens1_RG;
		program.rgName = "RayGen1";
		program.compileFlags = ShaderCompileFlag_KeepDXBC | ShaderCompileFlag_EnableScratch | ShaderCompileFlag_Hlsl2021;
		program.isRtHitGroup = false;
		program.rtMaxPayloadSizeInBytes = 12;
		outPrograms.push_back(program);
	}
	// Ray Shader: B_DoRT2
	{
		ShaderProgramDef program;
		program.source = "Systems/WorldRender/TwoRayGensSubgraph/TwoRayGensSubgraphB_B/TwoRayGens2.hlsl";
		program.shaderProgram = ShaderProgram_TwoRayGensSubgraph_B_TwoRayGens2_RG;
		program.rgName = "RayGen2";
		program.compileFlags = ShaderCompileFlag_KeepDXBC | ShaderCompileFlag_EnableScratch | ShaderCompileFlag_Hlsl2021;
		program.isRtHitGroup = false;
		program.rtMaxPayloadSizeInBytes = 12;
		outPrograms.push_back(program);
	}
}

}
