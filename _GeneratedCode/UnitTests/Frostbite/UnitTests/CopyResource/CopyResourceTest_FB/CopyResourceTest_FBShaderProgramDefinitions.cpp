// (c) Electronic Arts.  All Rights Reserved.

#include "CopyResourceTest_FBShaderProgramDefinitions.h"
#include <Engine.World.Render/CopyResourceTest_FB/CopyResourceTest_FBShaderProgramsDefs.h>
#include <Engine.Pipeline/Driver/PipelineRegistrar.h>
#include <Engine.Render.Pipeline/Settings.h>
#include <Engine.Shaders.Pipeline/Compile/ShaderCompileTypes.h>
#include <Engine.World.Render/Raytrace/RaytraceShaderPrograms.h>
#include <Engine.Render/Core/CommonGeometryDeclarations.h>

namespace fb
{

FB_IMPLEMENT_SHADERPROGRAMSPROVIDER(CopyResourceTest_FBShaderProgramsProvider);

uint CopyResourceTest_FBShaderProgramsProvider::getShaderProgramDefinitionsVersion() const
{
	uint version = makeVersionNumber("CopyResourceTest_FB", 1338);

	return version;
}

void CopyResourceTest_FBShaderProgramsProvider::gatherShaderProgramDefinitions(
	ShaderRenderPath path,
	Platform platform,
	const ShaderProgramDatabasePipelineParams* params,
	eastl::vector<ShaderProgramDef>& outPrograms) const
{
}

}
