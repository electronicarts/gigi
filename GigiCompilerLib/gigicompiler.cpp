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
#define ENUM_ITEM(x, y) void RunBackend_##x(GigiBuildFlavor buildFlavor, RenderGraph& renderGraph, GGUserFileLatest& ggUserFile);
#include "external/df_serialize/_fillunsetdefines.h"
#include "Schemas/BackendList.h"
// clang-format on

bool HandleOutputsToMultiInput(RenderGraph& renderGraph)
{
    struct OutputConnection
    {
        static inline size_t hash_combine(size_t A, size_t B)
        {
            return A ^ (0x9e3779b9 + (A << 6) + (A >> 2));
        }

        size_t operator()(const OutputConnection& key) const
        {
            return hash_combine(std::hash<int>()(key.nodeIndex), std::hash<int>()(key.pinIndex));
        }

        bool operator == (const OutputConnection& other) const
        {
            return
                nodeIndex == other.nodeIndex &&
                pinIndex == other.pinIndex;
        }

        int nodeIndex = -1;
        int pinIndex = -1;
    };

    struct InputConnection
    {
        int nodeIndex = -1;
        int pinIndex = -1;

        bool subtreeIsReadOnly = true;
    };

    std::unordered_map<OutputConnection, std::vector<InputConnection>, OutputConnection> outputConnections;

    // Gather output connections
    for (int nodeIndex = 0; nodeIndex < (int)renderGraph.nodes.size(); ++nodeIndex)
    {
        RenderGraphNode& node = renderGraph.nodes[nodeIndex];
        std::vector<FrontEndNodesNoCaching::PinInfo> pinInfos = FrontEndNodesNoCaching::GetPinInfo(renderGraph, node);
        int pinCount = (int)pinInfos.size();

        InputConnection inputConnection;
        inputConnection.nodeIndex = nodeIndex;

        for (int pinIndex = 0; pinIndex < pinCount; ++pinIndex)
        {
            const FrontEndNodesNoCaching::PinInfo& pinInfo = pinInfos[pinIndex];
            if (pinInfo.dstNode == nullptr || pinInfo.dstNode->empty() || pinInfo.dstPin == nullptr || pinInfo.dstPin->empty())
                continue;

            OutputConnection outputConnection;
            outputConnection.nodeIndex = FrontEndNodesNoCaching::GetNodeIndexByName(renderGraph, pinInfo.dstNode->c_str());

            Assert(outputConnection.nodeIndex != -1, "could not find node \"%s\"", pinInfo.dstNode->c_str(), pinInfo.dstPin->c_str());

            RenderGraphNode& dstNode = renderGraph.nodes[outputConnection.nodeIndex];
            outputConnection.pinIndex = FrontEndNodesNoCaching::GetPinIndexByName(renderGraph, dstNode, pinInfo.dstPin->c_str());

            Assert(outputConnection.pinIndex != -1, "could not find pin \"%s\" for node \"%s\"", pinInfo.dstPin->c_str(), pinInfo.dstNode->c_str());

            inputConnection.pinIndex = pinIndex;
            inputConnection.subtreeIsReadOnly = !FrontEndNodesNoCaching::DoesSubtreeWriteResource(renderGraph, node, pinIndex);

            outputConnections[outputConnection].push_back(inputConnection);
        }
    }

    // Every output pin that connects to multiple input pins needs to evaluated.
    // 1) If all paths only lead to read only operations, no changes are needed and we are done.
    // 2) If there are multiple paths that lead to read/write operations, a warning about arace condition needs to be issued.
    // 3) If some paths are read only and some are read write, we need to make a copy of the resource (via a resource node and acopy node) and...
    //   a) All read/write paths need to be parented against the "source" output pin of the copy node, so writes happen to the original resource.
    //   b) All read only paths need to be parented against the "dest" output pin of the copy node, so they the version of the resource they want to see.
    //
    // When the graph is flattened, it may turn out that the temporary resource and copy nodes aren't needed. The optimizing code will be able
    // to strip these away when possible and the optimizer deems it desirable.
    //
    std::ostringstream raceConditions;
    for (auto& it : outputConnections)
    {
        // If there is only one thing connected to an output pin, don't need to do anything
        std::vector<InputConnection>& inputConnections = it.second;
        if (inputConnections.size() <= 1)
            continue;

        // Count how many read only, and how many read/write connections we have
        int readOnlyCount = 0;
        int readWriteCount = 0;
        int readOnlyCountCanDuplicate = 0;
        for (const InputConnection& inputConnection : inputConnections)
        {
            if (inputConnection.subtreeIsReadOnly)
            {
                const RenderGraphNode& outputNode = renderGraph.nodes[inputConnection.nodeIndex];
                bool disallowDuplication = false;
                ExecuteOnActionNode(outputNode, [&inputConnection, &disallowDuplication](auto& node)
                    {
                        disallowDuplication = node.linkProperties[inputConnection.pinIndex].disallowDuplication;
                    }
                );
                if (!disallowDuplication)
                    readOnlyCountCanDuplicate++;

                readOnlyCount++;
            }
            else
                readWriteCount++;
        }

        // if we only have read only connections, everything is fine as is.
        if (readWriteCount == 0)
            continue;

        // Get the output node name and pin name
        std::string outputNodeName;
        std::string outputPinName;
        {
            const OutputConnection& outputConnection = it.first;
            RenderGraphNode& outputNode = renderGraph.nodes[outputConnection.nodeIndex];
            outputNodeName = GetNodeName(outputNode);
            outputPinName = FrontEndNodesNoCaching::GetPinInfo(renderGraph, outputNode)[outputConnection.pinIndex].srcPin;
        }

        // If we have multiple read write counts, we need to issue a warning about a race condition
        if (readWriteCount > 1)
        {
            raceConditions << "Node \"" << outputNodeName << "\", Output Pin \"" << outputPinName << "\" To:\n";

            for (const InputConnection& inputConnection : inputConnections)
            {
                if (inputConnection.subtreeIsReadOnly)
                    continue;

                RenderGraphNode& connectedNode = renderGraph.nodes[inputConnection.nodeIndex];
                raceConditions << "    Node \"" << GetNodeName(connectedNode) << "\", Input Pin \"" << FrontEndNodesNoCaching::GetPinInfo(renderGraph, connectedNode)[inputConnection.pinIndex].srcPin << "\"\n";
            }

            raceConditions << "\n";
        }

        // If we only have write connections, leave everything as is
        if (readOnlyCount == 0)
            continue;

        // If we don't have any connections that we can duplicate, that we want to duplicate, leave everything as is
        if (readOnlyCountCanDuplicate == 0)
            continue;

        // If we got here, we need to copy the resource to a temporary resource for the read access connections.
        // The write access connections will use the original resource.

        // Make a new resource node
        RenderGraphNode newResourceNode;
        {
            int resourceNodeIndex = FrontEndNodesNoCaching::GetRootNodeIndex(renderGraph, outputNodeName, outputPinName);
            newResourceNode = renderGraph.nodes[resourceNodeIndex];

            // Get a unique node name for the resource node
            int resourceNameIndex = -1;
            char resourceName[1024];
            do
            {
                resourceNameIndex++;
                sprintf_s(resourceName, "%s_%s_Copy_%i", outputNodeName.c_str(), outputPinName.c_str(), resourceNameIndex);
            }
            while (FrontEndNodesNoCaching::GetNodeIndexByName(renderGraph, resourceName) != -1);

            // Set the name
            ExecuteOnNode(newResourceNode, [resourceName](auto& node) { node.name = resourceName; });

            // Make sure the resource is transient, and the visibility is internal
            switch (newResourceNode._index)
            {
                case RenderGraphNode::c_index_resourceBuffer:
                {
                    newResourceNode.resourceBuffer.transient = true;
                    newResourceNode.resourceBuffer.visibility = ResourceVisibility::Internal;
                    break;
                }
                case RenderGraphNode::c_index_resourceTexture:
                {
                    newResourceNode.resourceTexture.transient = true;
                    newResourceNode.resourceTexture.visibility = ResourceVisibility::Internal;
                    break;
                }
                default:
                {
                    ShowErrorMessage("Unhandled node type in " __FUNCTION__" : %i", newResourceNode._index);
                    return false;
                }
            }
        }

        // Make a new copy resource node
        RenderGraphNode newCopyNode;
        {
            // Set the node up
            newCopyNode._index = RenderGraphNode::c_index_actionCopyResource;
            newCopyNode.actionCopyResource.source.node = outputNodeName;
            newCopyNode.actionCopyResource.source.pin = outputPinName;
            newCopyNode.actionCopyResource.dest.node = GetNodeName(newResourceNode);
            newCopyNode.actionCopyResource.dest.pin = "resource";

            // Get a unique node name for the copy node
            int copyNameIndex = -1;
            char copyName[1024];
            do
            {
                copyNameIndex++;
                sprintf_s(copyName, "Copy_%s_%s_%i", outputNodeName.c_str(), outputPinName.c_str(), copyNameIndex);
            }
            while (FrontEndNodesNoCaching::GetNodeIndexByName(renderGraph, copyName) != -1);

            // Set the name
            ExecuteOnNode(newCopyNode, [copyName](auto& node) { node.name = copyName; });
        }

        // Add the nodes to the render graph
        renderGraph.nodes.push_back(newResourceNode);
        renderGraph.nodes.push_back(newCopyNode);

        // All read/write connections should be parented to the "source" pin so they write the original resource.
        // All read only connections should be parented to the "dest" pin so they see the version they want to see in the copy
        for (const InputConnection& inputConnection : inputConnections)
        {
            RenderGraphNode& connectedNode = renderGraph.nodes[inputConnection.nodeIndex];
            std::vector<FrontEndNodesNoCaching::PinInfo> pinInfos = FrontEndNodesNoCaching::GetPinInfo(renderGraph, connectedNode);
            *pinInfos[inputConnection.pinIndex].dstNode = GetNodeName(newCopyNode);
            *pinInfos[inputConnection.pinIndex].dstPin = inputConnection.subtreeIsReadOnly ? "dest" : "source";
        }
    }

    // Output a warning if there were any race conditions
    std::string raceConditionsStr = raceConditions.str();
    if (!raceConditionsStr.empty())
        ShowWarningMessage("These node output pins lead to parallel writes. If the writes overlap, or a read in one path can be affected by a write in another path, this is a race condition since Gigi may execute the output pins in any order. It's possible this was done intentionally and that there is no race condition.\n%s", raceConditionsStr.c_str());

    return true;
}

