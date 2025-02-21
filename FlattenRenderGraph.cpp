///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "FlattenRenderGraph.h"
#include "Schemas/Types.h"
#include <vector>
#include "GigiCompilerLib/Backends/Shared.h"

static float CalculateRenderGraphScore(const RenderGraph& renderGraph)
{
    size_t ret = 0;
    for (const auto& it : renderGraph.transitions)
        ret += it.transitions.size();
    return (float)ret;
}

static void CalculateResourceTransitions(RenderGraph& renderGraph)
{
    // Initialize the states of every node (we only care about resource nodes but this is easier to program)
    // Leave an extra slot at the beginning for starting states.
    std::vector<std::vector<ShaderResourceAccessType>> resourceStatesAtEachStep(renderGraph.nodes.size());
    for (auto& states : resourceStatesAtEachStep)
        states.resize(renderGraph.nodes.size(), ShaderResourceAccessType::Count);

    // Set the state of each resource at each step of the render graph
    for (int stepIndex = 0; stepIndex < renderGraph.flattenedNodeList.size(); ++stepIndex)
    {
        auto& states = resourceStatesAtEachStep[stepIndex];
        int nodeIndex = renderGraph.flattenedNodeList[stepIndex];
        RenderGraphNode& node = renderGraph.nodes[nodeIndex];
        int pinCount = GetNodePinCount(node);
        for (int pinIndex = 0; pinIndex < pinCount; ++pinIndex)
        {
            InputNodeInfo pinInfo = GetNodePinInputNodeInfo(node, pinIndex);
            int resourceNodeIndex = GetResourceNodeForPin(renderGraph, node, pinIndex);
            if (resourceNodeIndex != -1)
            {
                ShaderResourceType resourceType = ShaderResourceType::Count;
                switch (renderGraph.nodes[resourceNodeIndex]._index)
                {
                    case RenderGraphNode::c_index_resourceBuffer: resourceType = ShaderResourceType::Buffer; break;
                    case RenderGraphNode::c_index_resourceShaderConstants: resourceType = ShaderResourceType::ConstantBuffer; break;
                    case RenderGraphNode::c_index_resourceTexture: resourceType = ShaderResourceType::Texture; break;
                    default:
                    {
                        Assert(false, "Unhandled render graph node type");
                        break;
                    }
                }

                states[resourceNodeIndex] = pinInfo.access;
                AddResourceNodeAccessedAs(renderGraph.nodes[resourceNodeIndex], pinInfo.access);
                AddResourceDependency(node, pinIndex, resourceNodeIndex, resourceType, pinInfo.access);
            }
        }
    }

    // set the final states on the resource nodes. Useful for creating textures in these states.
    // also set the starting states on the resource nodes. Useful for transitioning imported textures to this state before first use
    for (size_t nodeIndex = 0; nodeIndex < renderGraph.nodes.size(); ++nodeIndex)
    {
        if (!GetNodeIsResourceNode(renderGraph.nodes[nodeIndex]))
            continue;

        for (const auto& states : resourceStatesAtEachStep)
        {
            if (states[nodeIndex] != ShaderResourceAccessType::Count)
            {
                SetResourceNodeStartingState(renderGraph.nodes[nodeIndex], states[nodeIndex]);
                break;
            }
        }

        for (const auto& states : resourceStatesAtEachStep)
        {
            if (states[nodeIndex] != ShaderResourceAccessType::Count)
                SetResourceNodeFinalState(renderGraph.nodes[nodeIndex], states[nodeIndex]);
        }
    }

    // make the transitions, with the states starting at whatever their ending state is (aka previous frame ending state)
    std::vector<ShaderResourceAccessType> lastSetStates(renderGraph.nodes.size());
    for (int stepIndex = 0; stepIndex < renderGraph.flattenedNodeList.size(); ++stepIndex)
    {
        int nodeIndex = renderGraph.flattenedNodeList[stepIndex];
        if (!GetNodeIsResourceNode(renderGraph.nodes[nodeIndex]))
            continue;
        lastSetStates[nodeIndex] = GetResourceNodeFinalState(renderGraph.nodes[nodeIndex]);
    }

    std::vector<bool> firstTransition(renderGraph.nodes.size(), true);
    renderGraph.transitions.resize(renderGraph.nodes.size());
    for (int stepIndex = 0; stepIndex < renderGraph.flattenedNodeList.size(); ++stepIndex)
    {
        int nodeIndex = renderGraph.flattenedNodeList[stepIndex];

        // skip nodes that are resources
        if (GetNodeIsResourceNode(renderGraph.nodes[nodeIndex]))
            continue;

        // get our states required at this step
        const auto& states = resourceStatesAtEachStep[stepIndex];

        // make transitions for any resources that want them
        for (size_t resourceIndex = 0; resourceIndex < renderGraph.nodes.size(); ++resourceIndex)
        {
            // skip nodes that arent resources
            if (!GetNodeIsResourceNode(renderGraph.nodes[resourceIndex]))
                continue;

            ShaderResourceAccessType lastState = lastSetStates[resourceIndex];
            ShaderResourceAccessType nextState = states[resourceIndex];

            // skip this resource node if it isn't referenced by this step
            if (nextState == ShaderResourceAccessType::Count)
                continue;

            // if the state changed, or it goes from uavrw to uavrw, we need to emit a transition
            if ((lastState != nextState) || lastState == ShaderResourceAccessType::UAV)
            {
                // imported resources should skip their first transition, because we explicitly set them to the right state after importing
                if (GetNodeResourceVisibility(renderGraph.nodes[resourceIndex]) != ResourceVisibility::Imported || !firstTransition[resourceIndex])
                {
                    ResourceTransition newTransition;
                    newTransition.nodeIndex = (int)resourceIndex;
                    newTransition.oldState = lastState;
                    newTransition.newState = nextState;
                    renderGraph.transitions[stepIndex].transitions.push_back(newTransition);
                }
                else
                {
                    firstTransition[resourceIndex] = false;
                }

                lastSetStates[resourceIndex] = nextState;
            }
        }
    }
}

