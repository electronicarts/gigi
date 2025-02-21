///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "SubGraphs.h"

#include "gigicompiler.h"

#include "Schemas/JSON.h"
#include "Schemas/Visitor.h"
#include "Backends/Shared.h"
#include "Backends/GraphViz.h"
#include "RenderGraph/Visitors.h"
#include "FlattenRenderGraph.h"
#include <unordered_map>

#define INLINING_DEBUG() false // if true, prints info about inlining steps

#define INLINING_DEBUG_NODES() (true && INLINING_DEBUG())
#define INLINING_DEBUG_VARIABLES() (true && INLINING_DEBUG())

#if INLINING_DEBUG()
    #define INLINE_DEBUG(...) ShowInfoMessage(__VA_ARGS__);
    #if INLINING_DEBUG_NODES()
        #define INLINE_DEBUG_NODE(...) ShowInfoMessage(__VA_ARGS__);
    #endif
    #if INLINING_DEBUG_VARIABLES()
        #define INLINE_DEBUG_VARIABLE(...) ShowInfoMessage(__VA_ARGS__);
    #endif
#endif

#ifndef INLINE_DEBUG
#define INLINE_DEBUG(...)
#endif

#ifndef INLINE_DEBUG_NODE
#define INLINE_DEBUG_NODE(...)
#endif

#ifndef INLINE_DEBUG_VARIABLE
#define INLINE_DEBUG_VARIABLE(...)
#endif

static bool NodeNameExists(const RenderGraph& renderGraph, const char* name)
{
    for (const RenderGraphNode& baseNode : renderGraph.nodes)
    {
        bool matches = false;
        RenderGraphNodeLambda(baseNode,
            [&matches, name](auto& node)
            {
                matches = !_stricmp(name, node.name.c_str());
            }
        );
        if (matches)
            return true;
    }

    return false;
}

static bool VariableNameExists(const RenderGraph& renderGraph, const char* name)
{
    for (const Variable& variable : renderGraph.variables)
    {
        if (!_stricmp(name, variable.name.c_str()))
            return true;
    }
    return false;
}

static bool StructNameExists(const RenderGraph& renderGraph, const char* name)
{
    for (const Struct& s : renderGraph.structs)
    {
        if (!_stricmp(name, s.name.c_str()))
            return true;
    }
    return false;
}

static bool ShaderNameExists(const RenderGraph& renderGraph, const char* name)
{
    for (const Shader& s : renderGraph.shaders)
    {
        if (!_stricmp(name, s.name.c_str()))
            return true;
    }
    return false;
}

static bool EnumNameExists(const RenderGraph& renderGraph, const char* name)
{
    for (const Enum& e : renderGraph.enums)
    {
        if (!_stricmp(name, e.name.c_str()))
            return true;
    }
    return false;
}

static bool HitGroupNameExists(const RenderGraph& renderGraph, const char* name)
{
    for (const RTHitGroup& hitGroup : renderGraph.hitGroups)
    {
        if (!_stricmp(name, hitGroup.name.c_str()))
            return true;
    }
    return false;
}

struct NodePin
{
    std::string node;
    std::string pin;

    bool operator == (const NodePin& rhs) const
    {
        return node == rhs.node && pin == rhs.pin;
    }
};

template <>
struct std::hash<NodePin>
{
    size_t operator()(const NodePin& nodePin) const
    {
        // Portions of this software were based on http://stackoverflow.com/a/1646913/126995
        size_t ret = 17;
        ret = ret * 31 + hash<std::string>()(nodePin.node);
        ret = ret * 31 + hash<std::string>()(nodePin.pin);
        return ret;
    }
};

struct RenameData
{
    std::unordered_map<NodePin, NodePin> m_nodePinRenames;

    std::unordered_map<std::string, std::string> m_nodeRenames;
    std::unordered_map<std::string, std::string> m_structRenames;
    std::unordered_map<std::string, std::string> m_variableRenames;
    std::unordered_map<std::string, std::string> m_shaderRenames;
    std::unordered_map<std::string, std::string> m_enumRenames;
    std::unordered_map<std::string, std::string> m_hitGroupRenames;

    bool UpdateNodeName(std::string& name) const
    {
        return Rename(name, m_nodeRenames);
    }

    bool UpdateStructName(std::string& name) const
    {
        return Rename(name, m_structRenames);
    }

    bool UpdateVariableName(std::string& name) const
    {
        return Rename(name, m_variableRenames);
    }

    bool UpdateShaderName(std::string& name) const
    {
        return Rename(name, m_shaderRenames);
    }

    bool UpdateEnumName(std::string& name) const
    {
        return Rename(name, m_enumRenames);
    }

    bool UpdateNodePin(std::string& node, std::string& pin)
    {
        NodePin np;
        np.node = node;
        np.pin = pin;

        auto it = m_nodePinRenames.find(np);
        if (it == m_nodePinRenames.end())
            return false;

        node = it->second.node;
        pin = it->second.pin;
        return true;
    }

private:

    static bool Rename(std::string& name, const std::unordered_map<std::string, std::string>& map)
    {
        if (name.empty())
            return true;

        auto it = map.find(name);
        if (it == map.end())
            return false;

        name = it->second;
        return true;
    }
};

// update connections and references now that everything is renamed
struct RenameReferencesVisitor
{
    RenameReferencesVisitor(RenameData& renameData)
        : m_renameData(renameData)
    {

    }

    template <typename TDATA>
    bool Visit(TDATA& data, const std::string& path)
    {
        return true;
    }

    bool Visit(SetCBFromVar& setVar, const std::string& path)
    {
        m_renameData.UpdateVariableName(setVar.variable.name);
        return true;
    }

    bool Visit(Condition& condition, const std::string& path)
    {
        m_renameData.UpdateVariableName(condition.variable1);
        m_renameData.UpdateVariableName(condition.variable2);
        return true;
    }

    bool Visit(SetVariable& setVariable, const std::string& path)
    {
        m_renameData.UpdateVariableName(setVariable.destination.name);
        m_renameData.UpdateVariableName(setVariable.AVar.name);
        m_renameData.UpdateVariableName(setVariable.BVar.name);
        return true;
    }

    bool Visit(RTHitGroup& hitGroup, const std::string& path)
    {
        m_renameData.UpdateShaderName(hitGroup.closestHit.name);
        m_renameData.UpdateShaderName(hitGroup.anyHit.name);
        m_renameData.UpdateShaderName(hitGroup.intersection.name);
        return true;
    }