static bool ShadersCompatibleForNoDuplicate(const Shader& A, const Shader& B)
{
    auto ArraysEqual = [](const auto& arrayA, const auto& arrayB) -> bool
    {
        if (arrayA.size() != arrayB.size())
            return false;
        for (size_t i = 0; i < arrayA.size(); ++i)
        {
            if (!(arrayA[i] == arrayB[i]))
                return false;
        }
        return true;
    };

    // Return false for any incompatibilites, true otherwise.
    if (
        A.fileName != B.fileName ||
        A.destFileName != B.destFileName ||
        A.language != B.language ||
        A.type != B.type ||
        A.entryPoint != B.entryPoint ||
        A.NumThreads[0] != B.NumThreads[0] ||
        A.NumThreads[1] != B.NumThreads[1] ||
        A.NumThreads[2] != B.NumThreads[2] ||
        A.slangOptions != B.slangOptions ||
        !ArraysEqual(A.defines, B.defines) ||
        !ArraysEqual(A.tokenReplacements, B.tokenReplacements) ||
        !ArraysEqual(A.resources, B.resources) ||
        !ArraysEqual(A.samplers, B.samplers) ||
        !ArraysEqual(A.variableAliases, B.variableAliases)
    )
    {
        return false;
    }

    return true;
}

