///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

// clang-format off
#include "gigicompiler.h"

#include "Schemas/JSON.h"
#include "Schemas/Visitor.h"
#include "Backends/Shared.h"
#include "Backends/GraphViz.h"
#include "RenderGraph/Visitors.h"
#include "FlattenRenderGraph.h"
#include "SubGraphs.h"
// clang-format on

// Backend Run prototype functions
// clang-format off
#include "external/df_serialize/_common.h"
#define ENUM_ITEM(x, y) void RunBackend_##x(GigiBuildFlavor buildFlavor, RenderGraph& renderGraph);
#include "external/df_serialize/_fillunsetdefines.h"
#include "Schemas/BackendList.h"
// clang-format on

bool RemoveBackendData(RenderGraph& renderGraph)
{
    // Resolve all backend restrictions
    ResolveBackendRestrictions visitor(renderGraph);
    if (!Visit(renderGraph, visitor, "renderGraph"))
        return false;

    // Delete any file copies that aren't for this backend.
    renderGraph.fileCopies.erase(
        std::remove_if(renderGraph.fileCopies.begin(), renderGraph.fileCopies.end(),
            [&](FileCopy& copy)
            {
                return (copy.backends.backendFlags & (1 << (unsigned int)renderGraph.backend)) == 0;
            }
        ),
        renderGraph.fileCopies.end()
    );

    // Delete any variables that aren't for this backend.
    renderGraph.variables.erase(
        std::remove_if(renderGraph.variables.begin(), renderGraph.variables.end(),
            [&](Variable& variable)
            {
                return (variable.backends.backendFlags & (1 << (unsigned int)renderGraph.backend)) == 0;
            }
        ),
        renderGraph.variables.end()
    );

    // Delete any shaders that aren't for this backend
    renderGraph.shaders.erase(
        std::remove_if(renderGraph.shaders.begin(), renderGraph.shaders.end(),
            [&](Shader& shader)
            {
                return (shader.backends.backendFlags & (1 << (unsigned int)renderGraph.backend)) == 0;
            }
        ),
        renderGraph.shaders.end()
    );

    // Delete any nodes that aren't for this backend.
    // NOTE: Not sure what to do about connections at this point.
    // We could make everything that uses the output of this node use the source of those pins to "fix the breaks".
    // But, what if there is a different node slotted in for different platforms and the outputs shouldn't be fixed up?
    // May need a setting here, or there may be a more common usage, or we may enforce it to be one way or the other. Punting it for now though.
    renderGraph.nodes.erase(
        std::remove_if(renderGraph.nodes.begin(), renderGraph.nodes.end(),
            [&](RenderGraphNode& node)
            {
                return (GetNodeBackendFlags(node) & (1 << (unsigned int)renderGraph.backend)) == 0;
            }
        ),
        renderGraph.nodes.end()
    );

    // Delete any shader resources that aren't for this backend
    for (Shader& shader : renderGraph.shaders)
    {
        shader.resources.erase(
            std::remove_if(shader.resources.begin(), shader.resources.end(),
                [&](ShaderResource& resource)
                {
                    return (resource.backends.backendFlags & (1 << (unsigned int)renderGraph.backend)) == 0;
                }
            ),
            shader.resources.end()
        );
    }

    // Delete any connections in any compute or ray shaders which aren't for this backend
    for (RenderGraphNode& node_ : renderGraph.nodes)
    {
        switch (node_._index)
        {
            case RenderGraphNode::c_index_actionComputeShader:
            {
                RenderGraphNode_Action_ComputeShader& node = node_.actionComputeShader;

                node.connections.erase(
                    std::remove_if(node.connections.begin(), node.connections.end(),
                        [&](NodePinConnection& connection)
                        {
                            return (connection.backends.backendFlags & (1 << (unsigned int)renderGraph.backend)) == 0;
                        }
                    ),
                    node.connections.end()
                );
                break;
            }
            case RenderGraphNode::c_index_actionRayShader:
            {
                RenderGraphNode_Action_RayShader& node = node_.actionRayShader;

                node.connections.erase(
                    std::remove_if(node.connections.begin(), node.connections.end(),
                        [&](NodePinConnection& connection)
                        {
                            return (connection.backends.backendFlags & (1 << (unsigned int)renderGraph.backend)) == 0;
                        }
                    ),
                    node.connections.end()
                );
                break;
            }
            case RenderGraphNode::c_index_actionDrawCall:
            {
                RenderGraphNode_Action_DrawCall& node = node_.actionDrawCall;

                node.connections.erase(
                    std::remove_if(node.connections.begin(), node.connections.end(),
                        [&](NodePinConnection& connection)
                        {
                            return (connection.backends.backendFlags & (1 << (unsigned int)renderGraph.backend)) == 0;
                        }
                    ),
                    node.connections.end()
                );
                break;
            }
            case RenderGraphNode::c_index_actionSubGraph:
            {
                RenderGraphNode_Action_SubGraph& node = node_.actionSubGraph;

                node.connections.erase(
                    std::remove_if(node.connections.begin(), node.connections.end(),
                        [&](NodePinConnection& connection)
                        {
                            return (connection.backends.backendFlags & (1 << (unsigned int)renderGraph.backend)) == 0;
                        }
                    ),
                    node.connections.end()
                );
                break;
            }
            case RenderGraphNode::c_index_actionBarrier:
            {
                RenderGraphNode_Action_Barrier& node = node_.actionBarrier;

                node.connections.erase(
                    std::remove_if(node.connections.begin(), node.connections.end(),
                        [&](NodePinConnection& connection)
                        {
                            return (connection.backends.backendFlags & (1 << (unsigned int)renderGraph.backend)) == 0;
                        }
                    ),
                    node.connections.end()
                );
                break;
            }
        }
    }

    return true;
}