    bool Visit(RenderGraphNode& nodeBase, const std::string& path)
    {
        switch (nodeBase._index)
        {
            case RenderGraphNode::c_index_resourceBuffer:
            {
                RenderGraphNode_Resource_Buffer& node = nodeBase.resourceBuffer;

                m_renameData.UpdateNodeName(node.format.node.name);
                m_renameData.UpdateStructName(node.format.structureType.name);

                m_renameData.UpdateNodeName(node.count.node.name);
                m_renameData.UpdateVariableName(node.count.variable.name);
                return true;
            }
            case RenderGraphNode::c_index_resourceShaderConstants:
            {
                ShowErrorMessage("Shader constant resource nodes should not exist at this point. They should only be created later by analyzing the shaders. " __FUNCTION__);
                return false;
            }
            case RenderGraphNode::c_index_resourceTexture:
            {
                RenderGraphNode_Resource_Texture& node = nodeBase.resourceTexture;

                m_renameData.UpdateNodeName(node.format.node.name);
                m_renameData.UpdateVariableName(node.format.variable.name);

                m_renameData.UpdateNodeName(node.size.node.name);
                m_renameData.UpdateVariableName(node.size.variable.name);
                return true;
            }
            case RenderGraphNode::c_index_actionComputeShader:
            {
                RenderGraphNode_Action_ComputeShader& node = nodeBase.actionComputeShader;

                m_renameData.UpdateShaderName(node.shader.name);

                m_renameData.UpdateNodeName(node.dispatchSize.node.name);
                m_renameData.UpdateVariableName(node.dispatchSize.variable.name);

                m_renameData.UpdateNodePin(node.dispatchSize.indirectBuffer.node, node.dispatchSize.indirectBuffer.pin);
                m_renameData.UpdateNodeName(node.dispatchSize.indirectBuffer.node);

                for (NodePinConnection& connection : node.connections)
                {
                    m_renameData.UpdateNodePin(connection.dstNode, connection.dstPin);
                    m_renameData.UpdateNodeName(connection.dstNode);
                }

                return true;
            }
            case RenderGraphNode::c_index_actionRayShader:
            {
                RenderGraphNode_Action_RayShader& node = nodeBase.actionRayShader;

                m_renameData.UpdateShaderName(node.shader.name);

                m_renameData.UpdateNodeName(node.dispatchSize.node.name);
                m_renameData.UpdateVariableName(node.dispatchSize.variable.name);

                for (NodePinConnection& connection : node.connections)
                {
                    m_renameData.UpdateNodePin(connection.dstNode, connection.dstPin);
                    m_renameData.UpdateNodeName(connection.dstNode);
                }

                return true;
            }
            case RenderGraphNode::c_index_actionCopyResource:
            {
                RenderGraphNode_Action_CopyResource& node = nodeBase.actionCopyResource;

                m_renameData.UpdateNodePin(node.source.node, node.source.pin);
                m_renameData.UpdateNodePin(node.dest.node, node.dest.pin);

                m_renameData.UpdateNodeName(node.source.node);
                m_renameData.UpdateNodeName(node.dest.node);

                return true;
            }
            case RenderGraphNode::c_index_actionDrawCall:
            {
                RenderGraphNode_Action_DrawCall& node = nodeBase.actionDrawCall;

                m_renameData.UpdateShaderName(node.vertexShader.name);
                m_renameData.UpdateShaderName(node.pixelShader.name);

                m_renameData.UpdateNodePin(node.shadingRateImage.node, node.shadingRateImage.pin);
                m_renameData.UpdateNodePin(node.vertexBuffer.node, node.vertexBuffer.pin);
                m_renameData.UpdateNodePin(node.indexBuffer.node, node.indexBuffer.pin);
                m_renameData.UpdateNodePin(node.instanceBuffer.node, node.instanceBuffer.pin);

                m_renameData.UpdateNodeName(node.shadingRateImage.node);
                m_renameData.UpdateNodeName(node.vertexBuffer.node);
                m_renameData.UpdateNodeName(node.indexBuffer.node);
                m_renameData.UpdateNodeName(node.instanceBuffer.node);

                for (NodePinReferenceOptional& colorTarget : node.colorTargets)
                {
                    m_renameData.UpdateNodePin(colorTarget.node, colorTarget.pin);
                    m_renameData.UpdateNodeName(colorTarget.node);
                }

                m_renameData.UpdateNodePin(node.depthTarget.node, node.depthTarget.pin);
                m_renameData.UpdateNodeName(node.depthTarget.node);

                for (NodePinConnection& connection : node.connections)
                {
                    m_renameData.UpdateNodePin(connection.dstNode, connection.dstPin);
                    m_renameData.UpdateNodeName(connection.dstNode);
                }
                return true;
            }
            case RenderGraphNode::c_index_actionSubGraph:
            {
                RenderGraphNode_Action_SubGraph& node = nodeBase.actionSubGraph;

                for (NodePinConnection& connection : node.connections)
                {
                    m_renameData.UpdateNodePin(connection.dstNode, connection.dstPin);
                    m_renameData.UpdateNodeName(connection.dstNode);
                }
                return true;
            }
            case RenderGraphNode::c_index_actionBarrier:
            {
                RenderGraphNode_Action_Barrier& node = nodeBase.actionBarrier;

                for (NodePinConnection& connection : node.connections)
                {
                    m_renameData.UpdateNodePin(connection.dstNode, connection.dstPin);
                    m_renameData.UpdateNodeName(connection.dstNode);
                }

                return true;
            }
            case RenderGraphNode::c_index_reroute:
            {
                RenderGraphNode_Reroute& node = nodeBase.reroute;

				for (NodePinConnection& connection : node.connections)
				{
					m_renameData.UpdateNodePin(connection.dstNode, connection.dstPin);
					m_renameData.UpdateNodeName(connection.dstNode);
				}

				return true;
            }
            default:
            {
                ShowErrorMessage("Unhandled node type in Subgraphs.cpp " __FUNCTION__);
                return false;
            }
        }
        return true;
    }

    RenameData& m_renameData;
};

// Rename everything in the child graph to make it unique if inserted into the parent graph
struct RenameChildVisitor
{
    RenameChildVisitor(const RenderGraph& parentGraph, RenderGraph& childGraph, RenderGraphNode_Action_SubGraph& subGraphNode, RenameData& renameData)
        : m_parentGraph(parentGraph)
        , m_childGraph(childGraph)
        , m_subGraphNode(subGraphNode)
        , m_renameData(renameData)
    {

    }

    template <typename TDATA>
    bool Visit(TDATA& data, const std::string& path)
    {
        return true;
    }

