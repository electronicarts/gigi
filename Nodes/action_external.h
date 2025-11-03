///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace FrontEndNodes
{
    inline int GetPinCount(const RenderGraphNode_Action_External& node)
    {
        StaticNodeInfo staticNodeInfo = GetStaticNodeInfo(node);
        return (int)staticNodeInfo.pins.size();
    }

    inline std::string GetPinName(const RenderGraphNode_Action_External& node, int pinIndex)
    {
        StaticNodeInfo staticNodeInfo = GetStaticNodeInfo(node);
        return staticNodeInfo.pins[pinIndex].srcPin;
    }

    inline InputNodeInfo GetPinInputNodeInfo(const RenderGraphNode_Action_External& node, int pinIndex)
    {
        InputNodeInfo ret;

        switch (node.externalNodeData._index)
        {
            case ExternalNodeData::c_index_AMD_FidelityFXSDK_Upscaling:
            {
                StaticNodeInfo staticNodeInfo = GetStaticNodeInfo(node);

                struct TempPinInfo
                {
                    int nodeIndex;
                    int pinIndex;
                };

                #define HANDLE_PIN(name) \
                    { node.externalNodeData.AMD_FidelityFXSDK_Upscaling.name.nodeIndex, node.externalNodeData.AMD_FidelityFXSDK_Upscaling.name.nodePinIndex },

                static const TempPinInfo tempPinInfo[] = {
                    HANDLE_PIN(color)
                    HANDLE_PIN(colorOpaqueOnly)
                    HANDLE_PIN(depth)
                    HANDLE_PIN(motionVectors)
                    HANDLE_PIN(exposure)
                    HANDLE_PIN(reactive)
                    //HANDLE_PIN(transparencyAndComposition)
                    HANDLE_PIN(output)
                };

                #undef HANDLE_PIN

                ret.nodeIndex = tempPinInfo[pinIndex].nodeIndex;
                ret.pinIndex = tempPinInfo[pinIndex].pinIndex;
                ret.access = staticNodeInfo.pins[pinIndex].access;
                ret.required = staticNodeInfo.pins[pinIndex].required;

                break;
            }
            default:
            {
                Assert(false, "Unhandled external node type for node %s", node.name.c_str());
            }
        }
        
        return ret;
    }
};
