///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Schemas/Types.h"

struct OnNodeRenameVisitor
{
    OnNodeRenameVisitor(RenderGraph& renderGraph_, const std::string& oldName_, const std::string& newName_)
        : renderGraph(renderGraph_)
        , oldName(oldName_)
        , newName(newName_)
    { }

    template <typename TDATA>
    bool Visit(TDATA& data, const std::string& path)
    {
        return true;
    }

    bool Visit(NodePinReference& data, const std::string& path)
    {
        if (data.node == oldName)
            data.node = newName;
        return true;
    }

    bool Visit(NodePinReferenceOptional& data, const std::string& path)
    {
        if (data.node == oldName)
            data.node = newName;
        return true;
    }

    bool Visit(TextureOrBufferNodeReference& data, const std::string& path)
    {
        if (data.name == oldName)
            data.name = newName;
        return true;
    }

    bool Visit(BufferNodeReference& data, const std::string& path)
    {
        if (data.name == oldName)
            data.name = newName;
        return true;
    }

    bool Visit(TextureNodeReference& data, const std::string& path)
    {
        if (data.name == oldName)
            data.name = newName;
        return true;
    }

    RenderGraph& renderGraph;
    const std::string& oldName;
    const std::string& newName;
};