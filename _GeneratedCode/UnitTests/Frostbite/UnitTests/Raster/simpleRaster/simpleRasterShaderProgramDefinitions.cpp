// (c) Electronic Arts.  All Rights Reserved.

#include "simpleRasterShaderProgramDefinitions.h"
#include <Engine.World.Render/simpleRaster/simpleRasterShaderProgramsDefs.h>
#include <Engine.Pipeline/Driver/PipelineRegistrar.h>
#include <Engine.Render.Pipeline/Settings.h>
#include <Engine.Shaders.Pipeline/Compile/ShaderCompileTypes.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>
#include <Engine.Render/Core/CommonGeometryDeclarations.h>

namespace fb
{

FB_IMPLEMENT_SHADERPROGRAMSPROVIDER(simpleRasterShaderProgramsProvider);

uint simpleRasterShaderProgramsProvider::getShaderProgramDefinitionsVersion() const
{
	uint version = makeVersionNumber("simpleRaster", 1338);

	return version;
}

void simpleRasterShaderProgramsProvider::gatherShaderProgramDefinitions(
	ShaderRenderPath path,
	Platform platform,
	const ShaderProgramDatabasePipelineParams* params,
	eastl::vector<ShaderProgramDef>& outPrograms) const
{

	// Draw Call: Rasterize
	{
		ShaderProgramDef program;
		program.shaderProgram =ShaderProgram_simpleRaster_Rasterize_VSPS,
		program.source = "Systems/WorldRender/simpleRaster/_VSPS_Rasterize.hlsl";
		program.vsName = "VSMain";
		program.psName = "PSMain";
		program.compileFlags = ShaderCompileFlag_None;
		program.geometryDeclaration.streams[0].classification = VertexElementClassification_PerVertex;
		program.geometryDeclaration.elements[0] = GeometryDeclarationDesc::Element(VertexElementUsage_Pos + 0, VertexElementFormat_Float3, 0, 0);
		program.geometryDeclaration.elements[1] = GeometryDeclarationDesc::Element(VertexElementUsage_Normal + 2, VertexElementFormat_Float3, 12, 0);
		program.geometryDeclaration.streams[0].stride = 24;
		program.geometryDeclaration.elementCount = 2;
		program.geometryDeclaration.streamCount = 1;
		outPrograms.push_back(program);
	}
}

}