bool DuplicateNodeShaders(RenderGraph& renderGraph)
{
    std::unordered_set<std::string> usedShaders;

    auto HandleNodeShader = [&renderGraph, &usedShaders](std::string& shaderName, std::vector<ShaderDefine>& nodeDefines, std::string& nodeEntryPoint)
    {
        if (nodeDefines.empty() && nodeEntryPoint.empty())
        {
            usedShaders.insert(shaderName);
            return;
        }

        int shaderIndex = GetShaderIndex(renderGraph, shaderName.c_str());
        if (shaderIndex == -1)
            return;

        int uniqueShaderIndex = 0;
        Shader newShader = renderGraph.shaders[shaderIndex];
        newShader.name = FrontEndNodesNoCaching::GetUniqueShaderName(renderGraph, newShader.name.c_str(), &uniqueShaderIndex);

        if (!nodeEntryPoint.empty())
        {
            newShader.entryPoint = nodeEntryPoint;
            nodeEntryPoint = "";
        }

        if (!nodeDefines.empty())
        {
            newShader.defines.insert(newShader.defines.end(), nodeDefines.begin(), nodeDefines.end());
            nodeDefines.clear();
        }

        bool makeOutputFile = false;
        if (newShader.language == ShaderLanguage::Slang || renderGraph.backend == Backend::WebGPU)
            makeOutputFile = true;

        // If we need a unique processed shader file...
        // Make the output file name include the unique index used in the shader data record name, so the filename is unique too.
        if (makeOutputFile)
        {
            std::filesystem::path fileName = std::filesystem::path((newShader.destFileName.empty()) ? newShader.fileName : newShader.destFileName);

            std::ostringstream s;
            s << fileName.stem().string() << "_" << uniqueShaderIndex << fileName.extension().string();

            newShader.destFileName = std::filesystem::path(fileName).replace_filename(s.str()).string();
        }
        // Else, don't copy the shader file, use the same source
        else
        {
            newShader.copyFile = false;

            // If a shader record like the one we were going to make already exists, use that one instead
            for (const Shader& shader : renderGraph.shaders)
            {
                if (ShadersCompatibleForNoDuplicate(shader, newShader))
                {
                    shaderName = shader.name;
                    usedShaders.insert(shaderName);
                    return;
                }
            }
        }

        renderGraph.shaders.push_back(newShader);
        shaderName = newShader.name;
        usedShaders.insert(newShader.name);
    };

    // Node defines are per node, not per shader, even though the shader may be shared by multiple nodes.
    // A solution to copy the shader records and turn the node defines into shader defines.
    // Same with entry point.
    for (RenderGraphNode& nodeBase : renderGraph.nodes)
    {
        switch (nodeBase._index)
        {
            case RenderGraphNode::c_index_actionComputeShader:
            {
                RenderGraphNode_Action_ComputeShader& node = nodeBase.actionComputeShader;
                HandleNodeShader(node.shader.name, node.defines, node.entryPoint);
                break;
            }
            case RenderGraphNode::c_index_actionRayShader:
            {
                RenderGraphNode_Action_RayShader& node = nodeBase.actionRayShader;
                HandleNodeShader(node.shader.name, node.defines, node.entryPoint);
                break;
            }
            case RenderGraphNode::c_index_actionDrawCall:
            {
                RenderGraphNode_Action_DrawCall& node = nodeBase.actionDrawCall;

                std::string dummyEntryPoint;

                std::vector<ShaderDefine> nodeDefines = node.defines;
                HandleNodeShader(node.amplificationShader.name, nodeDefines, dummyEntryPoint);

                nodeDefines = node.defines;
                HandleNodeShader(node.meshShader.name, nodeDefines, dummyEntryPoint);

                nodeDefines = node.defines;
                HandleNodeShader(node.vertexShader.name, nodeDefines, dummyEntryPoint);

                nodeDefines = node.defines;
                HandleNodeShader(node.pixelShader.name, nodeDefines, dummyEntryPoint);

                node.defines.clear();

                break;
            }
        }
    }

    // Remove any shader data record that is no longer used, or was not used.
    // We only looked at compute, ray gen, and rasterize, so limit to those types.
    {
        renderGraph.shaders.erase(
            std::remove_if(
                renderGraph.shaders.begin(),
                renderGraph.shaders.end(),
                [&](const Shader& shader) -> bool
                {

                    switch (shader.type)
                    {
                        case ShaderType::Compute:
                        case ShaderType::RTRayGen:
                        case ShaderType::Amplification:
                        case ShaderType::Mesh:
                        case ShaderType::Vertex:
                        case ShaderType::Pixel:
                        {
                            break;
                        }
                        default: return false;
                    }

                    return usedShaders.count(shader.name) == 0;
                }
            ),
            renderGraph.shaders.end()
        );
    }

    // Make sure at least one of each file name is marked as copyFile.
    {
        std::unordered_set<std::string> copiedShaderFiles;

        for (Shader& shader : renderGraph.shaders)
        {
            std::string actualFileName = (shader.destFileName.empty()) ? shader.fileName : shader.destFileName;

            if (copiedShaderFiles.count(actualFileName) > 0)
                continue;

            shader.copyFile = true;
            copiedShaderFiles.insert(actualFileName);
        }
    }

    return true;
}

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
			case RenderGraphNode::c_index_reroute:
			{
				RenderGraphNode_Reroute& node = node_.reroute;

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

    std::filesystem::path absoluteOutputDir = std::filesystem::weakly_canonical(std::filesystem::path("./")) / std::filesystem::path(outputDir);
    ShowInfoMessage("Building %s as %s, to %s...", jsonFile.c_str(), EnumToString(buildFlavor), absoluteOutputDir.string().c_str());

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
    /*
    if (renderGraph.version != GIGI_VERSION())
    {
        Assert(false, "%s is using version %s but this exe is version %s", jsonFile.c_str(), renderGraph.version.c_str(), GIGI_VERSION());
        return GigiCompileResult::WrongVersion;
    }
    */

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

    renderGraph.generateGraphVizFlag = GENERATE_GRAPHVIZ_FLAG || renderGraph.buildSettings.makeGraphViz;

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

    // Add node info to shaders as defines
    {
        AddNodeInfoToShadersVisitor visitor(renderGraph);
        if (!Visit(renderGraph, visitor, "renderGraph"))
            return GigiCompileResult::AddNodeInfoToShaders;
    }

    // Duplicate shaders as needed.
    // Node defines are per node, not per shader, even though the shader may be shared by multiple nodes.
    // A solution to copy the shader records and turn the node defines into shader defines.
    {
        if (!DuplicateNodeShaders(renderGraph))
            return GigiCompileResult::DuplicateNodeShaders;
    }

    // Do a post load
    if (PostLoad)
        PostLoad(renderGraph);

    // Use temporary resources and copy nodes to make it so 1 output goes to only 1 input.
    // A make shift step towards static single assignment.
    {
        if (!HandleOutputsToMultiInput(renderGraph))
            return GigiCompileResult::HandleOutputsToMultiInput;
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

    // Shader references need to be resolved first
    {
        ShaderReferenceFixupVisitor visitor(renderGraph);
        if (!Visit(renderGraph, visitor, "renderGraph"))
            return GigiCompileResult::ReferenceFixup;
    }

    // Process Asserts declarations inside shaders
    if (backend == Backend::Interpreter)
    {
       ShaderAssertsVisitor visitor{ renderGraph };
       if (!Visit(renderGraph, visitor, "renderGraph"))
           return GigiCompileResult::ShaderAsserts;
    }

    // Get data from shaders
    {
        ShaderDataVisitor visitor(renderGraph, backend);
        if (!Visit(renderGraph, visitor, "renderGraph"))
            return GigiCompileResult::ShaderReflection;
    }

    // Do render graph validation
    {
        ValidationVisitor visitor(renderGraph);
        if (!Visit(renderGraph, visitor, "renderGraph"))
            return GigiCompileResult::Validation;
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

    // Dflt fixup to make sure variable dflts are correctly formatted (like that floats have an f on the end)
    {
        DfltFixupVisitor visitor(backend);
        if (!Visit(renderGraph, visitor, "renderGraph"))
            return GigiCompileResult::DfltFixup;
    }

    // Data fixup to simplify backend handling of data cases.
    {
        DataFixupVisitor visitor(renderGraph, backend);
        if (!Visit(renderGraph, visitor, "renderGraph"))
            return GigiCompileResult::DataFixup;
    }

    // Calculate optimized flattened render graph
    OptimizeAndFlattenRenderGraph(renderGraph);

    // Save out the final render graph file
    //WriteToJSONFile(renderGraph, "Optimized.gg");

    // Load the .gguser file if present
    GGUserFileLatest ggUserFile;
    {
        std::filesystem::path p(jsonFile);
        p.replace_extension(".gguser");
        if (ReadFromJSONFile(ggUserFile, p.string().c_str(), false))
            ShowInfoMessage("Loaded gg user file: %s", p.string().c_str());
        else
            ShowInfoMessage("Could not load gg user file, used default gguser data: %s", p.string().c_str());
    }

    // Run the backend code
    renderGraph.outputDirectory = absoluteOutputDir.string();
    switch (renderGraph.backend)
    {
        // clang-format off
        #include "external/df_serialize/_common.h"
        #define ENUM_ITEM(x, y) case Backend::x: RunBackend_##x(buildFlavor, renderGraph, ggUserFile); break;
        #include "external/df_serialize/_fillunsetdefines.h"
        #include "Schemas/BackendList.h"
        #undef ENUM_ITEM
        // clang-format on
    }

    ShowInfoMessage("Building complete");

    return GigiCompileResult::OK;
}

/*
TODO:
* Replace external node type with a variant

* have each DLL node type be able to specify:
 * connection pins
 * settings -> that can either be set to variables, or constants, which const variables are made for, behind the scenes?

? maybe the const things on nodes (like displayed name) should not be const in the DLL node and should be set when they are instantiated, if code doesn't need to access it statically

* In Gigi compiler, error if a DLL node is used but the platform doesn't support it, so it never even gets to the backend.  Need some centralized function to get DLL node info.

* Make this call into Fidelity FX DLL(s) as an example & some good initial thing. Maybe put demos in the public browser after it goes out. Could port the AMD demos from their sample.

* get approval for OSS includement of fidelity FX and libffi. I think both are MIT. verify. (not libffi. you removed that)

* make a unit test that uses a DLL - unit test for the AMD things.

* make sure this works for the installer / binaries too. The right files need to be put into the binaries and installer.

? what are all the DLL node types we should have?  https://github.com/GPUOpen-LibrariesAndSDKs/FidelityFX-SDK?tab=readme-ov-file

*/