    // Rename nodes
    bool Visit(RenderGraphNode_Base& node, const std::string& path)
    {
        // prepend the sub graph node name to make it unique.
        char newName[1024];
        sprintf_s(newName, "%s.%s", m_subGraphNode.name.c_str(), node.name.c_str());
        if (NodeNameExists(m_parentGraph, newName))
        {
            Assert(false, "Subgraph Node Name Collision");
            return false;
        }

        // remember this renaming
        m_renameData.m_nodeRenames[node.name] = newName;

        // renaming book keeping
        if (node.originalName.empty())
            node.originalName = node.name;

        // Set the new name
        node.name = newName;

        return true;
    }

    // Rename variables
    bool Visit(Variable& variable, const std::string& path)
    {
        // prepend the sub graph node name to make it unique.
        char newName[1024];
        sprintf_s(newName, "%s.%s", m_subGraphNode.name.c_str(), variable.name.c_str());
        if (VariableNameExists(m_parentGraph, newName))
        {
            Assert(false, "Subgraph Variable Name Collision");
            return false;
        }

        // remember this renaming
        m_renameData.m_variableRenames[variable.name] = newName;

        // renaming book keeping
        if (variable.originalName.empty())
            variable.originalName = variable.name;
        variable.scope = m_subGraphNode.name + "." + variable.scope;

        // Set the new name
        variable.name = newName;

        return true;
    }

    // Rename structs
    bool Visit(Struct& s, const std::string& path)
    {
        // prepend the sub graph node name and add numbers if needed, to make it unique.
        char newName[1024];
        int index = -1;
        do
        {
            if (index == -1)
                sprintf_s(newName, "%s.%s", m_subGraphNode.name.c_str(), s.name.c_str());
            else
                sprintf_s(newName, "%s.%s.%i", m_subGraphNode.name.c_str(), s.name.c_str(), index);
            index++;
        } while (StructNameExists(m_parentGraph, newName) || StructNameExists(m_childGraph, newName));

        // remember this renaming
        m_renameData.m_structRenames[s.name] = newName;

        // renaming book keeping
        if (s.originalName.empty())
            s.originalName = s.name;
        s.scope = m_subGraphNode.name + "." + s.scope;

        // Set the new name
        s.name = newName;

        return true;
    }

    // Rename shaders
    bool Visit(Shader& s, const std::string& path)
    {
        // Make sure this shader has a unique name
        // prepend the sub graph node name and add numbers if needed, to make it unique.
        char newName[1024];
        int index = -1;
        do
        {
            if (index == -1)
                sprintf_s(newName, "%s.%s", m_subGraphNode.name.c_str(), s.name.c_str());
            else
                sprintf_s(newName, "%s.%s.%i", m_subGraphNode.name.c_str(), s.name.c_str(), index);
            index++;
        } while (ShaderNameExists(m_parentGraph, newName) || ShaderNameExists(m_childGraph, newName));

        // remember this renaming
        m_renameData.m_shaderRenames[s.name] = newName;

        // renaming book keeping
        if (s.originalName.empty())
            s.originalName = s.name;
        s.scope = m_subGraphNode.name + "." + s.scope;

        // Set the new name
        s.name = newName;

        // Update where the file should be written out to.
        // Need to handle the subgraph possibly being in a parent directory etc.
        if (s.destFileName.empty())
            s.destFileName = s.fileName;
        s.destFileName = (std::filesystem::path(m_subGraphNode.fileName).filename().replace_extension() / s.destFileName).string();
        StringReplaceAll(s.destFileName, "\\", "/");

        // Update where the file lives on disk
        // Make the file path relative to the parent graph, not the child
        std::filesystem::path GGFilePath = std::filesystem::path(m_childGraph.baseDirectory) / std::filesystem::path(s.fileName);
        s.fileName = std::filesystem::proximate(GGFilePath, m_parentGraph.baseDirectory).string();

        return true;
    }

    // Rename enums
    bool Visit(Enum& e, const std::string& path)
    {
        // prepend the sub graph node name and add numbers if needed, to make it unique.
        char newName[1024];
        int index = -1;
        do
        {
            if (index == -1)
                sprintf_s(newName, "%s.%s", m_subGraphNode.name.c_str(), e.name.c_str());
            else
                sprintf_s(newName, "%s.%s.%i", m_subGraphNode.name.c_str(), e.name.c_str(), index);
            index++;
        } while (EnumNameExists(m_parentGraph, newName) || EnumNameExists(m_childGraph, newName));

        // remember this renaming
        m_renameData.m_enumRenames[e.name] = newName;

        // renaming book keeping
        if (e.originalName.empty())
            e.originalName = e.name;
        e.scope = m_subGraphNode.name + "." + e.scope;

        // Set the new name
        e.name = newName;

        return true;
    }

    // Rename hit groups
    bool Visit(RTHitGroup& hitGroup, const std::string& path)
    {
        // prepend the sub graph node name and add numbers if needed, to make it unique.
        char newName[1024];
        int index = -1;
        do
        {
            if (index == -1)
                sprintf_s(newName, "%s.%s", m_subGraphNode.name.c_str(), hitGroup.name.c_str());
            else
                sprintf_s(newName, "%s.%s.%i", m_subGraphNode.name.c_str(), hitGroup.name.c_str(), index);
            index++;
        } while (HitGroupNameExists(m_parentGraph, newName) || HitGroupNameExists(m_childGraph, newName));

        // remember this renaming
        m_renameData.m_hitGroupRenames[hitGroup.name] = newName;

        // renaming book keeping
        if (hitGroup.originalName.empty())
            hitGroup.originalName = hitGroup.name;
        hitGroup.scope = m_subGraphNode.name + "." + hitGroup.scope;

        // Set the new name
        hitGroup.name = newName;

        return true;
    }

    // Update file copy paths
    bool Visit(FileCopy& fileCopy, const std::string& path)
    {
        // Update where the file should be written out to.
        // Need to handle the subgraph possibly being in a parent directory etc.
        if (fileCopy.destFileName.empty())
            fileCopy.destFileName = fileCopy.fileName;
        fileCopy.destFileName = (std::filesystem::path(m_subGraphNode.fileName).filename().replace_extension() / fileCopy.destFileName).string();

        // make the file path relative to the parent graph, not the child
        std::filesystem::path GGFilePath = std::filesystem::path(m_childGraph.baseDirectory) / std::filesystem::path(fileCopy.fileName);
        fileCopy.fileName = std::filesystem::proximate(GGFilePath, m_parentGraph.baseDirectory).string();

        return true;
    }

    const RenderGraph& m_parentGraph;
    RenderGraph& m_childGraph;
    RenderGraphNode_Action_SubGraph& m_subGraphNode;
    RenameData& m_renameData; // Child graph renames
};

