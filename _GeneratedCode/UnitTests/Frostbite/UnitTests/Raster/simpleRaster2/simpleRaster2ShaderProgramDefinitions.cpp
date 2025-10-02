// (c) Electronic Arts.  All Rights Reserved.

#include "simpleRaster2ShaderProgramDefinitions.h"
#include <Engine.World.Render/simpleRaster2/simpleRaster2ShaderProgramsDefs.h>
#include <Engine.Pipeline/Driver/PipelineRegistrar.h>
#include <Engine.Render.Pipeline/Settings.h>
#include <Engine.Shaders.Pipeline/Compile/ShaderCompileTypes.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>
#include <Engine.Render/Core/CommonGeometryDeclarations.h>

namespace fb
{

FB_IMPLEMENT_SHADERPROGRAMSPROVIDER(simpleRaster2ShaderProgramsProvider);

uint simpleRaster2ShaderProgramsProvider::getShaderProgramDefinitionsVersion() const
{
	uint version = makeVersionNumber("simpleRaster2", 1338);

	return version;
}

void simpleRaster2ShaderProgramsProvider::gatherShaderProgramDefinitions(
	ShaderRenderPath path,
	Platform platform,
	const ShaderProgramDatabasePipelineParams* params,
	eastl::vector<ShaderProgramDef>& outPrograms) const
{

	// Draw Call: Rasterize
	{
		ShaderProgramDef program;
		program.shaderProgram =ShaderProgram_simpleRaster2_Rasterize_VSPS,
		program.source = "Systems/WorldRender/simpleRaster2/_VSPS_Rasterize.hlsl";
		program.vsName = "VSMain";
		program.psName = "PSMain";
		program.compileFlags = ShaderCompileFlag_None;
		program.geometryDeclaration.streams[0].classification = VertexElementClassification_PerVertex;
		program.geometryDeclaration.streams[0].stride = ;
		program.geometryDeclaration.elementCount = 0;
		program.geometryDeclaration.streamCount = 1;
		outPrograms.push_back(program);
	}
}

}
