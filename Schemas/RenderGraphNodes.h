///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

//========================================================
// Enums
//========================================================

#include <unordered_map>

ENUM_BEGIN(ResourceVisibility, "")
    ENUM_ITEM(Imported, "Provided as input by the host application")
    ENUM_ITEM(Internal, "Used internally to the technique only")
    ENUM_ITEM(Exported, "Managed by the technique but visible to the host application")

    ENUM_ITEM(Count, "")
ENUM_END()

ENUM_BEGIN(DrawCullMode, "")
    ENUM_ITEM(None, "")
    ENUM_ITEM(Front, "")
    ENUM_ITEM(Back, "")
ENUM_END()

ENUM_BEGIN(DepthTestFunction, "")
    ENUM_ITEM(Never, "")
    ENUM_ITEM(Less, "")
    ENUM_ITEM(Equal, "")
    ENUM_ITEM(LessEqual, "")
    ENUM_ITEM(Greater, "")
    ENUM_ITEM(NotEqual, "")
    ENUM_ITEM(GreaterEqual, "")
    ENUM_ITEM(Always, "")
ENUM_END()

ENUM_BEGIN(DrawBlendMode, "")
    ENUM_ITEM(Zero, "")
    ENUM_ITEM(One, "")
    ENUM_ITEM(SrcColor, "")
    ENUM_ITEM(InvSrcColor, "")
    ENUM_ITEM(SrcAlpha, "")
    ENUM_ITEM(InvSrcAlpha, "")
    ENUM_ITEM(DestAlpha, "")
    ENUM_ITEM(InvDestAlpha, "")
    ENUM_ITEM(DestColor, "")
    ENUM_ITEM(InvDestColor, "")
ENUM_END()

ENUM_BEGIN(StencilOp, "")
    ENUM_ITEM(Keep, "")
    ENUM_ITEM(Zero, "")
    ENUM_ITEM(Replace, "")
    ENUM_ITEM(IncrementSaturate, "")
    ENUM_ITEM(DecrimentSaturate, "")
    ENUM_ITEM(Invert, "")
    ENUM_ITEM(Increment, "")
    ENUM_ITEM(Decriment, "")
ENUM_END()

ENUM_BEGIN(ShadingRate, "Used by variable rate shading")
    ENUM_ITEM(_1x1, "")
    ENUM_ITEM(_1x2, "")
    ENUM_ITEM(_2x1, "")
    ENUM_ITEM(_2x2, "")
    ENUM_ITEM(_2x4, "")
    ENUM_ITEM(_4x2, "")
    ENUM_ITEM(_4x4, "")
ENUM_END()

ENUM_BEGIN(ShadingRateCombiner, "Used by variable rate shading")
    ENUM_ITEM(PassThrough, "")
    ENUM_ITEM(Override, "")
    ENUM_ITEM(Min, "")
    ENUM_ITEM(Max, "")
    ENUM_ITEM(Sum, "")
ENUM_END()

ENUM_BEGIN(GeometryType, "")
    ENUM_ITEM(TriangleList, "")
    ENUM_ITEM(LineList, "")
    ENUM_ITEM(PointList, "")
ENUM_END()

#include "TextureFormats.h"

//========================================================
// Helper Structures
//========================================================