static void HandleSubgraphOutputPin(RenderGraph& parentGraph, RenderGraph& childGraph, const RenderGraphNode_Action_SubGraph& subGraphNode, const std::string& resourceName, std::string& newOutputNode, std::string& newOutputPin)
{
    struct NodePin
    {
        std::string node;
        std::string pin;
    };

    // Find all the "terminal access" points of this resource
    std::vector<NodePin> terminalRefs;
    std::vector<NodePin> refs(1);
    refs[0].node = resourceName;
    refs[0].pin = "resource";
    while (!refs.empty())
    {
        // Get the last ref in the list
        NodePin ref = *refs.rbegin();
        refs.pop_back();

        int refCount = 0;
        for (RenderGraphNode& node : childGraph.nodes)
        {
            if (GetNodeIsResourceNode(node))
                continue;

            const auto pins = FrontEndNodesNoCaching::GetPinInfo(childGraph, node);
            for (const auto& pin : pins)
            {
                if (*pin.dstNode != ref.node || *pin.dstPin != ref.pin)
                    continue;

                NodePin newRef;
                newRef.node = GetNodeName(node);
                newRef.pin = pin.srcPin;
                refs.push_back(newRef);

                refCount++;
            }
        }

        if (refCount == 0)
            terminalRefs.push_back(ref);
    }

    // If there is more than 1 terminal reference, make a barrier
    if (terminalRefs.size() > 1)
    {
        // Make a barrier node in the child graph, and plug all of the terminal access points into it.
        RenderGraphNode barrierNodeBase;
        {
            // set up a node as a barrier node
            barrierNodeBase._index = RenderGraphNode::c_index_actionBarrier;
            RenderGraphNode_Action_Barrier& barrierNode = barrierNodeBase.actionBarrier;

            // Make a unique name for the barrier node
            char newName[1024];
            int index = -1;
            do
            {
                if (index == -1)
                    sprintf_s(newName, "%s export barrier %s", subGraphNode.name.c_str(), resourceName.c_str());
                else
                    sprintf_s(newName, "%s export barrier %s %i", subGraphNode.name.c_str(), resourceName.c_str(), index);
                index++;
            } while (NodeNameExists(parentGraph, newName) || NodeNameExists(childGraph, newName));
            barrierNode.name = newName;

            // connect the terminal references into this barrier node
            barrierNode.connections.resize(terminalRefs.size());
            for (size_t i = 0; i < terminalRefs.size(); ++i)
            {
                char pinName[256];
                sprintf_s(pinName, "Pin %i", (int)i);
                barrierNode.connections[i].srcPin = pinName;
                barrierNode.connections[i].dstNode = terminalRefs[i].node;
                barrierNode.connections[i].dstPin = terminalRefs[i].pin;
            }

            // add the barrier node to the child graph
            childGraph.nodes.push_back(barrierNodeBase);
        }

        newOutputNode = barrierNodeBase.actionBarrier.name;
        newOutputPin = "Pin 0";
    }
    // else there is only one terminal ref, so use that
    else
    {
        newOutputNode = terminalRefs[0].node;
        newOutputPin = terminalRefs[0].pin;
    }
}

static void HandleSubgraphOutputPins(RenderGraph& parentGraph, RenderGraph& childGraph, const RenderGraphNode_Action_SubGraph& subGraphNode, const RenameData& renameData)
{
    for (size_t resourceIndex = 0; resourceIndex < subGraphNode.subGraphData.importedResources.size(); ++resourceIndex)
    {
        const std::string& resourceName = subGraphNode.subGraphData.importedResources[resourceIndex];

        // Make a barrier node to barrier all the writes of the resource before moving on in the technique.
        // The "false ordering from read dependencies" optimization will make this only barrier writes.
        // Reads in the technique will be able to be parallelized with the parent graph work, before graph flattening.
        std::string renamedResourcename = resourceName;
        renameData.UpdateNodeName(renamedResourcename);
        std::string barrierNodeName;
        std::string barrierNodePin;
        HandleSubgraphOutputPin(parentGraph, childGraph, subGraphNode, renamedResourcename, barrierNodeName, barrierNodePin);

        // Everything in the parent graph which referenced the barrier node's first pin, should now reference the new barrier node's "Pin 0" instead.
        int nodeIndex = -1;
        for (RenderGraphNode& node : parentGraph.nodes)
        {
            nodeIndex++;
            if (GetNodeIsResourceNode(node))
                continue;

            auto pins = FrontEndNodesNoCaching::GetPinInfo(parentGraph, node);
            for (auto& pin : pins)
            {
                if (*pin.dstNode != subGraphNode.name || *pin.dstPin != resourceName)
                    continue;
                *pin.dstNode = barrierNodeName;
                *pin.dstPin = barrierNodePin;
            }
        }
    }

    for (size_t resourceIndex = 0; resourceIndex < subGraphNode.subGraphData.exportedResources.size(); ++resourceIndex)
    {
        const std::string& resourceName = subGraphNode.subGraphData.exportedResources[resourceIndex];

        // Make a barrier node to barrier all the writes of the resource before moving on in the technique.
        // The "false ordering from read dependencies" optimization will make this only barrier writes.
        // Reads in the technique will be able to be parallelized with the parent graph work, before graph flattening.
        std::string renamedResourcename = resourceName;
        renameData.UpdateNodeName(renamedResourcename);
        std::string barrierNodeName;
        std::string barrierNodePin;
        HandleSubgraphOutputPin(parentGraph, childGraph, subGraphNode, renamedResourcename, barrierNodeName, barrierNodePin);

        // Everything in the parent graph which referenced the barrier node's first pin, should now reference the new barrier node's "Pin 0" instead.
        int nodeIndex = -1;
        for (RenderGraphNode& node : parentGraph.nodes)
        {
            nodeIndex++;
            if (GetNodeIsResourceNode(node))
                continue;

            auto pins = FrontEndNodesNoCaching::GetPinInfo(parentGraph, node);
            for (auto& pin : pins)
            {
                if (*pin.dstNode != subGraphNode.name || *pin.dstPin != resourceName)
                    continue;
                *pin.dstNode = barrierNodeName;
                *pin.dstPin = barrierNodePin;
            }
        }
    }
}

