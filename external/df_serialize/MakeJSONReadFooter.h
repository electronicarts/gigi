
// Specialize this function to implement post load work, such as version upgrade fixups
template <typename T>
bool ReadFromJSON_PostLoad(T& value)
{
    return true;
}

inline bool LoadTextFile(const char* fileName, TDYNAMICARRAY<char>& data)
{
    // open the file if we can
    FILE* file = nullptr;
    fopen_s(&file, fileName, "rb");
    if (!file)
        return false;

    // get the file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // read the file into memory and return success.
    // don't forget the null terminator
    TDYNAMICARRAY_RESIZE(data, size + 1);
    fread(&data[0], 1, size, file);
    fclose(file);
    data[TDYNAMICARRAY_SIZE(data) - 1] = 0;
    return true;
}

// Read a structure from a JSON string
template<typename TROOT>
bool ReadFromJSONBuffer(TROOT& root, const char* data)
{
    rapidjson::Document document;
    rapidjson::ParseResult ok = document.Parse<rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag>(data);
    if (!ok)
    {
        size_t errorOffset = ok.Offset();

        // count what line we are on
        int lineNumber = 1;
        {
            size_t index = errorOffset;
            while (index > 0)
            {
                if (data[index] == '\n')
                    lineNumber++;
                index--;
            }
        }

        // back up to the beginning of the line
        while (errorOffset > 0 && data[errorOffset] != '\n')
            errorOffset--;
        if (errorOffset > 0)
            errorOffset++;

        // get the next couple lines from the error
        size_t end = errorOffset;
        for (int i = 0; i < 4; ++i)
        {
            while (data[end] != 0 && data[end] != '\n')
                end++;
            if (data[end] != 0)
                end++;
        }

        TSTRING s;
        TSTRING_RESIZE(s, end - errorOffset + 1);
        memcpy(&s[0], &data[errorOffset], end - errorOffset);
        s[end - errorOffset] = 0;

        DFS_LOG("JSON parse error line %i\n%s\n%s\n", lineNumber, GetParseError_En(ok.Code()), &s[0]);
        return false;
    }

    return JSONRead(root, document) && ReadFromJSON_PostLoad(root);
}

template<typename TROOT>
bool ReadFromJSONBuffer(TROOT& root, const TDYNAMICARRAY<char>& data)
{
    return ReadFromJSONBuffer(root, &data[0]);
}

template<typename TROOT>
bool ReadFromJSONBuffer(TROOT& root, const TSTRING& data)
{
    return ReadFromJSONBuffer(root, &data[0]);
}

// Read a structure from a JSON file
template<typename TROOT>
bool ReadFromJSONFile(TROOT& root, const char* fileName, bool fileMustExist = true)
{
    TDYNAMICARRAY<char> fileData;
    if (!LoadTextFile(fileName, fileData))
    {
        if (fileMustExist)
            DFS_LOG("Could not read file %s", fileName);
        return false;
    }

    return ReadFromJSONBuffer(root, fileData);
}