STRUCT_BEGIN(NodeReference, "A generic node reference")
    STRUCT_FIELD(std::string, name, "", "The name of the node", 0)

    STRUCT_FIELD(int, nodeIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_INHERIT_BEGIN(TextureNodeReference, NodeReference, "A texture node reference")
    STRUCT_FIELD(struct RenderGraphNode_Resource_Texture*, textureNode, nullptr, "A pointer to the texture node", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_INHERIT_BEGIN(BufferNodeReference, NodeReference, "A buffer node reference")
    STRUCT_FIELD(struct RenderGraphNode_Resource_Buffer*, bufferNode, nullptr, "A pointer to the buffer node", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_INHERIT_BEGIN(TextureOrBufferNodeReference, NodeReference, "A texture or buffer node reference")
    STRUCT_FIELD(struct RenderGraphNode_Resource_Texture*, textureNode, nullptr, "A pointer to the texture node", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(struct RenderGraphNode_Resource_Buffer*, bufferNode, nullptr, "A pointer to the texture node", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(NodePinReference, "A reference to a pin on a node")
    STRUCT_FIELD(std::string, node, "", "The name of the node", 0)
    STRUCT_FIELD(std::string, pin, "", "The name of the pin", 0)

    STRUCT_FIELD(int, nodeIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(int, nodePinIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(int, resourceNodeIndex, -1, "Calculated for convenience. The resource node ultimately plugged into this pin.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(NodePinReferenceOptional, "A reference to a pin on a node")
    STRUCT_FIELD(std::string, node, "", "The name of the node", 0)
    STRUCT_FIELD(std::string, pin, "", "The name of the pin", 0)

    STRUCT_FIELD(int, nodeIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(int, nodePinIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(int, resourceNodeIndex, -1, "Calculated for convenience. The resource node ultimately plugged into this pin.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(NodePinConnection, "A connection from a node to another node")
    STRUCT_FIELD(std::string, srcPin, "", "The name of this pin", 0)
    STRUCT_FIELD(std::string, dstNode, "", "The name of the node", 0)
    STRUCT_FIELD(std::string, dstPin, "", "The name of the pin", 0)
    STRUCT_FIELD(BackendRestriction, backends, {}, "The backends this connection is present for.", 0)

    STRUCT_FIELD(int, srcNodePinIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(int, dstNodeIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(int, dstNodePinIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(TextureSizeDesc, "size = (inputSize + preAdd) * multiply / divide + postAdd.  inputSize is (1,1,1) if nothing given.")
    STRUCT_FIELD(TextureNodeReference, node, {}, "The node referenced, if there is one.", 0)
    STRUCT_FIELD(VariableReference, variable, {}, "If a variable is given, the size will be based on the value of the variable", 0)
    STRUCT_STATIC_ARRAY(int, multiply, 3, { 1 COMMA 1 COMMA 1 }, "", SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)
    STRUCT_STATIC_ARRAY(int, divide, 3, { 1 COMMA 1 COMMA 1 }, "", SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)
    STRUCT_STATIC_ARRAY(int, preAdd, 3, { 0 COMMA 0 COMMA 0 }, "", SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)
    STRUCT_STATIC_ARRAY(int, postAdd, 3, { 0 COMMA 0 COMMA 0 }, "", SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)
STRUCT_END()

STRUCT_BEGIN(DispatchSizeDesc, "The number of threads to dispatch. Not thread groups.  size = (inputSize + preAdd) * multiply / divide + postAdd.  inputSize is (1,1,1) if nothing given.")
    STRUCT_FIELD(TextureOrBufferNodeReference, node, {}, "If a texture or buffer is specified, the dispatch size will be based on the size of the texture or buffer", 0)
    STRUCT_FIELD(VariableReference, variable, {}, "If a variable is given, the dispatch size will be based on the value of the variable", 0)
    STRUCT_FIELD(NodePinReferenceOptional, indirectBuffer, {}, "If given, this buffer will be used as an indirect dispatch buffer", SCHEMA_FLAG_NO_UI)
    STRUCT_STATIC_ARRAY(int, multiply, 3, { 1 COMMA 1 COMMA 1 }, "", SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)
    STRUCT_STATIC_ARRAY(int, divide, 3, { 1 COMMA 1 COMMA 1 }, "", SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)
    STRUCT_STATIC_ARRAY(int, preAdd, 3, { 0 COMMA 0 COMMA 0 }, "", SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)
    STRUCT_STATIC_ARRAY(int, postAdd, 3, { 0 COMMA 0 COMMA 0 }, "", SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)
    STRUCT_FIELD(VariableReference, indirectOffsetVariable, {}, "If a variable is given, it will be used as the offset into the indirect dispatch buffer. 0 would be the start of the buffer, 1 would start at the 4th value in the buffer, and so on.", 0)
    STRUCT_FIELD(int, indirectOffsetValue, 0, "The offset into the indirect dispatch buffer if no variable given.  0 would be the start of the buffer, 1 would start at the 4th value in the buffer, and so on.", 0)
STRUCT_END()

STRUCT_BEGIN(RayDispatchSizeDesc, "size = (inputSize + preAdd) * multiply / divide + postAdd.  inputSize is (1,1,1) if nothing given.")
    STRUCT_FIELD(TextureOrBufferNodeReference, node, {}, "If a texture or buffer is specified, the dispatch size will be based on the size of the texture or buffer", 0)
    STRUCT_FIELD(VariableReference, variable, {}, "If a variable is given, the dispatch size will be based on the value of the variable", 0)
    STRUCT_STATIC_ARRAY(int, multiply, 3, { 1 COMMA 1 COMMA 1 }, "", SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)
    STRUCT_STATIC_ARRAY(int, divide, 3, { 1 COMMA 1 COMMA 1 }, "", SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)
    STRUCT_STATIC_ARRAY(int, preAdd, 3, { 0 COMMA 0 COMMA 0 }, "", SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)
    STRUCT_STATIC_ARRAY(int, postAdd, 3, { 0 COMMA 0 COMMA 0 }, "", SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)
STRUCT_END()

STRUCT_BEGIN(BufferCountDesc, "count = (inputCount + preAdd) * multiply / divide + postAdd.  inputCount is 1 if nothing given.")
    STRUCT_FIELD(BufferNodeReference, node, {}, "The node referenced, if there is one.", 0)
    STRUCT_FIELD(VariableReference, variable, {}, "The variable referenced, if there is one.", 0)
    STRUCT_FIELD(int, multiply, 1, "", 0)
    STRUCT_FIELD(int, divide, 1, "", 0)
    STRUCT_FIELD(int, preAdd, 0, "", 0)
    STRUCT_FIELD(int, postAdd, 0, "", 0)
STRUCT_END()

STRUCT_BEGIN(TextureFormatDesc, "Specifies a texture format")
    STRUCT_FIELD(TextureNodeReference, node, {}, "This node's format is used, if specified", 0)
    STRUCT_FIELD(TextureFormat, format, TextureFormat::Any, "A specific format can be specified", 0)
    STRUCT_FIELD(VariableReference, variable, {}, "The variable that holds the texture format. Assumed to be a uint32.", 0)
STRUCT_END()

STRUCT_BEGIN(BufferFormatDesc, "Specifies a buffer format")
    STRUCT_FIELD(BufferNodeReference, node, {}, "This node's format is used, if specified", 0)
    STRUCT_FIELD(StructReference, structureType, {}, "The buffer type, if it is a structured buffer", 0)
    STRUCT_FIELD(DataFieldType, type, DataFieldType::Count, "The buffer type. May become a typed or structured buffer, depending on the type chosen.", 0)
STRUCT_END()

STRUCT_BEGIN(ResourceDependency, "Specifies a resource dependency")
    STRUCT_FIELD(int, nodeIndex, -1, "The index of the resource node.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(int, pinIndex, -1, "The pin index of this action node", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(ShaderResourceAccessType, access, ShaderResourceAccessType::Count, "How the resource is accessed.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(ShaderResourceType, type, ShaderResourceType::Count, "What type of resource it is.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(SetCBFromVar, "Specify that a structure field should be set to the value of a variable")
    STRUCT_FIELD(std::string, field, "", "The name of the field to set", 0)
    STRUCT_FIELD(VariableReference, variable, {}, "The name of the variable to take the value from", 0)
STRUCT_END()

STRUCT_BEGIN(ColorTargetSettings, "Settings for a color target in a draw call")
    STRUCT_FIELD(bool, clear, false, "If true, clears the color target before drawing", 0)
    STRUCT_STATIC_ARRAY(float, clearColor, 4, { 1.0f COMMA 1.0f COMMA 1.0f COMMA 1.0f}, "The color to clear the render target", SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)
    STRUCT_STATIC_ARRAY(bool, writeChannels, 4, { true COMMA true COMMA true COMMA true }, "Controls whether R,G,B,A get written or not", SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)

    STRUCT_FIELD(bool, enableBlending, false, "Enables alpha blending", 0)
    STRUCT_FIELD(DrawBlendMode, srcBlend, DrawBlendMode::One, "", 0)
    STRUCT_FIELD(DrawBlendMode, destBlend, DrawBlendMode::Zero, "", 0)
    STRUCT_FIELD(DrawBlendMode, srcBlendAlpha, DrawBlendMode::One, "", 0)
    STRUCT_FIELD(DrawBlendMode, destBlendAlpha, DrawBlendMode::Zero, "", 0)

    STRUCT_FIELD(int, arrayIndex, 0, "The array index, for Texture2DArray, Texture3D etc", 0)
    STRUCT_FIELD(int, mipLevel, 0, "The mip to use", 0)
STRUCT_END()

STRUCT_BEGIN(SubgraphDataVariable, "Variables in the subgraph")
    STRUCT_FIELD(std::string, name, "", "variable name", SCHEMA_FLAG_UI_CONST)
    STRUCT_FIELD(VariableVisibility, visibility, VariableVisibility::Internal, "Who can see and interact with this variable", 0)
STRUCT_END()

STRUCT_BEGIN(SubGraphData, "Cached data about a subgraph")
    STRUCT_DYNAMIC_ARRAY(std::string, importedResources, "input output pins", 0)
    STRUCT_DYNAMIC_ARRAY(std::string, exportedResources, "output only pins", 0)
    STRUCT_DYNAMIC_ARRAY(SubgraphDataVariable, variables, "Variables in the subgraph", 0)
STRUCT_END()

STRUCT_BEGIN(SubGraphVariableSettings, "Cached data about a subgraph")
    STRUCT_FIELD(std::string, name, "", "variable name", SCHEMA_FLAG_UI_CONST)
    STRUCT_FIELD(VariableVisibility, visibility, VariableVisibility::Internal, "Who can see and interact with this variable", 0)
    STRUCT_FIELD(std::string, replaceWithStr, {}, "If set, the subgraph variable will be deleted and all references will use this parent graph variable instead.", 0)
    STRUCT_FIELD(std::string, replaceWithValue, {}, "Replace the variable with a literal value. At gigi compile time it makes an internal private variable of the correct type with this string as the default value.", 0)
    STRUCT_FIELD(bool, isLoopIndex, false, "If true, this variable will recieve the loop index.", 0)

    // deprecated in 0.97b
    // replaced by replaceWithStr
    STRUCT_FIELD(VariableReference, replaceWith, {}, "If set, the subgraph variable will be deleted and all references will use this parent graph variable instead.", SCHEMA_FLAG_NO_UI)
STRUCT_END()

STRUCT_BEGIN(LinkProperties, "Properties of links between nodes.")
    STRUCT_FIELD(int, UAVMipIndex, 0, "The mip index to use, if this is a UAV connection.", 0)
STRUCT_END()

//========================================================
// Base Nodes
//========================================================

STRUCT_BEGIN(RenderGraphNode_Base, "The base type of all node types")
    STRUCT_FIELD(std::string, name, "", "The name of the node", 0)
    STRUCT_FIELD(BackendRestriction, backends, {}, "This node can be limited to specific backends", SCHEMA_FLAG_UI_COLLAPSABLE)
    STRUCT_FIELD(std::string, comment, "", "A comment about the node which can appear in generated code", 0)

    STRUCT_STATIC_ARRAY(float, editorPos, 2, { 0.0f COMMA 0.0f }, "The position of the node in the editor", SCHEMA_FLAG_NO_UI)
    STRUCT_FIELD(std::unordered_map<std::string COMMA int>, inputPinIds, {}, "", SCHEMA_FLAG_NO_UI | SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(std::unordered_map<std::string COMMA int>, outputPinIds, {}, "", SCHEMA_FLAG_NO_UI | SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(int, nodeIndex, -1, "The index in the list of render graph nodes. This is filled in after loading by the ReferenceFixupVisitor and is in [0,N) with no gaps.", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(std::string, originalName, "", "The name before renames and sanitization", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_INHERIT_BEGIN(RenderGraphNode_ResourceBase, RenderGraphNode_Base, "The base type for resource node types")
    STRUCT_FIELD(bool, transient, true, "A non transient resource will persist across executions and frames. A transient resource may not, and is decided by the backend as a potential optimization opportunity.", 0)

    STRUCT_CONST(bool, c_isResourceNode, true, "Whether or not this is a resource node.", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(ShaderResourceAccessType, startingState, ShaderResourceAccessType::Count, "The first state that a reosurce is in. Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(ShaderResourceAccessType, finalState, ShaderResourceAccessType::Count, "The last state that a reosurce is in. Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(unsigned int, accessedAs, 0, "A bitfield of all the ways this resource is accessed (ShaderResourceAccessType). Useful for creating resources with the correct usage flags.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_INHERIT_BEGIN(RenderGraphNode_ActionBase, RenderGraphNode_Base, "The base type for action node types")
    STRUCT_CONST(bool, c_isResourceNode, false, "Whether or not this is a resource node.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_DYNAMIC_ARRAY(ResourceDependency, resourceDependencies, "Filled in before backend code is called.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(Condition, condition, {}, "An optional condition added for the action to happen", SCHEMA_FLAG_UI_COLLAPSABLE)

    STRUCT_DYNAMIC_ARRAY(LinkProperties, linkProperties, "Specify array index / mip level for each pin. Should be same size and order as GetNodePins family of functons.", SCHEMA_FLAG_NO_UI)

    STRUCT_DYNAMIC_ARRAY(NodePinConnection, connections, "What is plugged into the pins", SCHEMA_FLAG_NO_UI)
STRUCT_END()

//========================================================
// Resource Nodes
//========================================================

STRUCT_INHERIT_BEGIN(RenderGraphNode_Resource_Buffer, RenderGraphNode_ResourceBase, "Declares a buffer")
    STRUCT_CONST(std::string, c_editorName, "Buffer", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(std::string, c_shortTypeName, "Buffer", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(std::string, c_shorterTypeName, "Buff", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(bool, c_showInEditor, true, "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(ResourceVisibility, visibility, ResourceVisibility::Internal, "Who can see the buffer and who owns it", 0)
    STRUCT_FIELD(BufferFormatDesc, format, {}, "The format of the texture.", SCHEMA_FLAG_UI_COLLAPSABLE)
    STRUCT_FIELD(BufferCountDesc, count, {}, "How many items are in the buffer.", SCHEMA_FLAG_UI_COLLAPSABLE)
STRUCT_END()

STRUCT_INHERIT_BEGIN(RenderGraphNode_Resource_ShaderConstants, RenderGraphNode_ResourceBase, "Declares a shader constant buffer")
    STRUCT_CONST(std::string, c_editorName, "Constants", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(std::string, c_shortTypeName, "Constants", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(std::string, c_shorterTypeName, "Const", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(bool, c_showInEditor, false, "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(StructReference, structure, {}, "The structure of the constant buffer.", 0)

    STRUCT_DYNAMIC_ARRAY(SetCBFromVar, setFromVar, "Set constant buffer (left) to the value of variable (right) every execution", SCHEMA_FLAG_UI_COLLAPSABLE | SCHEMA_FLAG_UI_ARRAY_FATITEMS)
STRUCT_END()

STRUCT_INHERIT_BEGIN(RenderGraphNode_Resource_Texture, RenderGraphNode_ResourceBase, "Declares a texture")
    STRUCT_CONST(std::string, c_editorName, "Texture", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(std::string, c_shortTypeName, "Texture", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(std::string, c_shorterTypeName, "Tex", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(bool, c_showInEditor, true, "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(ResourceVisibility, visibility, ResourceVisibility::Internal, "Who can see the texture and who owns it", 0)
    STRUCT_FIELD(TextureFormatDesc, format, {}, "The format of the texture.", SCHEMA_FLAG_UI_COLLAPSABLE)
    STRUCT_FIELD(TextureSizeDesc, size, {}, "The size of the texture.", SCHEMA_FLAG_UI_COLLAPSABLE)
    STRUCT_FIELD(unsigned int, numMips, 1, "The number of mips the texture should have. 0 means the full set. 1 means just the full size image, and no smaller mips.", 0)
    STRUCT_FIELD(TextureDimensionType, dimension, TextureDimensionType::Texture2D, "The dimensionality of the texture", 0)

    STRUCT_FIELD(std::string, loadFileName, "", "If not empty, it load it as an image. This is currently not exposed to users, and is created when shaders use an image token", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(bool, loadFileNameAsSRGB, true, "If true, the source file is treated as sRGB", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

//========================================================
// Action Nodes
//========================================================

STRUCT_INHERIT_BEGIN(RenderGraphNode_Action_ComputeShader, RenderGraphNode_ActionBase, "Executes a compute shader")
    STRUCT_CONST(std::string, c_editorName, "Compute Shader", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(std::string, c_shortTypeName, "Compute", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(std::string, c_shorterTypeName, "CS", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(bool, c_showInEditor, true, "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(ComputeShaderReference, shader, {}, "The shader.", 0)
    STRUCT_FIELD(DispatchSizeDesc, dispatchSize, {}, "The dispatch size.", SCHEMA_FLAG_UI_COLLAPSABLE)

    STRUCT_FIELD(std::string, entryPoint, "", "The shader entrypoint. Overrides the shader entry entryPoint.", 0)
    STRUCT_DYNAMIC_ARRAY(ShaderDefine, defines, "The defines the shader is compiled with, on top of whatever defines the shader has already", SCHEMA_FLAG_UI_COLLAPSABLE | SCHEMA_FLAG_UI_ARRAY_FATITEMS)
STRUCT_END()

STRUCT_INHERIT_BEGIN(RenderGraphNode_Action_RayShader, RenderGraphNode_ActionBase, "Executes a dispatch rays shader")
    STRUCT_CONST(std::string, c_editorName, "Ray Gen Shader", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(std::string, c_shortTypeName, "RayGen", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(std::string, c_shorterTypeName, "RGS", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(bool, c_showInEditor, true, "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(RayGenShaderReference, shader, {}, "The ray gen shader.", 0)
    STRUCT_FIELD(RayDispatchSizeDesc, dispatchSize, {}, "The dispatch size.", SCHEMA_FLAG_UI_COLLAPSABLE)

    STRUCT_FIELD(std::string, entryPoint, "", "The shader entrypoint. Overrides the shader entry entryPoint.", 0)
    STRUCT_DYNAMIC_ARRAY(ShaderDefine, defines, "The defines the shader is compiled with, on top of whatever defines the shader has already", SCHEMA_FLAG_UI_COLLAPSABLE)

    STRUCT_FIELD(int, maxRecursionDepth, 3, "The maximum recursion depth of the ray.", 0)
    STRUCT_FIELD(unsigned int, rayPayloadSize, 64, "The size of the ray payload, in bytes. 64 bytes is four float4s.", 0)
STRUCT_END()

STRUCT_INHERIT_BEGIN(RenderGraphNode_Action_CopyResource, RenderGraphNode_ActionBase, "Copies a resource to another resource")
    STRUCT_CONST(std::string, c_editorName, "Copy Resource", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(std::string, c_shortTypeName, "Copy", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(std::string, c_shorterTypeName, "Copy", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(bool, c_showInEditor, true, "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(NodePinReference, source, {}, "The resource being copied from.", SCHEMA_FLAG_NO_UI)
    STRUCT_FIELD(NodePinReference, dest, {}, "The resource being copied to.", SCHEMA_FLAG_NO_UI)
STRUCT_END()

STRUCT_INHERIT_BEGIN(RenderGraphNode_Action_DrawCall, RenderGraphNode_ActionBase, "Rasterization")
    STRUCT_CONST(std::string, c_editorName, "Draw Call", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(std::string, c_shortTypeName, "Draw", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(std::string, c_shorterTypeName, "Draw", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(bool, c_showInEditor, true, "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(AmplificationShaderReferenceOptional, amplificationShader, {}, "The amplification shader.", 0)
    STRUCT_FIELD(MeshShaderReferenceOptional, meshShader, {}, "The mesh shader.", 0)
    STRUCT_FIELD(VertexShaderReferenceOptional, vertexShader, {}, "The vertex shader.", 0)
    STRUCT_FIELD(PixelShaderReference, pixelShader, {}, "The pixel shader.", 0)

    // Vertex shader specific
    STRUCT_FIELD(int, countPerInstance, -1, "If using an index buffer, this is indexCountPerInstance, else is vertexCountPerInstance.  If -1, will use the count of the buffer.  Else, if a buffer is given, will use min(buffer count, countPerInstance).", 0)
    STRUCT_FIELD(int, instanceCount, 1, "How many instances to draw. If -1, will use the count of the instance buffer. Else, if an instance buffer is given, will use min (instance buffer count, instanceCount).", 0)

    // Mesh shader specific
    STRUCT_FIELD(DispatchSizeDesc, meshShaderDispatchSize, {}, "The mesh shader dispatch size.", SCHEMA_FLAG_UI_COLLAPSABLE)

    // Depth Settings
    STRUCT_FIELD(bool, depthTargetClear, false, "If true, clears the depth target before doing a draw call.", 0)
    STRUCT_FIELD(float, depthTargetClearValue, 0.0f, "The value to clear the depth target to.", 0)
    STRUCT_FIELD(bool, depthWrite, true, "If false, disables writing to the depth buffer", 0)
    STRUCT_FIELD(int, depthArrayIndex, 0, "The array index, for Texture2DArray, Texture3D etc", 0)
    STRUCT_FIELD(int, depthMipLevel, 0, "The mip to use", 0)
    STRUCT_FIELD(DepthTestFunction, depthTest, DepthTestFunction::Less, "", 0)

    // Stencil Settings
    STRUCT_FIELD(bool, stencilClear, false, "If true, clears the stencil before doing a draw call.", 0)
    STRUCT_FIELD(uint8_t, stencilClearValue, 0, "", 0)
    STRUCT_FIELD(uint8_t, stencilRef, 0, "Sets the reference value for depth stencil tests.", 0)
    STRUCT_FIELD(uint8_t, stencilReadMask, 255, "", 0)
    STRUCT_FIELD(uint8_t, stencilWriteMask, 255, "", 0)
    STRUCT_FIELD(StencilOp, frontFaceStencilFail, StencilOp::Keep, "When stencil test fails", 0)
    STRUCT_FIELD(StencilOp, backFaceStencilFail, StencilOp::Keep, "When stencil test fails", 0)
    STRUCT_FIELD(StencilOp, frontFaceStencilDepthFail, StencilOp::Keep, "When stencil passes but depth fails", 0)
    STRUCT_FIELD(StencilOp, backFaceStencilDepthFail, StencilOp::Keep, "When stencil passes but depth fails", 0)
    STRUCT_FIELD(StencilOp, frontFaceStencilPass, StencilOp::Keep, "When stencil and depth both pass", 0)
    STRUCT_FIELD(StencilOp, backFaceStencilPass, StencilOp::Keep, "When stencil and depth both pass", 0)
    STRUCT_FIELD(DepthTestFunction, frontFaceStencilFunc, DepthTestFunction::Always, "How to test stencil data against existing stencil data", 0)
    STRUCT_FIELD(DepthTestFunction, backFaceStencilFunc, DepthTestFunction::Always, "How to test stencil data against existing stencil data", 0)

    STRUCT_FIELD(bool, conservativeRasterization, false, "Turns on conservative rasterization", 0)
    STRUCT_FIELD(bool, alphaAsCoverage, false, "Turns on alpha as coverage", 0)

    STRUCT_FIELD(bool, independentAlpha, false, "If false, colorTargetSettings[0] defines blend mode for all color targets", 0)

    STRUCT_STATIC_ARRAY(ColorTargetSettings, colorTargetSettings, 8, {}, "Settings for the color targets", SCHEMA_FLAG_UI_COLLAPSABLE | SCHEMA_FLAG_UI_ARRAY_FATITEMS)

    STRUCT_DYNAMIC_ARRAY(ShaderDefine, defines, "The defines the shaders ares compiled with, on top of whatever defines the shaders have already", SCHEMA_FLAG_UI_COLLAPSABLE)

    STRUCT_FIELD(DrawCullMode, cullMode, DrawCullMode::None, "", 0)
    STRUCT_FIELD(bool, frontIsCounterClockwise, true, "", 0)

    STRUCT_FIELD(ShadingRate, shadingRate, ShadingRate::_1x1, "For variable rate shading", 0)
    STRUCT_FIELD(ShadingRateCombiner, shadingRateCombiner1, ShadingRateCombiner::PassThrough, "For variable rate shading. This combines the provoking vertex shading rate with the rate set by the command list.", 0)
    STRUCT_FIELD(ShadingRateCombiner, shadingRateCombiner2, ShadingRateCombiner::PassThrough, "For variable rate shading. This combines the image based shading rate with the shading rate set by the command list and provoking vertex.", 0)
    STRUCT_FIELD(NodePinReferenceOptional, shadingRateImage, {}, "The image used to determine shading rate on the screen. Must be a 2d texture with format R8_Uint.", SCHEMA_FLAG_NO_UI)

    STRUCT_FIELD(NodePinReferenceOptional, vertexBuffer, {}, "The vertex buffer being rendered. If not given, a vertexCount needs to be given.", SCHEMA_FLAG_NO_UI)
    STRUCT_FIELD(NodePinReferenceOptional, indexBuffer, {}, "Index buffer for vertex buffer (Optional).", SCHEMA_FLAG_NO_UI)
    STRUCT_FIELD(NodePinReferenceOptional, instanceBuffer, {}, "Instance buffer (Optional).", SCHEMA_FLAG_NO_UI)

    STRUCT_STATIC_ARRAY(NodePinReferenceOptional, colorTargets, 8, {}, "Color Targets", SCHEMA_FLAG_NO_UI)
    STRUCT_FIELD(NodePinReferenceOptional, depthTarget, {}, "Depth Target", SCHEMA_FLAG_NO_UI)

    STRUCT_FIELD(GeometryType, geometryType, GeometryType::TriangleList, "What to draw", 0)
STRUCT_END()

STRUCT_INHERIT_BEGIN(RenderGraphNode_Action_SubGraph, RenderGraphNode_ActionBase, "Runs another Gigi technique")
    STRUCT_CONST(std::string, c_editorName, "Subgraph", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(std::string, c_shortTypeName, "Subgraph", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(std::string, c_shorterTypeName, "Sub", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(bool, c_showInEditor, true, "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(std::string, fileName, "", "Relative file name", 0)

    STRUCT_FIELD(SubGraphData, subGraphData, {}, "A cache of the interface of the other graph.", SCHEMA_FLAG_NO_UI)
    STRUCT_DYNAMIC_ARRAY(SubGraphVariableSettings, variableSettings, "Per variable settings for subgraph variables.", 0)

    STRUCT_FIELD(int, loopCount, 1, "Number of times to execute the technique.", 0)

    STRUCT_FIELD(VariableReferenceConstOnly, loopCountVariable, {}, "The variable to use for the loopCount. Only const variables supported currently.", 0)

    STRUCT_FIELD(int, loopIndex, -1, "When unrolling subgraph loops, the loop index of the node is stored here.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_INHERIT_BEGIN(RenderGraphNode_Action_Barrier, RenderGraphNode_ActionBase, "Causes all input operations to be executed before anything plugged into the output")
    STRUCT_CONST(std::string, c_editorName, "Barrier", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(std::string, c_shortTypeName, "Barrier", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(std::string, c_shorterTypeName, "Bar", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(bool, c_showInEditor, true, "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_INHERIT_BEGIN(RenderGraphNode_Reroute, RenderGraphNode_ActionBase, "Used in editor only to reroute links")
    STRUCT_CONST(std::string, c_editorName, "Reroute", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(std::string, c_shortTypeName, "Reroute", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(std::string, c_shorterTypeName, "Rr", "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_CONST(bool, c_showInEditor, false, "Used by the editor.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

//========================================================
// Variant
//========================================================

#include "RenderGraphNodesVariant.h"