static void HandleSubgraphInputPins(RenderGraph& parentGraph, RenderGraph& childGraph, const RenderGraphNode_Action_SubGraph& subGraphNode, const RenameData& renameData)
{
    // For each imported resource (input pin)
    for (size_t resourceIndex = 0; resourceIndex < subGraphNode.subGraphData.importedResources.size(); ++resourceIndex)
    {
        // Delete the resource node from the child graph
        const std::string& resourceName = subGraphNode.subGraphData.importedResources[resourceIndex];
        std::string renamedResourceName = resourceName;
        renameData.UpdateNodeName(renamedResourceName);
        childGraph.nodes.erase(
            std::remove_if(childGraph.nodes.begin(), childGraph.nodes.end(),
                [&renamedResourceName](const RenderGraphNode& node)
                {
                    return GetNodeName(node) == renamedResourceName;
                }
            ),
            childGraph.nodes.end()
        );

        // 1) Any node/pin reference to the imported resource should be replaced by what is plugged into that input pin.
        {
            RenameData rd;

            NodePin src;
            src.node = renamedResourceName;
            src.pin = "resource";

            NodePin dest;
            dest.node = subGraphNode.connections[resourceIndex].dstNode;
            dest.pin = subGraphNode.connections[resourceIndex].dstPin;

            rd.m_nodePinRenames[src] = dest;

            RenameReferencesVisitor visitor(rd);
            if (!Visit(childGraph, visitor, "childGraph"))
                ShowErrorMessage("Problem (1) renaming references in " __FUNCTION__);

            //INLINE_DEBUG_NODE("    %s:%s in %s is being renamed to %s:%s", src.node.c_str(), src.pin.c_str(), subGraphNode.name.c_str(), dest.node.c_str(), dest.pin.c_str());
        }

        // 2) Any other reference to the imported resource should be replaced by the resource that the node plugged into the input pin represents.
        {
            int nodeIndex = FrontEndNodesNoCaching::GetRootNodeIndex(parentGraph, subGraphNode.connections[resourceIndex].dstNode, subGraphNode.connections[resourceIndex].dstPin);
            if (nodeIndex >= 0)
            {
                if (!GetNodeIsResourceNode(parentGraph.nodes[nodeIndex]))
                    ShowErrorMessage("GetRootNodeIndex of \"%s\":\"%s\" did not give a resource node " __FUNCTION__, subGraphNode.connections[resourceIndex].dstNode.c_str(), subGraphNode.connections[resourceIndex].dstPin.c_str());

                RenameData rd;
                rd.m_nodeRenames[renamedResourceName] = GetNodeName(parentGraph.nodes[nodeIndex]);

                RenameReferencesVisitor visitor(rd);
                if (!Visit(childGraph, visitor, "childGraph"))
                    ShowErrorMessage("Problem (2) renaming references in " __FUNCTION__);

                //INLINE_DEBUG_NODE("    %s in %s is being replaced by %s", renamedResourceName.c_str(), subGraphNode.name.c_str(), GetNodeName(parentGraph.nodes[nodeIndex]).c_str());
            }
            else
                ShowErrorMessage("Could not GetRootNodeIndex of \"%s\":\"%s\" in " __FUNCTION__, subGraphNode.connections[resourceIndex].dstNode.c_str(), subGraphNode.connections[resourceIndex].dstPin.c_str());
        }
    }
}