// version fixup
inline bool ReadFromJSON_PostLoad(RenderGraph& renderGraph)
{
    if (renderGraph.version != GIGI_VERSION())
    {
        renderGraph.versionUpgraded = true;
        renderGraph.versionUpgradedFrom = renderGraph.version;
    }

    while (renderGraph.version != GIGI_VERSION())
    {
        if (renderGraph.version == "0.9b")
        {
            // An implicit hit group was made for each closest hit shader, but 0.91b got rid of that.
            // Make explicit hit groups here for each closest hit shader.
            bool groupAdded = false;
            int hitGroupNameIndex = 0;
            for (const Shader& shader : renderGraph.shaders)
            {
                if (shader.type != ShaderType::RTClosestHit)
                    continue;

                // make a unique name for this hit group
                char hitGroupName[256];
                bool hitGroupNameExists = true;
                do
                {
                    sprintf_s(hitGroupName, "HitGroup%i", hitGroupNameIndex);

                    hitGroupNameExists = false;
                    for (const RTHitGroup& hitGroup : renderGraph.hitGroups)
                    {
                        if (hitGroup.name == hitGroupName)
                        {
                            hitGroupNameExists = true;
                            break;
                        }
                    }
                    hitGroupNameIndex++;
                }
                while(hitGroupNameExists);

                // Create a new hit group for this shader
                {
                    RTHitGroup newHitGroup;
                    newHitGroup.name = hitGroupName;
                    newHitGroup.closestHit.name = shader.name;
                    renderGraph.hitGroups.push_back(newHitGroup);
                }

                // remember that we added a group
                groupAdded = true;
            }

            // make a message about what was upgraded, and what is expected, for the user to act on it
            if (groupAdded)
                renderGraph.versionUpgradedMessage += "One or more RTClosestHit shaders had explicit hit groups created for them. Shaders which use the old style _chindex_<ShaderName> "
                "variable names in the ray tracing call will be broken until they are updated to use the newer _hgindex_<HitGroupName>.\n";

            // upgrade complete
            renderGraph.version = "0.91b";
        }
        else if (renderGraph.version == "0.91b")
        {
            // since CSNumThreads is used to calculate dispatch, set it to (1,1,1) for any exsisting compute shaders, to give the same result
            bool updated = false;
            for (Shader& shader : renderGraph.shaders)
            {
                if (shader.type != ShaderType::Compute)
                    continue;

                shader.CSNumThreads[0] = 1;
                shader.CSNumThreads[1] = 1;
                shader.CSNumThreads[2] = 1;
                updated = true;
            }

            if (updated)
                renderGraph.versionUpgradedMessage += "One or more Compute shaders has had their CSNumThreads set to (1,1,1) to keep the same functionality.\n";

            // upgrade complete
            renderGraph.version = "0.92b";
        }
        else if (renderGraph.version == "0.92b")
        {
            bool usesRaytracingShaders = false;
            for (const RenderGraphNode& node : renderGraph.nodes)
            {
                if (node._index == RenderGraphNode::c_index_actionRayShader)
                {
                    usesRaytracingShaders = true;
                    break;
                }
            }

            if (usesRaytracingShaders)
            {
                renderGraph.versionUpgradedMessage +=
                    "Variables _hgindex_<name>, _rt_hit_group_count and _missindex_<name> in ray shaders is no longer supported.\n"
                    "Please use /*$(RTHitGroupIndex:<name>)*/, /*$(RTHitGroupCount)*/ and /*$(RTMissIndex:<name>)*/ instead.\n"
                    ;
            }

            // upgrade complete
            renderGraph.version = "0.93b";
        }
        else if (renderGraph.version == "0.93b")
        {
            for (Variable& variable : renderGraph.variables)
                variable.UISettings.UIHint = variable.UIHint;

            // upgrade complete
            renderGraph.version = "0.94b";
        }
        else if (renderGraph.version == "0.94b")
        {
            // upgrade complete. Just wanted to bump the version number for the initial frostbite code generation work.
            renderGraph.version = "0.95b";
        }
        else if (renderGraph.version == "0.95b")
        {
            // renamed cause it's for more than CS. also mesh shaders, ray gen on consoles, etc.
            for (Shader& shader : renderGraph.shaders)
                shader.NumThreads = shader.CSNumThreads;

            // upgrade complete
            renderGraph.version = "0.96b";
        }
        else if (renderGraph.version == "0.96b")
        {
            renderGraph.settings.dx12.shaderCompiler = DXShaderCompiler::DXC;

            if (renderGraph.settings.dx12.shaderModelCs == "cs_5_1")
                renderGraph.settings.dx12.shaderModelCs = "cs_6_1";

            if (renderGraph.settings.dx12.shaderModelVs == "vs_5_1")
                renderGraph.settings.dx12.shaderModelVs = "vs_6_1";

            if (renderGraph.settings.dx12.shaderModelPs == "ps_5_1")
                renderGraph.settings.dx12.shaderModelPs = "ps_6_1";

            renderGraph.versionUpgradedMessage += "DX12 shader compiler default changed from FXC to DXC, and the shader model for VS, PS, CS has changed from 5_1 to 6_1.  If that breaks your technique you can switch it back. Apologies if so! Changing the defaults to make more things work correctly by default.\n";
            renderGraph.version = "0.97b";
        }
        else if (renderGraph.version == "0.97b")
        {
            for (RenderGraphNode& node : renderGraph.nodes)
            {
                if (node._index != RenderGraphNode::c_index_actionSubGraph)
                    continue;

                for (SubGraphVariableSettings& varSettings : node.actionSubGraph.variableSettings)
                    varSettings.replaceWithStr = varSettings.replaceWith.name;
            }
            renderGraph.version = "0.98b";
        }
        else if (renderGraph.version == "0.98b")
        {
            // changing default behavior with this version, but want old versions to be unaffected.
            // PODAsStructuredBuffer has a default of true, but old versions acted as if it was false.
            for (Shader& shader : renderGraph.shaders)
            {
                for (ShaderResource& resource : shader.resources)
                    resource.buffer.PODAsStructuredBuffer = false;
            }
            renderGraph.version = "0.99b";
        }
        else
        {
            return false;
        }
    }

    return true;
}