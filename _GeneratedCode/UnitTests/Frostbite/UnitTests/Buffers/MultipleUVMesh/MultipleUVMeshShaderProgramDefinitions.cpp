// (c) Electronic Arts.  All Rights Reserved.

#include "MultipleUVMeshShaderProgramDefinitions.h"
#include <Engine.World.Render/MultipleUVMesh/MultipleUVMeshShaderProgramsDefs.h>
#include <Engine.Pipeline/Driver/PipelineRegistrar.h>
#include <Engine.Render.Pipeline/Settings.h>
#include <Engine.Shaders.Pipeline/Compile/ShaderCompileTypes.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>
#include <Engine.Render/Core/CommonGeometryDeclarations.h>

namespace fb
{

FB_IMPLEMENT_SHADERPROGRAMSPROVIDER(MultipleUVMeshShaderProgramsProvider);

uint MultipleUVMeshShaderProgramsProvider::getShaderProgramDefinitionsVersion() const
{
	uint version = makeVersionNumber("MultipleUVMesh", 1338);

	return version;
}

void MultipleUVMeshShaderProgramsProvider::gatherShaderProgramDefinitions(
	ShaderRenderPath path,
	Platform platform,
	const ShaderProgramDatabasePipelineParams* params,
	eastl::vector<ShaderProgramDef>& outPrograms) const
{
}

}