static bool InlineSubGraph(RenderGraph& parentGraph, RenderGraphNode_Action_SubGraph& subGraphNode)
{
    // load the child graph
    std::string childFileName = (std::filesystem::path(parentGraph.baseDirectory) / std::filesystem::path(subGraphNode.fileName)).string();
    RenderGraph childGraph;
    if (ReadFromJSONFile(childGraph, childFileName.c_str()))
    {
        if (childGraph.version != std::string(GIGI_VERSION()))
        {
            ShowErrorMessage("Could not load subgraph. File %s is version %s and couldn't be upgraded to version %s.", childFileName.c_str(), childGraph.version.c_str(), GIGI_VERSION());
            return false;
        }

        if (childGraph.versionUpgraded)
            ShowInfoMessage("Sub Graph \"%s\" Upgraded from %s to %s%s%s", childFileName.c_str(), childGraph.versionUpgradedFrom.c_str(), childGraph.version.c_str(), childGraph.versionUpgradedMessage.empty() ? "" : ":\n", childGraph.versionUpgradedMessage.c_str());
    }
    else
    {
        ShowErrorMessage("Could not load subgraph %s.", childFileName.c_str());
        return false;
    }

    // get the base directory of the render graph
    {
        std::string rgPath = childFileName.c_str();
        int pos = (int)rgPath.find_last_of("\\/");
        if (pos != -1)
        {
            childGraph.baseDirectory = rgPath.substr(0, rgPath.find_last_of("\\/"));
            childGraph.baseDirectory += "/";
        }
        else
            childGraph.baseDirectory = "./";
    }

    // Handle variable replacement
    // This is when subgraph variables are replaced by parent graph variables
    {
        // Handle "replaceWithValue" - make a constant var with that as the default, and set replaceWithStr to it
        {
            for (SubGraphVariableSettings& variableSettings : subGraphNode.variableSettings)
            {
                if (variableSettings.replaceWithValue.empty())
                    continue;

                int childVariableIndex = GetVariableIndex(childGraph, variableSettings.name.c_str());
                if (childVariableIndex == -1)
                {
                    ShowErrorMessage("Could not find variable \"%s\" in \"%s\" for node \"%s\".", variableSettings.name.c_str(), childFileName.c_str(), subGraphNode.name.c_str());
                    continue;
                }

                // make a unique name for the new variable
                char variableName[1024];
                int index = 0;
                do
                {
                    sprintf_s(variableName, "__literal_%i", index);
                    index++;
                }
                while (VariableNameExists(parentGraph, variableName));

                // make a new variable in the parent graph
                Variable newVariable;
                newVariable.name = variableName;
                newVariable.comment = "Made to replace variable \"" + variableSettings.name + "\" with a constant value in subgraph node \"" + subGraphNode.name + "\"";
                newVariable.type = childGraph.variables[childVariableIndex].type;
                newVariable.Const = true;
                newVariable.dflt = variableSettings.replaceWithValue;
                newVariable.transient = true;
                parentGraph.variables.push_back(newVariable);

                // tell the subgraph to use this new variable instead
                variableSettings.replaceWithStr = variableName;
            }
        }

        // Replace variable references
        {
            RenameData renameData;
            for (const SubGraphVariableSettings& variableSettings : subGraphNode.variableSettings)
            {
                // if no variable replacement to do, skip this
                if (variableSettings.replaceWithStr.empty() && !variableSettings.isLoopIndex)
                    continue;

                // If this is the loop index, make the var to replace it with
                std::string replacementVarName = variableSettings.replaceWithStr;
                if (variableSettings.isLoopIndex)
                {
                    char loopIndexVarname[256];
                    sprintf_s(loopIndexVarname, "__loopIndexValue_%i", subGraphNode.loopIndex);
                    replacementVarName = loopIndexVarname;
                }

                renameData.m_variableRenames[variableSettings.name] = replacementVarName;

                // record this into the render graph so we can look up variables for shaders
                VariableReplacement replacement;
                replacement.srcScope = subGraphNode.name + ".";
                replacement.srcName = variableSettings.name;

                // Set the destName with scope in tact
                size_t dotPos = subGraphNode.name.find_last_of('.');
                if (dotPos != std::string::npos)
                    replacement.destName = subGraphNode.name.substr(0, dotPos + 1) + replacementVarName;
                else
                    replacement.destName = replacementVarName;

                parentGraph.variableReplacements.push_back(replacement);
            }

            RenameReferencesVisitor visitor(renameData);
            if (!Visit(childGraph, visitor, "childGraph"))
                return false;
        }

        // Delete replaced variables
        childGraph.variables.erase(
            std::remove_if(childGraph.variables.begin(), childGraph.variables.end(),
                [&subGraphNode](const Variable& variable)
                {
                    for (const SubGraphVariableSettings& variableSettings : subGraphNode.variableSettings)
                    {
                        if (variableSettings.name != variable.name)
                            continue;
                        return !variableSettings.replaceWithStr.empty();
                    }
                    return false;
                }
            ),
            childGraph.variables.end()
        );
    }

    // Make the child graph ready to have it's information inserted into the parent graph by giving everything a unique name and fixing up the references.
    RenameData renameData;
    {
        RenameChildVisitor visitor(parentGraph, childGraph, subGraphNode, renameData);
        if (!Visit(childGraph, visitor, "childGraph"))
            return false;
    }
    {
        RenameReferencesVisitor visitor(renameData);
        if (!Visit(childGraph, visitor, "childGraph"))
            return false;

        // Also update all childgraph subgraph nodes to have a file path relative to the parent, instead of the child
        for (RenderGraphNode& baseNode : childGraph.nodes)
        {
            if (baseNode._index != RenderGraphNode::c_index_actionSubGraph)
                continue;

            RenderGraphNode_Action_SubGraph& node = baseNode.actionSubGraph;

            std::filesystem::path GGFilePath = std::filesystem::path(childGraph.baseDirectory) / std::filesystem::path(node.fileName);
            node.fileName = std::filesystem::proximate(GGFilePath, parentGraph.baseDirectory).string();
        }
    }

    // Handle the sub graph output pins (exported resources, and imported resources when the technique is done with them)
    HandleSubgraphOutputPins(parentGraph, childGraph, subGraphNode, renameData);

    // Handle the sub graph input pins (imported resources)
    HandleSubgraphInputPins(parentGraph, childGraph, subGraphNode, renameData);

    // Copy objects from the child graph to the parent graph
    {
        // Shaders
        for (const Shader& childObject : childGraph.shaders)
            parentGraph.shaders.push_back(childObject);

        // Variables - set visibility as specified in variable settings
        for (Variable childObject : childGraph.variables)
        {
            for (const SubGraphVariableSettings& variableSettings : subGraphNode.variableSettings)
            {
                if (variableSettings.name == childObject.originalName)
                {
                    childObject.visibility = variableSettings.visibility;
                    break;
                }
            }
            parentGraph.variables.push_back(childObject);
        }

        // Structs
        for (const Struct& childObject : childGraph.structs)
            parentGraph.structs.push_back(childObject);

        // enums
        for (const Enum& childObject : childGraph.enums)
            parentGraph.enums.push_back(childObject);

        // Hit Groups
        for (const RTHitGroup& childObject : childGraph.hitGroups)
            parentGraph.hitGroups.push_back(childObject);

        // file Copies
        for (const FileCopy& childObject : childGraph.fileCopies)
        {
            bool alreadyExists = false;
            for (const FileCopy& parentObject : parentGraph.fileCopies)
            {
                if (parentObject == childObject)
                {
                    alreadyExists = true;
                    break;
                }
            }

            if (!alreadyExists)
                parentGraph.fileCopies.push_back(childObject);
        }

        // set vars
        for (const SetVariable& childObject : childGraph.setVars)
            parentGraph.setVars.push_back(childObject);

        // Nodes
        // NOTE: this invalidates the "subGraphNode" function parameter so is done last
        bool subgraphAlwaysFalse = subGraphNode.condition.alwaysFalse;
        for (const RenderGraphNode& childObject : childGraph.nodes)
        {
            parentGraph.nodes.push_back(childObject);

            // If a subgraph is disabled by setting the condition to always false, all the nodes should get that flag too.
            DispatchLambdaAction(*parentGraph.nodes.rbegin(),
                [subgraphAlwaysFalse](RenderGraphNode_ActionBase& node)
                {
                    node.condition.alwaysFalse = subgraphAlwaysFalse;
                }
            );
        }
    }

    // Add this to the list of subgraph file names
    parentGraph.subGGGraphFileNames.push_back(std::filesystem::absolute(childFileName).string().c_str());

    return true;
}

static bool CanInlineSubgraph(RenderGraph& parentGraph, const RenderGraphNode_Action_SubGraph& subGraphNode)
{
    // A sub graph can be inlined if we can find a resource node at the left most part of the render graph, for each input pin to the sub graph.
    // This won't be true if ultimately using a resource exported from a different sub graph.
    // When that happens, it means we need to inline a different sub graph and try again.
    for (size_t resourceIndex = 0; resourceIndex < subGraphNode.subGraphData.importedResources.size(); ++resourceIndex)
    {
        int nodeIndex = FrontEndNodesNoCaching::GetRootNodeIndex(parentGraph, subGraphNode.connections[resourceIndex].dstNode, subGraphNode.connections[resourceIndex].dstPin);
        if (nodeIndex < 0)
            return false;
        if (!GetNodeIsResourceNode(parentGraph.nodes[nodeIndex]))
            return false;
    }
    return true;
}