void OptimizeAndFlattenRenderGraph(RenderGraph& renderGraph)
{
    struct DAGNode
    {
        int nodeIndex;
        std::vector<int> dependentOn;
    };

    struct DAG
    {
        std::vector<int> flattenedList;
        std::vector<DAGNode> dag;
    };

    // build the render graph DAG
    DAG rgdag;
    rgdag.dag.resize(renderGraph.nodes.size());
    for (int index = 0; index < (int)renderGraph.nodes.size(); ++index)
    {
        rgdag.dag[index].nodeIndex = index;

        // skip resource nodes
        if (GetNodeIsResourceNode(renderGraph.nodes[index]))
            continue;

        int pinCount = GetNodePinCount(renderGraph.nodes[index]);

        // Get the input dependencies of the node
        for (int pinIndex = 0; pinIndex < pinCount; ++pinIndex)
        {
            // Get what we this pin is connected to
            InputNodeInfo connectionInfo = GetNodePinInputNodeInfo(renderGraph.nodes[index], pinIndex);
            if (connectionInfo.nodeIndex == -1)
            {
                if (connectionInfo.required)
                    Assert(false, "A pin is missing a connection: %s %s", GetNodeName(renderGraph.nodes[index]).c_str(), GetNodePinName(renderGraph.nodes[index], pinIndex).c_str());
                continue;
            }

            rgdag.dag[index].dependentOn.push_back(connectionInfo.nodeIndex);

            // TODO: need to sort out false dependency ordering problem from read only access
            // TOOD: when fixed, make sure barrier nodes are honored
#if 0

            // if this is not a read only access type, take it as is to keep the ordering constraint
            // TODO: need to think about this. I think that we SHOULD allow this, but if multiple things want read access to the same thing before a write, it needs to copy it and make a temporary resource.
            // TODO: we may actually need to disable this optimization until it's reworked, i don't think this fix is enough
            if (!ShaderResourceTypeIsReadOnly(connectionInfo.access))
            {
                rgdag.dag[index].dependentOn.push_back(connectionInfo.nodeIndex);
                continue;
            }

            // If what we are conncected to has read access to what is plugged into it, follow the path back
            // to the source resource until you hit a write, or the resource node itself.
            // This removes false ordering constraints in the DAG, allowing for more DAG parallelism
            // which means there are more possible ways to reorder the nodes, to find orderings that
            // score better in the optimization function and so are thus more optimized.
            int parentNodeIndex = connectionInfo.nodeIndex;
            int parentPinIndex = connectionInfo.pinIndex;
            while (1)
            {
                // If we found a resource node, we are done.
                if (GetNodeIsResourceNode(renderGraph.nodes[parentNodeIndex]))
                {
                    rgdag.dag[index].dependentOn.push_back(parentNodeIndex);
                    break;
                }

                // if we found a non read only connection, the parent of this connection is the one to be dependent on.
                Assert(parentPinIndex != -1, "Error");
                InputNodeInfo inputNodeInfo = GetNodePinInputNodeInfo(renderGraph.nodes[parentNodeIndex], parentPinIndex);
                Assert(inputNodeInfo.nodeIndex != -1, "Error");
                if (!ShaderResourceTypeIsReadOnly(inputNodeInfo.access))
                {
                    rgdag.dag[index].dependentOn.push_back(parentNodeIndex);
                    break;
                }

                // iterate to next connection
                parentNodeIndex = inputNodeInfo.nodeIndex;
                parentPinIndex = inputNodeInfo.pinIndex;
            }
#endif
        }
    }

    // Do topological sorting to flatten the DAG
    // Process all possible orderings of the nodes in the DAG to find the best one
    std::vector<DAG> processing;
    processing.push_back(rgdag);
    RenderGraph bestRenderGraph = renderGraph;
    float bestRenderGraphScore = FLT_MAX;
    while (!processing.empty())
    {
        // get the next partially flattened DAG to process
        DAG dag = processing.back();
        processing.pop_back();

        // make a dag on the processing stack, taking each choice we have for the next item in the flattened list
        for (size_t index = 0; index < dag.dag.size(); ++index)
        {
            if (!dag.dag[index].dependentOn.empty())
                continue;
            int chosenNodeIndex = dag.dag[index].nodeIndex;

            // make the choice
            // 1) Add it to the flattened list
            // 2) Remove the node from the dag
            // 3) Clear any dependencies on this node index
            DAG newDag = dag;
            newDag.flattenedList.push_back(chosenNodeIndex);
            newDag.dag.erase(newDag.dag.begin() + index);
            for (DAGNode& dagNode : newDag.dag)
                dagNode.dependentOn.erase(std::remove(dagNode.dependentOn.begin(), dagNode.dependentOn.end(), chosenNodeIndex), dagNode.dependentOn.end());

            // if there is more processing to do, add it to the queue
            if (!newDag.dag.empty())
            {
                processing.push_back(newDag);
            }
            // else we are done so consider keeping it
            else
            {
                // reverse the order of the flattened lists since we built it backwards
                std::reverse(newDag.dag.begin(), newDag.dag.end());

                // score this proposed flattened render graph
                RenderGraph candidate = renderGraph;
                candidate.flattenedNodeList = newDag.flattenedList;

                // Remove all barrier nodes from the list now that the render graph is flattened.
                // We don't want it adding extra resource transitions, and it is a no-op at runtime.
                candidate.flattenedNodeList.erase(
                    std::remove_if(candidate.flattenedNodeList.begin(), candidate.flattenedNodeList.end(),
                        [&candidate] (int nodeIndex)
                        {
                            return candidate.nodes[nodeIndex]._index == RenderGraphNode::c_index_actionBarrier || candidate.nodes[nodeIndex]._index == RenderGraphNode::c_index_reroute;
                        }
                    ),
                    candidate.flattenedNodeList.end()
                );

                // calculate the render graph score and keep it if it's the best we've seen so far
                CalculateResourceTransitions(candidate);
                float candidateScore = CalculateRenderGraphScore(candidate);
                if (candidateScore < bestRenderGraphScore)
                {
                    bestRenderGraphScore = candidateScore;
                    bestRenderGraph = candidate;
                }
            }
            break;
        }
    }

    // Keep the best render graph we found
    renderGraph = bestRenderGraph;
}