GigiCompileResult GigiCompile(GigiBuildFlavor buildFlavor, const std::string& jsonFile, const std::string& outputDir, void (*PostLoad)(RenderGraph&), RenderGraph* outRenderGraph, bool GENERATE_GRAPHVIZ_FLAG)
{
    Backend backend;
    if (!GigiBuildFlavorBackend(buildFlavor, backend))
    {
        Assert(false, "could not get backend for build flavor %s", EnumToString(buildFlavor));
        return GigiCompileResult::NoBackend;
    }

    if (outputDir.empty())
        ShowInfoMessage("Building %s as %s", jsonFile.c_str(), EnumToString(buildFlavor));
    else
        ShowInfoMessage("Building %s as %s, to %s...", jsonFile.c_str(), EnumToString(buildFlavor), outputDir.c_str());

    // Load the render graph
    RenderGraph renderGraph_;
    RenderGraph& renderGraph = outRenderGraph ? *outRenderGraph : renderGraph_;
    if (!ReadFromJSONFile(renderGraph, jsonFile.c_str()))
    {
        Assert(false, "could not load %s", jsonFile.c_str());
        return GigiCompileResult::CantLoadRenderGraph;
    }
    renderGraph.backend = backend;

    if (renderGraph.versionUpgraded)
        ShowInfoMessage("File Upgraded from %s to %s%s%s", renderGraph.versionUpgradedFrom.c_str(), renderGraph.version.c_str(), renderGraph.versionUpgradedMessage.empty() ? "" : ":\n", renderGraph.versionUpgradedMessage.c_str());

    // verify the version number
    if (renderGraph.version != GIGI_VERSION())
    {
        Assert(false, "%s is using version %s but this exe is version %s", jsonFile.c_str(), renderGraph.version.c_str(), GIGI_VERSION());
        return GigiCompileResult::WrongVersion;
    }

    // get the base directory of the render graph
    {
        std::string rgPath = jsonFile.c_str();
        int pos = (int)rgPath.find_last_of("\\/");
        if (pos != -1)
        {
            renderGraph.baseDirectory = rgPath.substr(0, rgPath.find_last_of("\\/"));
            renderGraph.baseDirectory += "/";
        }
        else
            renderGraph.baseDirectory = "./";
    }

    renderGraph.generateGraphVizFlag = GENERATE_GRAPHVIZ_FLAG;

    // Set the technique name to the file name if it's empty
    if (renderGraph.name.empty() || renderGraph.name == "Unnamed")
        renderGraph.name = std::filesystem::path(jsonFile).filename().replace_extension().string();

    // Inline SubGraph nodes
    {
        if (!InlineSubGraphs(renderGraph))
            return GigiCompileResult::InlineSubGraphs;
    }

    // Remove data not for the backend chosen
    {
        if (!RemoveBackendData(renderGraph))
            return GigiCompileResult::BackendData;
    }

    // Sanitize IDs
    {
        SanitizeVisitor visitor(renderGraph);
        if (!Visit(renderGraph, visitor, "renderGraph"))
            return GigiCompileResult::Sanitize;
    }

    // Make sure no two shaders write to the same output file
    {
        ShaderFileDuplicationVisitor visitor(renderGraph);
        if (!Visit(renderGraph, visitor, "renderGraph"))
            return GigiCompileResult::ShaderFileDuplication;
    }

    // Do a post load
    if (PostLoad)
        PostLoad(renderGraph);

    // Get data from shaders
    {
        ShaderDataVisitor visitor(renderGraph);
        if (!Visit(renderGraph, visitor, "renderGraph"))
            return GigiCompileResult::ShaderReflection;
    }

    // Do render graph validation
    {
        ValidationVisitor visitor(renderGraph);
        if (!Visit(renderGraph, visitor, "renderGraph"))
            return GigiCompileResult::Validation;
    }

    // Shader references need to be resolved first
    {
        ShaderReferenceFixupVisitor visitor(renderGraph);
        if (!Visit(renderGraph, visitor, "renderGraph"))
            return GigiCompileResult::ReferenceFixup;
    }

    // resolve the node references from names into indices
    {
        ReferenceFixupVisitor visitor(renderGraph);
        if (!Visit(renderGraph, visitor, "renderGraph"))
            return GigiCompileResult::ReferenceFixup;
    }

    // De-pluralize file copies
    {
        DepluralizeFileCopiesVisitor visitor;
        if (!Visit(renderGraph, visitor, "renderGraph"))
            return GigiCompileResult::DepluralizeFileCopies;
    }

    // Other error checks
    {
        ErrorCheckVisitor visitor;
        if (!Visit(renderGraph, visitor, "renderGraph"))
            return GigiCompileResult::ErrorCheck;
    }

    // Add node info to shaders as defines
    {
        AddNodeInfoToShadersVisitor visitor;
        if (!Visit(renderGraph, visitor, "renderGraph"))
            return GigiCompileResult::AddNodeInfoToShaders;
    }

    // Dflt fixup to make sure variable dflts are correctly formatted (like that floats have an f on the end)
    {
        DfltFixupVisitor visitor;
        if (!Visit(renderGraph, visitor, "renderGraph"))
            return GigiCompileResult::DfltFixup;
    }

    // Data fixup to simplify backend handling of data cases.
    {
        DataFixupVisitor visitor;
        if (!Visit(renderGraph, visitor, "renderGraph"))
            return GigiCompileResult::DataFixup;
    }

    // Calculate optimized flattened render graph
    OptimizeAndFlattenRenderGraph(renderGraph);

    // Save out the final render graph file
    //WriteToJSONFile(renderGraph, "Optimized.gg");

    // Run the backend code
    renderGraph.outputDirectory = outputDir;
    switch (renderGraph.backend)
    {
        // clang-format off
        #include "external/df_serialize/_common.h"
        #define ENUM_ITEM(x, y) case Backend::x: RunBackend_##x(buildFlavor, renderGraph); break;
        #include "external/df_serialize/_fillunsetdefines.h"
        #include "Schemas/BackendList.h"
        #undef ENUM_ITEM
        // clang-format on
    }

    ShowInfoMessage("Building complete");

    return GigiCompileResult::OK;
}