bool ExpandLoopedSubgraphs(RenderGraph& renderGraph)
{
	// Looping of subgraphs is implemented by duplicating the node in the graph and linking
	// the duplicates to each other.

	// The duplicates are renamed by adding 'iteration X' to the end.
	
	// This holds the renaming data only of the last iteration in the loop
	// so that backward references to this node can be updated.
	RenameData renameData;

	// These nodes will be added to the render graph and all other subgraph nodes will be deleted
	std::vector<RenderGraphNode> loopNodes;

	for (int nodeIndex = 0; nodeIndex < renderGraph.nodes.size(); ++nodeIndex)
	{
		// skip nodes that aren't sub graph nodes
		RenderGraphNode& node = renderGraph.nodes[nodeIndex];
		if (node._index != RenderGraphNode::c_index_actionSubGraph)
			continue;

		const RenderGraphNode_Action_SubGraph& subGraph = node.actionSubGraph;

        // Get the loop count
        // If a variable is specified, it overrides the literal value given.
        int loopCount = subGraph.loopCount;
        if (!subGraph.loopCountVariable.name.empty())
        {
            for (const Variable& var : renderGraph.variables)
            {
                if (var.name == subGraph.loopCountVariable.name)
                {
                    sscanf_s(var.dflt.c_str(), "%i", &loopCount);
                    break;
                }
            }
        }

		if (loopCount > 1)
		{
			// Loop over the number of iterations we want
			for (int loopIdx = 0; loopIdx < loopCount; ++loopIdx)
			{
				// Copy the original node
				RenderGraphNode loopNode = node;
				RenderGraphNode_Action_SubGraph& loopSubGraph = loopNode.actionSubGraph;
				std::string oldName = loopSubGraph.name;

				// Update node name
				std::stringstream loopStream;
				loopStream << " Iteration " << loopIdx;
				loopSubGraph.name += loopStream.str();

				// set the loop index
				loopSubGraph.loopIndex = loopIdx;

				// Loop nodes other than the first need to have their connections updated to point
				// to the previous iteration.
				if (loopIdx != 0)
				{
					// Outbound connections need to be setup
					for (NodePinConnection& connection : loopSubGraph.connections)
					{
						std::stringstream connectionStream;
						connectionStream << " Iteration " << (loopIdx - 1);
						connection.dstNode = oldName + connectionStream.str();
						connection.dstPin = connection.srcPin;
					}
				}

				// If this is the last iteration, other nodes in the parent graph need to have references
				// to the original subgraph updated to point to the last iteration.
				if (loopIdx == loopCount-1)
				{
					renameData.m_nodeRenames[oldName] = loopSubGraph.name;
				}

				// Add the modified node to the list
				loopNodes.push_back(loopNode);
			}
		}
		else // subGraph.loopCount <= 1
		{
			// Just add the same node back.
			// Note we don't currently properly handle loopCount <= 0
			loopNodes.push_back(node);
		}
	}

	// Erase existing SubGraph nodes
	startErase:
	for (int nodeIndex = 0; nodeIndex < renderGraph.nodes.size(); ++nodeIndex)
	{
		RenderGraphNode& node = renderGraph.nodes[nodeIndex];
		if (node._index == RenderGraphNode::c_index_actionSubGraph)
		{
			renderGraph.nodes.erase(renderGraph.nodes.begin() + nodeIndex);
			goto startErase;
		}
	}

	// Add the newly created nodes
	for (const RenderGraphNode& loopNode : loopNodes)
	{
		renderGraph.nodes.push_back(loopNode);
	}

	// Rename all references to the original subgraphs to point to the last iterations of any
	// that were expanded
	RenameReferencesVisitor visitor(renameData);
	if (!Visit(renderGraph, visitor, "parentGraph"))
		return false;

	return true;
}

bool InlineSubGraphs(RenderGraph& renderGraph)
{
	// First expand any looped graphs
	if (!ExpandLoopedSubgraphs(renderGraph))
		return false;

    // Inline all the sub graph nodes, recursively.
    //
    // We can't go just in any order though.
    // Part of inlining a subgraph is turning all of the subgraph's resource node references (like a CS dispatch size based on a resource size)
    // into a specific resource node, which is found by "going left" on the graph until you hit the resource node.
    // When doing that, you may hit a different subgraph node though, if what is plugged into a subgraph input node, is ultimately what comes
    // out of a different subgraph as an output node from an exported resource.
    // So, we need to process the sub graph nodes, one at a time, processing one that can find resource nodes for all of it's inputs.
    // This should always be possible, but we still error if this loop is unable to make progress.
    // Note that this process is recursive because the act of inlining a subgraph may bring another subgraph with it, by a subgraph having
    // a subgraph inside of it.
    //
    int maxLoopIndex = -1;
    bool subgraphNodesRemain = true;
    bool madeProgress = true;
    while (subgraphNodesRemain)
    {
        subgraphNodesRemain = false;
        madeProgress = false;

        // Show Debug Info
        {
            INLINE_DEBUG("\n\n=========New Loop Begins=========\n");

            // Show nodes
            INLINE_DEBUG_NODE("Nodes:\n");
            for (int nodeIndex = 0; nodeIndex < renderGraph.nodes.size(); ++nodeIndex)
            {
                INLINE_DEBUG_NODE("  [%i] %s (%s)", nodeIndex, GetNodeName(renderGraph.nodes[nodeIndex]).c_str(), GetNodeTypeString(renderGraph.nodes[nodeIndex]).c_str());

                DispatchLambdaAction(renderGraph.nodes[nodeIndex],
                    [](const auto& node)
                    {
                        for (const NodePinConnection& connection : node.connections)
                        {
                            if (!connection.dstNode.empty())
                            {
                                INLINE_DEBUG_NODE("    %s <- %s : %s", connection.srcPin.c_str(), connection.dstNode.c_str(), connection.dstPin.c_str());
                            }
                            else
                            {
                                INLINE_DEBUG_NODE("    %s", connection.srcPin.c_str());
                            }
                        }
                    }
                );

                // copy resource doesn't fill out connections right now
                if (renderGraph.nodes[nodeIndex]._index == RenderGraphNode::c_index_actionCopyResource)
                {
                    const RenderGraphNode_Action_CopyResource& node = renderGraph.nodes[nodeIndex].actionCopyResource;
                    INLINE_DEBUG_NODE("    Source <- %s : %s", node.source.node.c_str(), node.source.pin.c_str());
                    INLINE_DEBUG_NODE("    Dest <- %s : %s", node.dest.node.c_str(), node.dest.pin.c_str());
                }

                /*
                if (renderGraph.nodes[nodeIndex]._index == RenderGraphNode::c_index_actionSubGraph)
                {
                    INLINE_DEBUG_NODE("    Imported Resources:");
                    const RenderGraphNode_Action_SubGraph& node = renderGraph.nodes[nodeIndex].actionSubGraph;
                    for (const auto& importedResource : node.subGraphData.importedResources)
                    {
                        INLINE_DEBUG_NODE("      %s", importedResource.c_str());
                    }
                }
                */
            }
            INLINE_DEBUG_NODE("\n");

            // Show variables
            if (renderGraph.variables.size() > 0)
            {
                INLINE_DEBUG_VARIABLE("Variables:\n");
                for (const Variable& variable : renderGraph.variables)
                {
                    INLINE_DEBUG_VARIABLE("  %s (%s)\n", variable.name.c_str(), EnumToString(variable.type));
                }
                INLINE_DEBUG_VARIABLE("\n");
            }
            else
            {
                INLINE_DEBUG_VARIABLE("Variables: None\n\n");
            }

            // Show variable replacements
            if (renderGraph.variableReplacements.size() > 0)
            {
                INLINE_DEBUG_VARIABLE("Variable Replacements:\n");
                for (const VariableReplacement& replacement : renderGraph.variableReplacements)
                {
                    INLINE_DEBUG_VARIABLE("  %s%s replaced with %s", replacement.srcScope.c_str(), replacement.srcName.c_str(), replacement.destName.c_str());
                }
                INLINE_DEBUG_VARIABLE("\n");
            }
            else
            {
                INLINE_DEBUG_VARIABLE("Variable Replacements: None\n\n");
            }
        }

        for (int nodeIndex = 0; nodeIndex < renderGraph.nodes.size(); ++nodeIndex)
        {
            // skip nodes that aren't sub graph nodes
            RenderGraphNode& node = renderGraph.nodes[nodeIndex];
            if (node._index != RenderGraphNode::c_index_actionSubGraph)
                continue;

            // Remember that we saw at least one sub graph
            subgraphNodesRemain = true;

            // inline the sub graph node if we can
            std::string subgraphNodeName = node.actionSubGraph.name;
            if (CanInlineSubgraph(renderGraph, node.actionSubGraph))
            {
                maxLoopIndex = std::max(maxLoopIndex, node.actionSubGraph.loopIndex);

                // inline the node
                if (!InlineSubGraph(renderGraph, node.actionSubGraph))
                {
                    INLINE_DEBUG("  %s: Failed to inline", subgraphNodeName.c_str());
                    return false;
                }
                else
                {
                    INLINE_DEBUG("  %s: Inlined", subgraphNodeName.c_str());
                }

                // delete the node
                renderGraph.nodes.erase(renderGraph.nodes.begin() + nodeIndex);

                // remember that we made progress
                madeProgress = true;
                break;
            }
            else
            {
                INLINE_DEBUG("  %s: Cannot Inline", subgraphNodeName.c_str());
            }
        }

        // detect and report the error of not being able to make progress
        if (subgraphNodesRemain && !madeProgress)
        {
            for (int nodeIndex = 0; nodeIndex < renderGraph.nodes.size(); ++nodeIndex)
            {
                RenderGraphNode& node = renderGraph.nodes[nodeIndex];
                if (node._index != RenderGraphNode::c_index_actionSubGraph)
                    continue;

                ShowErrorMessage("Unable to make progress inlining subgraphs. Node \"%s\" could not be inlined, are all of it's inputs plugged in?", node.actionSubGraph.name.c_str());
                return false;
            }
        }
    }

    // make the loop index variables needed
    for (int loopIndexValueVarIndex = 0; loopIndexValueVarIndex <= maxLoopIndex; ++loopIndexValueVarIndex)
    {
        char loopIndexVarname[256];
        sprintf_s(loopIndexVarname, "__loopIndexValue_%i", loopIndexValueVarIndex);

        char value[256];
        sprintf_s(value, "%i", loopIndexValueVarIndex);

        Variable newVariable;
        newVariable.name = loopIndexVarname;
        newVariable.type = DataFieldType::Int;
        newVariable.dflt = value;
        newVariable.visibility = VariableVisibility::Internal;
        newVariable.transient = true;

        renderGraph.variables.push_back(newVariable);
    }

    // return success
    return true;
}

