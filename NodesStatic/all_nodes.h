///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2026 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// This file is meant to be a minimal centralized interface to get information about nodes.
// It is meant to replace the other interfaces as much as possible.
// Not all nodes are implemented in this new way yet. They should be converted, and the old interfaces be deleted / minimized.

#include "Schemas/Types.h"

struct StaticNodePinInfo
{
    std::string srcPin;  // The name of this pin
    std::string* dstNode = nullptr; // The name of the node plugged into this pin
    std::string* dstPin = nullptr;  // The name of the pin on the node plugged into this pin

    std::string toolTip;

    bool outputOnly = false;
    bool readOnly = true;
    bool required = true;

    ShaderResourceAccessType access = ShaderResourceAccessType::Count;
};

struct StaticNodeInfo
{
    bool backendSupported[EnumCount<Backend>()] = {};
    std::vector<StaticNodePinInfo> pins;
};

inline bool ShaderResourceTypeIsReadOnly(ShaderResourceAccessType access)
{
    switch (access)
    {
        case ShaderResourceAccessType::UAV: return false;
        case ShaderResourceAccessType::RTScene: return true;
        case ShaderResourceAccessType::SRV: return true;
        case ShaderResourceAccessType::CopySource: return true;
        case ShaderResourceAccessType::CopyDest: return false;
        case ShaderResourceAccessType::CBV: return true;
        case ShaderResourceAccessType::Indirect: return true;
        case ShaderResourceAccessType::VertexBuffer: return true;
        case ShaderResourceAccessType::IndexBuffer: return true;
        case ShaderResourceAccessType::RenderTarget: return false;
        case ShaderResourceAccessType::DepthTarget: return false;
        case ShaderResourceAccessType::Barrier: return false;
        case ShaderResourceAccessType::ShadingRate: return true;
    }

    //GigiAssert(false, "Unhandled ShaderResourceType: %i", access);
    return true;
}

#include "action_external.h"
