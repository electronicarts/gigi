// (c) Electronic Arts.  All Rights Reserved.

#include "YesVertexType_NoIndex_NoInstanceShaderProgramDefinitions.h"
#include <Engine.World.Render/YesVertexType_NoIndex_NoInstance/YesVertexType_NoIndex_NoInstanceShaderProgramsDefs.h>
#include <Engine.Pipeline/Driver/PipelineRegistrar.h>
#include <Engine.Render.Pipeline/Settings.h>
#include <Engine.Shaders.Pipeline/Compile/ShaderCompileTypes.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>
#include <Engine.Render/Core/CommonGeometryDeclarations.h>

namespace fb
{

FB_IMPLEMENT_SHADERPROGRAMSPROVIDER(YesVertexType_NoIndex_NoInstanceShaderProgramsProvider);

uint YesVertexType_NoIndex_NoInstanceShaderProgramsProvider::getShaderProgramDefinitionsVersion() const
{
	uint version = makeVersionNumber("YesVertexType_NoIndex_NoInstance", 1338);

	return version;
}

void YesVertexType_NoIndex_NoInstanceShaderProgramsProvider::gatherShaderProgramDefinitions(
	ShaderRenderPath path,
	Platform platform,
	const ShaderProgramDatabasePipelineParams* params,
	eastl::vector<ShaderProgramDef>& outPrograms) const
{

	// Draw Call: Rasterize
	{
		ShaderProgramDef program;
		program.shaderProgram =ShaderProgram_YesVertexType_NoIndex_NoInstance_Rasterize_VSPS,
		program.source = "Systems/WorldRender/YesVertexType_NoIndex_NoInstance/_VSPS_Rasterize.hlsl";
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