/*
------------------------ SUBGRAPH INFO ------------------------

Subgraphs need some rethinking. They are more complex than needed, and have some unhandled edge cases, I'm sure!
Here are some notes about how subgraphs actually work.

How does inlining a subgraph work?

-- LOOP COUNT --

If a subgraph has a loop count greater than 1, multiple copies of the subgraph node are inserted, but renamed to add "Iteration <x>" onto the name of each node.
* the nodes connect their pins together
* anything connected to the output of the subgraph instead connects to the same pins of the last subgraph node created.
* This happens in ExpandLoopedSubgraphs()

-- CHOOSING WHICH SUBGRAPH TO INLINE --

While there are subgraphs remaining in the render graph, it looks for the first one that is able to be inlined, and inlines it.

A subgraph is able to be inlined if all of it's inputs can be resolved to a resource (not an export from another subgraph).

The reason for this is because if something like ac compute dispatch size references the imported resource that's internal to
the subgraph, it needs to be modified to reference the actual resource plugged into that input pin, instead.

-- INLINING A SUBGRAPH --

The subgraph render graph is loaded, and the base directory is set to where the subgraph .gg file lives.

Variables
The subgraph may have settings on it for replacing internal variables with variables from the parent graph. That replacement happens first.
That information also goes into parentGraph.variableReplacements as a look up for where the variable references can't be updated,
such as when read from shader files.
A replacement always goes from a child scope to a parent scope, such as <scope1>.<scope2>.<varA> to <scope1>.<varB>.

Subgraph Object Names
RenameChildVisitor is used to rename items from the child graph to ensure unique names when inlined into the parent graph.
This includes: nodes, variables, structs, shaders, enums, hitgroups, and file copy paths.
The RenameReferencesVisitor makes sure all references in the child graph are updated to use the new names
The editor should have child graph structs, enums, etc exposed to the parent graph, but that doesn't currently happen.

Subgraph Node Pins
HandleSubgraphOutputPins() makes it so that anything which was referencing a subgraph output pin, is now referencing the appropriate
inlined node from the subgraph.  If a subgraph had multiple "terminal references" of a resource, those resources are plugged into
a barrier node, and output references will reference the barrier.  The barrier is a "gigi compiler" construct which is only there
to ensure that all work plugged into the barrier comes before any work that comes after. So, whatever is connected to the output
pin of a subgraph node, will in reality connect to the "last access" of that resource within the render graph.

TODO: if this doesn't already happen, the "false read dependency ordering" optimization should happen after this, which
allows work re-ordering, even within flattened render graph work, so long as it doesn't violate read/write constraints.

HandleSubgraphInputPins() will make it so subgraph nodes referencing imported resources will instead reference the actual resources
plugged into the input pins.

Subgraph Objects
Objects are then copied from the child graph to the parent graph.
These include shaders, variables, structs, enums, hit groups, file copies, set variables, and of course, nodes!

inlining is then complete.


Replacement vars vs Rename vars

A subgraph node lets you specify that a variable in the subgraph should be replaced by a variable in the parent graph.
All references to the variable in the subgraph are replaced by references to the variable in the parent graph.
Shaders may still reference the replaced variable so GetScopedVariableIndex() exists to follow the trail of replaced variables for this usage case.

Variables that aren't replaced need to be renamed to help make sure they are unique.  They are renamed as <subgraphNodeName>.<variableName>
in RenameChildVisitor.


TODO:
 ? maybe get rid of scope on variables and make it implicit?
 ? also on shaders? idk
 ? and enums?
 ! variable scopes are used by the viewer to separate variables by grouping.
  * could still get it from var names.
? maybe we can get rid of VariableReplacement.srcScope and just put it in the srcName?



*/