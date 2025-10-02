///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace FrontEndNodes
{
    inline int GetPinCount(const RenderGraphNode_Action_DrawCall& node)
    {
        int ret = 0;

        ret += node.vertexShader.shader ? (int)node.vertexShader.shader->resources.size() : 0;
        ret += node.pixelShader.shader ? (int)node.pixelShader.shader->resources.size() : 0;
        ret += node.amplificationShader.shader ? (int)node.amplificationShader.shader->resources.size() : 0;
        ret += node.meshShader.shader ? (int)node.meshShader.shader->resources.size() : 0;

        ret +=
            1 + // shading rate image
            3 + // vertex buffer, index buffer, instance buffer
            1 + // depth target
            (int)node.colorTargets.size(); // color targets

        return ret;
    }

    inline std::string GetPinName(const RenderGraphNode_Action_DrawCall& node, int pinIndex)
    {
        // required vertex shader resource pins
        if (node.vertexShader.shader)
        {
            if (pinIndex < node.vertexShader.shader->resources.size())
                return node.vertexShader.shader->resources[pinIndex].name;
            pinIndex -= (int)node.vertexShader.shader->resources.size();
        }

        // required pixel shader resource pins
        if (node.pixelShader.shader)
        {
            if (pinIndex < node.pixelShader.shader->resources.size())
                return node.pixelShader.shader->resources[pinIndex].name;
            pinIndex -= (int)node.pixelShader.shader->resources.size();
        }

        // required amplification shader resource pins
        if (node.amplificationShader.shader)
        {
            if (pinIndex < node.amplificationShader.shader->resources.size())
                return node.amplificationShader.shader->resources[pinIndex].name;
            pinIndex -= (int)node.amplificationShader.shader->resources.size();
        }

        // required mesh shader resource pins
        if (node.meshShader.shader)
        {
            if (pinIndex < node.meshShader.shader->resources.size())
                return node.meshShader.shader->resources[pinIndex].name;
            pinIndex -= (int)node.meshShader.shader->resources.size();
        }

        // optional shading rate image
        if (pinIndex == 0)
            return "shadingRateImage";
        pinIndex--;

        // optional indirect args buffer
        if (pinIndex == 0)
            return "indirectBuffer";
        pinIndex--;

        // optional vertex buffer
        if (pinIndex == 0)
            return "vertexBuffer";
        pinIndex--;

        // optional index buffer
        if (pinIndex == 0)
            return "indexBuffer";
        pinIndex--;

        // optional instance buffer
        if (pinIndex == 0)
            return "instanceBuffer";
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

    inline InputNodeInfo GetPinInputNodeInfo(const RenderGraphNode_Action_DrawCall& node, int pinIndex)
    {
        int origPinIndex = pinIndex;

        InputNodeInfo ret;
        ret.nodeIndex = -1;

        // Required vertex shader resource pins
        int shaderPinCountSoFar = 0;
        if (node.vertexShader.shader)
        {
            if (pinIndex < node.vertexShader.shader->resources.size())
            {
                for (size_t index = 0; index < node.connections.size(); ++index)
                {
                    const NodePinConnection& connection = node.connections[index];
                    if (connection.srcNodePinIndex == origPinIndex)
                    {
                        ret.access = node.vertexShader.shader->resources[index - shaderPinCountSoFar].access;
                        ret.originalAccess = node.vertexShader.shader->resources[index - shaderPinCountSoFar].originalAccess;
                        ret.nodeIndex = connection.dstNodeIndex;
                        ret.pinIndex = connection.dstNodePinIndex;
                        ret.shaderResource = &node.vertexShader.shader->resources[index - shaderPinCountSoFar];
                        return ret;
                    }
                }
            }
            pinIndex -= (int)node.vertexShader.shader->resources.size();
            shaderPinCountSoFar += (int)node.vertexShader.shader->resources.size();
        }

        // Required pixel shader resource pins
        if (node.pixelShader.shader)
        {
            if (pinIndex < node.pixelShader.shader->resources.size())
            {
                for (size_t index = 0; index < node.connections.size(); ++index)
                {
                    const NodePinConnection& connection = node.connections[index];
                    if (connection.srcNodePinIndex == origPinIndex)
                    {
                        ret.access = node.pixelShader.shader->resources[index - shaderPinCountSoFar].access;
                        ret.originalAccess = node.pixelShader.shader->resources[index - shaderPinCountSoFar].originalAccess;
                        ret.nodeIndex = connection.dstNodeIndex;
                        ret.pinIndex = connection.dstNodePinIndex;
                        ret.shaderResource = &node.pixelShader.shader->resources[index - shaderPinCountSoFar];
                        return ret;
                    }
                }
            }
            pinIndex -= (int)node.pixelShader.shader->resources.size();
            shaderPinCountSoFar += (int)node.pixelShader.shader->resources.size();
        }

        // Required amplification shader resource pins
        if (node.amplificationShader.shader)
        {
            if (pinIndex < node.amplificationShader.shader->resources.size())
            {
                for (size_t index = 0; index < node.connections.size(); ++index)
                {
                    const NodePinConnection& connection = node.connections[index];
                    if (connection.srcNodePinIndex == origPinIndex)
                    {
                        ret.access = node.amplificationShader.shader->resources[index - shaderPinCountSoFar].access;
                        ret.originalAccess = node.amplificationShader.shader->resources[index - shaderPinCountSoFar].originalAccess;
                        ret.nodeIndex = connection.dstNodeIndex;
                        ret.pinIndex = connection.dstNodePinIndex;
                        ret.shaderResource = &node.amplificationShader.shader->resources[index - shaderPinCountSoFar];
                        return ret;
                    }
                }
            }
            pinIndex -= (int)node.amplificationShader.shader->resources.size();
            shaderPinCountSoFar += (int)node.amplificationShader.shader->resources.size();
        }

        // Required mesh shader resource pins
        if (node.meshShader.shader)
        {
            if (pinIndex < node.meshShader.shader->resources.size())
            {
                for (size_t index = 0; index < node.connections.size(); ++index)
                {
                    const NodePinConnection& connection = node.connections[index];
                    if (connection.srcNodePinIndex == origPinIndex)
                    {
                        ret.access = node.meshShader.shader->resources[index - shaderPinCountSoFar].access;
                        ret.originalAccess = node.meshShader.shader->resources[index - shaderPinCountSoFar].originalAccess;
                        ret.nodeIndex = connection.dstNodeIndex;
                        ret.pinIndex = connection.dstNodePinIndex;
                        ret.shaderResource = &node.meshShader.shader->resources[index - shaderPinCountSoFar];
                        return ret;
                    }
                }
            }
            pinIndex -= (int)node.meshShader.shader->resources.size();
            shaderPinCountSoFar += (int)node.meshShader.shader->resources.size();
        }

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

        // Optional indirect buffer
        if (pinIndex == 0)
        {
            ret.access = ShaderResourceAccessType::Indirect;
            ret.nodeIndex = node.indirectBuffer.nodeIndex;
            ret.pinIndex = node.indirectBuffer.nodePinIndex;
            ret.required = false;
            return ret;
        }
        pinIndex--;

        // Optional vertex buffer
        if (pinIndex == 0)
        {
            ret.access = ShaderResourceAccessType::VertexBuffer;
            ret.nodeIndex = node.vertexBuffer.nodeIndex;
            ret.pinIndex = node.vertexBuffer.nodePinIndex;
            ret.required = false;
            return ret;
        }
        pinIndex--;

        // Optional index buffer
        if (pinIndex == 0)
        {
            ret.access = ShaderResourceAccessType::IndexBuffer;
            ret.nodeIndex = node.indexBuffer.nodeIndex;
            ret.pinIndex = node.indexBuffer.nodePinIndex;
            ret.required = false;
            return ret;
        }
        pinIndex--;

        // Optional instance buffer
        if (pinIndex == 0)
        {
            ret.access = ShaderResourceAccessType::VertexBuffer;
            ret.nodeIndex = node.instanceBuffer.nodeIndex;
            ret.pinIndex = node.instanceBuffer.nodePinIndex;
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
