// (c) Electronic Arts.  All Rights Reserved.
#pragma once

#include <EASTL/vector.h>
#include <Engine.Render/Core/ShaderPrograms.h>
#include <Engine.Render.Pipeline/ShaderProgramDefinitions.h>
#include <Engine.World.Pipeline/Module.h>

namespace fb
{

class SubGraphTestShaderProgramsProvider : public IShaderProgramsProvider
{
	FB_DECLARE_SHADERPROGRAMSPROVIDER(SubGraphTestShaderProgramsProvider, IShaderProgramsProvider, WorldPipeline);
public:
	uint getShaderProgramDefinitionsVersion() const override;
	void gatherShaderProgramDefinitions(ShaderRenderPath path, Platform platform, const ShaderProgramDatabasePipelineParams* params, eastl::vector<ShaderProgramDef>& outPrograms) const override;
};

}
