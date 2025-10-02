// (c) Electronic Arts.  All Rights Reserved.

#include "StencilShaderProgramDefinitions.h"
#include <Engine.World.Render/Stencil/StencilShaderProgramsDefs.h>
#include <Engine.Pipeline/Driver/PipelineRegistrar.h>
#include <Engine.Render.Pipeline/Settings.h>
#include <Engine.Shaders.Pipeline/Compile/ShaderCompileTypes.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>
#include <Engine.Render/Core/CommonGeometryDeclarations.h>

namespace fb
{

FB_IMPLEMENT_SHADERPROGRAMSPROVIDER(StencilShaderProgramsProvider);

uint StencilShaderProgramsProvider::getShaderProgramDefinitionsVersion() const
{
	uint version = makeVersionNumber("Stencil", 1338);

	return version;
}

void StencilShaderProgramsProvider::gatherShaderProgramDefinitions(
	ShaderRenderPath path,
	Platform platform,
	const ShaderProgramDatabasePipelineParams* params,
	eastl::vector<ShaderProgramDef>& outPrograms) const
{

	// Draw Call: Draw_1
	{
		ShaderProgramDef program;
		program.shaderProgram =ShaderProgram_Stencil_Draw_1_VSPS,
		program.source = "Systems/WorldRender/Stencil/_VSPS_Draw_1.hlsl";
		program.vsName = "Draw1VS";
		program.psName = "Draw1PS";
		program.compileFlags = ShaderCompileFlag_None;
		program.geometryDeclaration.streams[0].classification = VertexElementClassification_PerVertex;
		program.geometryDeclaration.streams[0].stride = ;
		program.geometryDeclaration.elementCount = 0;
		program.geometryDeclaration.streamCount = 1;
		outPrograms.push_back(program);
	}

	// Draw Call: Draw_2
	{
		ShaderProgramDef program;
		program.shaderProgram =ShaderProgram_Stencil_Draw_2_VSPS,
		program.source = "Systems/WorldRender/Stencil/_VSPS_Draw_2.hlsl";
		program.vsName = "Draw2VS";
		program.psName = "Draw2PS";
		program.compileFlags = ShaderCompileFlag_None;
		program.geometryDeclaration.elementCount = 0;
		program.geometryDeclaration.streamCount = 0;
		outPrograms.push_back(program);
	}
}

}
