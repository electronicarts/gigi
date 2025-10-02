///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace FrontEndNodes
{
    inline int GetPinCount(const RenderGraphNode_Action_WorkGraph& node)
    {
        int ret = 0;

        ret += node.entryShader.shader ? (int)node.entryShader.shader->resources.size() : 0;

        ret +=
            1 + // records buffer
            1 + // shading rate image
            1 + // depth target
            (int)node.colorTargets.size(); // color targets

        return ret;
    }

    inline std::string GetPinName(const RenderGraphNode_Action_WorkGraph& node, int pinIndex)
    {
        // required vertex shader resource pins
        if (node.entryShader.shader)
        {
            if (pinIndex < node.entryShader.shader->resources.size())
                return node.entryShader.shader->resources[pinIndex].name;
            pinIndex -= (int)node.entryShader.shader->resources.size();
        }

        // optional records buffer
        if (pinIndex == 0)
            return "recordsBuffer";
        pinIndex--;

        // optional shading rate image
        if (pinIndex == 0)
            return "shadingRateImage";
        pinIndex--;

        // optional depth target
        if (pinIndex == 0)
            return "depthTarget";
        pinIndex--;

        // optional color targets
        for (int i = 0; i < node.colorTargets.size(); ++i)
        {
            if (pinIndex == 0)
            {
                char buffer[256];
                sprintf_s(buffer, "colorTarget%i", i);
                return buffer;
            }
            pinIndex--;
        }

        return "";
    }

    inline InputNodeInfo GetPinInputNodeInfo(const RenderGraphNode_Action_WorkGraph& node, int pinIndex)
    {
        int origPinIndex = pinIndex;

        InputNodeInfo ret;
        ret.nodeIndex = -1;

        // Required pixel shader resource pins
        if (node.entryShader.shader)
        {
            if (pinIndex < node.entryShader.shader->resources.size())
            {
                for (size_t index = 0; index < node.connections.size(); ++index)
                {
                    const NodePinConnection& connection = node.connections[index];
                    if (connection.srcNodePinIndex == origPinIndex)
                    {
                        ret.access = node.entryShader.shader->resources[index].access;
                        ret.originalAccess = node.entryShader.shader->resources[index].originalAccess;
                        ret.nodeIndex = connection.dstNodeIndex;
                        ret.pinIndex = connection.dstNodePinIndex;
                        return ret;
                    }
                }
            }
            pinIndex -= (int)node.entryShader.shader->resources.size();
        }

        // Optional records buffer
        if (pinIndex == 0)
        {
            ret.access = ShaderResourceAccessType::SRV; // TODO: jan  check
            ret.nodeIndex = node.records.nodeIndex;
            ret.pinIndex = node.records.nodePinIndex;
            ret.required = false;
            return ret;
        }
        pinIndex--;

        // Optional shading rate image
        if (pinIndex == 0)
        {
            ret.access = ShaderResourceAccessType::ShadingRate;
            ret.nodeIndex = node.shadingRateImage.nodeIndex;
            ret.pinIndex = node.shadingRateImage.nodePinIndex;
            ret.required = false;
            return ret;
        }
        pinIndex--;

        // Optional depth target
        if (pinIndex == 0)
        {
            ret.access = ShaderResourceAccessType::DepthTarget;
            ret.nodeIndex = node.depthTarget.nodeIndex;
            ret.pinIndex = node.depthTarget.nodePinIndex;
            ret.required = false;
            return ret;
        }
        pinIndex--;

        // Optional Color Targets
        for (int i = 0; i < node.colorTargets.size(); ++i)
        {
            if (pinIndex == 0)
            {
                ret.access = ShaderResourceAccessType::RenderTarget;
                ret.nodeIndex = node.colorTargets[i].nodeIndex;
                ret.pinIndex = node.colorTargets[i].nodePinIndex;
                ret.required = false;
                return ret;
            }
            pinIndex--;
        }

        return ret;

    }

};
