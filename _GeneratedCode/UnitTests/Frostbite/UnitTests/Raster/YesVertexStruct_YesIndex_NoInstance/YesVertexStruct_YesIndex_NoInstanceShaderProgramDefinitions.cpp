// (c) Electronic Arts.  All Rights Reserved.

#include "YesVertexStruct_YesIndex_NoInstanceShaderProgramDefinitions.h"
#include <Engine.World.Render/YesVertexStruct_YesIndex_NoInstance/YesVertexStruct_YesIndex_NoInstanceShaderProgramsDefs.h>
#include <Engine.Pipeline/Driver/PipelineRegistrar.h>
#include <Engine.Render.Pipeline/Settings.h>
#include <Engine.Shaders.Pipeline/Compile/ShaderCompileTypes.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>
#include <Engine.Render/Core/CommonGeometryDeclarations.h>

namespace fb
{

FB_IMPLEMENT_SHADERPROGRAMSPROVIDER(YesVertexStruct_YesIndex_NoInstanceShaderProgramsProvider);

uint YesVertexStruct_YesIndex_NoInstanceShaderProgramsProvider::getShaderProgramDefinitionsVersion() const
{
	uint version = makeVersionNumber("YesVertexStruct_YesIndex_NoInstance", 1338);

	return version;
}

void YesVertexStruct_YesIndex_NoInstanceShaderProgramsProvider::gatherShaderProgramDefinitions(
	ShaderRenderPath path,
	Platform platform,
	const ShaderProgramDatabasePipelineParams* params,
	eastl::vector<ShaderProgramDef>& outPrograms) const
{

	// Draw Call: Rasterize
	{
		ShaderProgramDef program;
		program.shaderProgram =ShaderProgram_YesVertexStruct_YesIndex_NoInstance_Rasterize_VSPS,
		program.source = "Systems/WorldRender/YesVertexStruct_YesIndex_NoInstance/_VSPS_Rasterize.hlsl";
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
