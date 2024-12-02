///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

//========================================================
// Enums
//========================================================

ENUM_BEGIN(ShaderResourceType, "The type of a shader resource")
    ENUM_ITEM(Texture, "Unordered Access View (Read/Write)")
    ENUM_ITEM(Buffer, "Shader Resource View")
    ENUM_ITEM(ConstantBuffer, "Constant Buffer View")
    ENUM_ITEM(Count, "")
ENUM_END()

ENUM_BEGIN(ShaderResourceAccessType, "The type of a shader resource")
    ENUM_ITEM(UAV, "Unordered Access View (Read/Write)")
    ENUM_ITEM(SRV, "Shader Resource View")
    ENUM_ITEM(CBV, "Constant Buffer View")
    ENUM_ITEM(CopySource, "SRV copy source")
    ENUM_ITEM(CopyDest, "SRV copy dest")
    ENUM_ITEM(Indirect, "Used in Execute Indirect")
    ENUM_ITEM(RTScene, "The scene for raytracing (acceleration structure)")
    ENUM_ITEM(VertexBuffer, "Used as a vertex buffer")
    ENUM_ITEM(IndexBuffer, "Used as an index buffer")
    ENUM_ITEM(RenderTarget, "Used as a color buffer")
    ENUM_ITEM(DepthTarget, "Used as a depth buffer")
    ENUM_ITEM(Barrier, "Used by the barrier node")
    ENUM_ITEM(ShadingRate, "Used as a shading rate image")
    ENUM_ITEM(Count, "")
ENUM_END()

#include "DataFieldTypes.h"

ENUM_BEGIN(TextureViewType, "The type that a texture is actually viewed as, in a shader. A subset of DataFieldType.")
    ENUM_ITEM(Int, "int")
    ENUM_ITEM(Int4, "int[4]")
    ENUM_ITEM(Uint, "uint")
    ENUM_ITEM(Uint4, "uint[4]")
    ENUM_ITEM(Float, "float")
    ENUM_ITEM(Float2, "float[2]")
    ENUM_ITEM(Float3, "float[3]")
    ENUM_ITEM(Float4, "float[4]")
ENUM_END()

ENUM_BEGIN(SamplerFilter, "The type of filter a sampler uses")
    ENUM_ITEM(MinMagMipPoint, "Point")
    ENUM_ITEM(MinMagLinear_MipPoint, "Bilinear")
    ENUM_ITEM(MinMagMipLinear, "Trilinear")
    ENUM_ITEM(Count, "")
ENUM_END()

ENUM_BEGIN(SamplerAddressMode, "The sampler address mode")
    ENUM_ITEM(Clamp, "Clamp")
    ENUM_ITEM(Wrap, "Wrap")
	ENUM_ITEM(Border, "Border")
    ENUM_ITEM(Count, "")
ENUM_END()

ENUM_BEGIN(TextureDimensionType, "The type of a texture")
    ENUM_ITEM(Texture2D, "Texture2D")
    ENUM_ITEM(Texture2DArray, "Texture2DArray")
    ENUM_ITEM(Texture3D, "Texture3D")
    ENUM_ITEM(TextureCube, "TextureCube")
ENUM_END()

ENUM_BEGIN(ShaderType, "The type of a shader resource")
    ENUM_ITEM(Compute, "Ray generation shader")
    ENUM_ITEM(RTRayGen, "Ray generation shader")
    ENUM_ITEM(RTClosestHit, "Closest hit shader")
    ENUM_ITEM(RTAnyHit, "Any hit shader")
    ENUM_ITEM(RTIntersection, "Intersection shader")
    ENUM_ITEM(RTMiss, "Miss shader")
    ENUM_ITEM(Vertex, "Vertex shader")
    ENUM_ITEM(Pixel, "Pixel shader")
    ENUM_ITEM(Amplification, "Amplification shader")
    ENUM_ITEM(Mesh, "Mesh shader")
    ENUM_ITEM(Count, "")
ENUM_END()

ENUM_BEGIN(StructFieldSemantic, "Used to specify if the struct field has special meaning, such as a vertex position in a vertex buffer.")
    ENUM_ITEM(Position, "float3")
    ENUM_ITEM(Color, "float4")
    ENUM_ITEM(Normal, "float3")
    ENUM_ITEM(Tangent, "float4")
    ENUM_ITEM(UV, "float2")
    ENUM_ITEM(MaterialID, "int")
    ENUM_ITEM(ShapeID, "int")
    ENUM_ITEM(Count, "")
ENUM_END()

//========================================================
// Helper Structures
//========================================================

STRUCT_BEGIN(VariableReference, "A reference to a variable")
    STRUCT_FIELD(std::string, name, "", "The name of the variable.", 0)

    STRUCT_FIELD(int, variableIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(VariableReferenceNoConst, "A reference to a variable. No const variables allowed.")
    STRUCT_FIELD(std::string, name, "", "The name of the variable.", 0)

    STRUCT_FIELD(int, variableIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(VariableReferenceConstOnly, "A reference to a variable. Only const variables allowed.")
    STRUCT_FIELD(std::string, name, "", "The name of the variable.", 0)

    STRUCT_FIELD(int, variableIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(StructReference, "A reference to a struct")
    STRUCT_FIELD(std::string, name, "", "The name of the struct.", 0)

    STRUCT_FIELD(int, structIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(ComputeShaderReference, "A reference to a shader")
    STRUCT_FIELD(std::string, name, "", "The name of the shader", 0)

    STRUCT_FIELD(int, shaderIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(struct Shader*, shader, nullptr, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(RayGenShaderReference, "A reference to a ray gen shader")
    STRUCT_FIELD(std::string, name, "", "The name of the shader", 0)

    STRUCT_FIELD(int, shaderIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(struct Shader*, shader, nullptr, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(RTClosestHitShaderReference, "A reference to a RTClosestHit shader")
    STRUCT_FIELD(std::string, name, "", "The name of the shader", 0)

    STRUCT_FIELD(int, shaderIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(struct Shader*, shader, nullptr, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(RTClosestHitShaderReferenceOptional, "Optional version")
    STRUCT_FIELD(std::string, name, "", "The name of the shader", 0)

    STRUCT_FIELD(int, shaderIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(struct Shader*, shader, nullptr, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(RTAnyHitShaderReference, "A reference to RTAnyHit shader")
    STRUCT_FIELD(std::string, name, "", "The name of the shader", 0)

    STRUCT_FIELD(int, shaderIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(struct Shader*, shader, nullptr, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(RTAnyHitShaderReferenceOptional, "Optional version")
    STRUCT_FIELD(std::string, name, "", "The name of the shader", 0)

    STRUCT_FIELD(int, shaderIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(struct Shader*, shader, nullptr, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(RTIntersectionShaderReference, "A reference to an RTIntersection shader")
    STRUCT_FIELD(std::string, name, "", "The name of the shader", 0)

    STRUCT_FIELD(int, shaderIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(struct Shader*, shader, nullptr, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(RTIntersectionShaderReferenceOptional, "Optional version")
    STRUCT_FIELD(std::string, name, "", "The name of the shader", 0)

    STRUCT_FIELD(int, shaderIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(struct Shader*, shader, nullptr, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(VertexShaderReference, "A reference to a vertex shader")
    STRUCT_FIELD(std::string, name, "", "The name of the shader", 0)

    STRUCT_FIELD(int, shaderIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(struct Shader*, shader, nullptr, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(PixelShaderReference, "A reference to a pixel shader")
    STRUCT_FIELD(std::string, name, "", "The name of the shader", 0)

    STRUCT_FIELD(int, shaderIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(struct Shader*, shader, nullptr, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(AmplificationShaderReference, "A reference to an amplification shader")
    STRUCT_FIELD(std::string, name, "", "The name of the shader", 0)

    STRUCT_FIELD(int, shaderIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(struct Shader*, shader, nullptr, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(MeshShaderReference, "A reference to a mesh shader")
    STRUCT_FIELD(std::string, name, "", "The name of the shader", 0)

    STRUCT_FIELD(int, shaderIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(struct Shader*, shader, nullptr, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(VertexShaderReferenceOptional, "An optional reference to a vertex shader")
    STRUCT_FIELD(std::string, name, "", "The name of the shader", 0)

    STRUCT_FIELD(int, shaderIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(struct Shader*, shader, nullptr, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(PixelShaderReferenceOptional, "An optional reference to a pixel shader")
    STRUCT_FIELD(std::string, name, "", "The name of the shader", 0)

    STRUCT_FIELD(int, shaderIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(struct Shader*, shader, nullptr, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(AmplificationShaderReferenceOptional, "An optional reference to an amplification shader")
    STRUCT_FIELD(std::string, name, "", "The name of the shader", 0)

    STRUCT_FIELD(int, shaderIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(struct Shader*, shader, nullptr, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(MeshShaderReferenceOptional, "An optional reference to a mesh shader")
    STRUCT_FIELD(std::string, name, "", "The name of the shader", 0)

    STRUCT_FIELD(int, shaderIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(struct Shader*, shader, nullptr, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(StructField, "A field in a struct")
    STRUCT_FIELD(std::string, name, "", "The name of the field", 0)
    STRUCT_FIELD(DataFieldType, type, DataFieldType::Count, "The type of the field", 0)
    STRUCT_FIELD(std::string, dflt, "", "The default value", 0)
    STRUCT_FIELD(std::string, comment, "", "A comment to explain the field", 0)
    STRUCT_FIELD(std::string, Enum, "", "Integer types can specify an enum, which will then make symbols in both C++ and shader code.", 0)
    STRUCT_FIELD(StructFieldSemantic, semantic, StructFieldSemantic::Count, "Used to specify if the struct field has special meaning, such as a vertex position in a vertex buffer. If none is given, it shows up in shaders as an autonumbering text coordinate.", 0)
    STRUCT_FIELD(int, semanticIndex, 0, "Some semantics can have multiple channels, like UVs and colors", 0)

    STRUCT_FIELD(int, enumIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(size_t, sizeInBytes, 0, "The size in bytes of this field", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(Struct, "A description of a struct")
    STRUCT_FIELD(std::string, name, "", "The name of the struct.", 0)
    STRUCT_FIELD(bool, forceHostVisible, false, "If true, the struct will be visible to the host, even if the struct isn't used by anything host visible.", 0)
    STRUCT_DYNAMIC_ARRAY(StructField, fields, "The data fields", SCHEMA_FLAG_UI_ARRAY_FATITEMS)//SCHEMA_FLAG_NO_UI)

    STRUCT_FIELD(size_t, sizeInBytes, 0, "The size in bytes of this struct", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(bool, exported, false, "If true, will be visible to the host app", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(std::string, definition, "", "The parsed string definition of the struct.", SCHEMA_FLAG_UI_MULTILINETEXT | SCHEMA_FLAG_NO_UI)

    STRUCT_FIELD(std::string, originalName, "", "The name before renames and sanitization", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(std::string, scope, "", "The scope that the node lives in. A possibly nested list of subgraph node names, seperated by a dot.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(ShaderResourceBuffer, "Data specific to buffers")
    STRUCT_FIELD(DataFieldType, type, DataFieldType::Count, "The data type of the buffer if a simple type", 0)
    STRUCT_FIELD(StructReference, typeStruct, {}, "The data type of the buffer if a struct type", 0)
    STRUCT_FIELD(bool, raw, false, "If true, will be viewed raw in the shader (E.g. DX12 ByteAddressBuffer)", 0)
    STRUCT_FIELD(bool, PODAsStructuredBuffer, true, "Set this to true if you want it to be StructuredBuffer instead of a Buffer, for non structure typed buffers.", 0)
    STRUCT_FIELD(bool, globallyCoherent, false, "Set this to true if you want the resource to be declared as globallycoherent.", 0)
STRUCT_END()

STRUCT_BEGIN(ShaderResourceTexture, "Data specific to textures")
    STRUCT_FIELD(TextureDimensionType, dimension, TextureDimensionType::Texture2D, "The dimensionality of the texture", 0)
    STRUCT_FIELD(TextureViewType, viewType, TextureViewType::Float4, "The dimensionality of the texture", 0)
	STRUCT_FIELD(bool, globallyCoherent, false, "Set this to true if you want the resource to be declared as globallycoherent.", 0)
STRUCT_END()

STRUCT_BEGIN(ShaderSampler, "Data specific to samplers")
    STRUCT_FIELD(std::string, name, "", "The name of the resource in the shader", 0)
    STRUCT_FIELD(SamplerFilter, filter, SamplerFilter::MinMagMipLinear, "The type of filtering to do", 0)
    STRUCT_FIELD(SamplerAddressMode, addressMode, SamplerAddressMode::Wrap, "The sampling address mode", 0)
STRUCT_END()

STRUCT_BEGIN(ShaderResource, "A declaration of a resource that a shader wants")
    STRUCT_FIELD(std::string, name, "", "The name of the resource in the shader", 0)
    STRUCT_FIELD(ShaderResourceType, type, ShaderResourceType::Count, "The resource type", 0)
    STRUCT_FIELD(ShaderResourceAccessType, access, ShaderResourceAccessType::Count, "How the resource is accessed", 0)
    STRUCT_FIELD(ShaderResourceBuffer, buffer, {}, "Data specific to buffers", SCHEMA_FLAG_UI_COLLAPSABLE)
    STRUCT_FIELD(ShaderResourceTexture, texture, {}, "Data specific to textures", SCHEMA_FLAG_UI_COLLAPSABLE)
    STRUCT_FIELD(BackendRestriction, backends, {}, "The backends this resource is present for.", SCHEMA_FLAG_UI_COLLAPSABLE)

    STRUCT_FIELD(int, registerIndex, -1, "For root signatures and shader code that wants registers declared. Calculated before backend code is called, for convenience of backends.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(int, constantBufferStructIndex, -1, "for CBVs, this is the index in renderGraph.structs that describes the constant buffer", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(ShaderConstantBuffer, "A reference to a struct")
    STRUCT_FIELD(std::string, resourceName, "", "The name of the resource in the shader", 0)
    STRUCT_FIELD(std::string, structName, "", "The name of the struct used", 0)

    STRUCT_FIELD(int, structIndex, -1, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(ShaderDefine, "A shader define as part of shader compilation")
    STRUCT_FIELD(std::string, name, "", "The name of the define.", 0)
    STRUCT_FIELD(std::string, value, "", "The value of the define.", 0)
STRUCT_END()

STRUCT_BEGIN(TokenReplacement, "A shader token replacement")
    STRUCT_FIELD(std::string, name, "", "The token string.", 0)
    STRUCT_FIELD(std::string, value, "", "The replacement.", 0)
STRUCT_END()

STRUCT_BEGIN(LoadedTextureReference, "Information about a loaded texture referenced by this shader.")
    STRUCT_FIELD(std::string, token, "", "The token as it appears in the shader.", 0)
    STRUCT_FIELD(std::string, resourceName, "", "The name of the resource to replace it with.", 0)
STRUCT_END()

//========================================================
// A Hit Group
//========================================================

STRUCT_BEGIN(RTHitGroup, "A declaration of a ray tracing hit group, which may contain a closest hit, any hit, and intersection shader")
    STRUCT_FIELD(std::string, name, "", "The unique name of the hit group", 0)
    STRUCT_FIELD(RTClosestHitShaderReferenceOptional, closestHit, {}, "The closest hit shader", 0)
    STRUCT_FIELD(RTAnyHitShaderReferenceOptional, anyHit, {}, "The any hit shader", 0)
    STRUCT_FIELD(RTIntersectionShaderReferenceOptional, intersection, {}, "The intersection shader", 0)

    STRUCT_FIELD(std::string, originalName, "", "The name before renames and sanitization", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(std::string, scope, "", "The scope that the node lives in. A possibly nested list of subgraph node names, seperated by a dot.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

//========================================================
// Slang settings
//========================================================

STRUCT_BEGIN(SlangOptions, "A declaration of a shader")
    STRUCT_FIELD(bool, process, false, "if true, this shader will be processed by slang", 0)
STRUCT_END()

//========================================================
// A Shader
//========================================================

STRUCT_BEGIN(Shader, "A declaration of a shader")
    STRUCT_FIELD(std::string, name, "", "The name of the shader, as it will be referenced by nodes", 0)
    STRUCT_FIELD(std::string, fileName, "", "The file name of the shader file.", 0)
    STRUCT_FIELD(std::string, destFileName, "", "Filled out by compiler.  Where the shader file is supposed to go after compilation.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(ShaderType, type, ShaderType::Compute, "The type of shader it is", 0)

    STRUCT_FIELD(std::string, entryPoint, "", "The shader entrypoint.", 0)
    STRUCT_DYNAMIC_ARRAY(ShaderDefine, defines, "The defines the shader is compiled with.", SCHEMA_FLAG_UI_COLLAPSABLE | SCHEMA_FLAG_UI_ARRAY_FATITEMS)
    STRUCT_DYNAMIC_ARRAY(TokenReplacement, tokenReplacements, "The token replacements specific for the shader.", SCHEMA_FLAG_NO_SERIALIZE)

    // deprecated in 0.95b
    // replaced by NumThreads
    STRUCT_STATIC_ARRAY(int, CSNumThreads, 3, { 8 COMMA 8 COMMA 1 }, "For compute shaders only, the number of threads each dispatch has. 61,1,1 suggested for 1d. 8,8,1 for 2d. 4,4,4 for 3d.", SCHEMA_FLAG_NO_UI)

    STRUCT_STATIC_ARRAY(int, NumThreads, 3, { 8 COMMA 8 COMMA 1 }, "The number of threads each dispatch has, for applicable shader types. 64,1,1 suggested for 1d. 8,8,1 for 2d. 4,4,4 for 3d.", SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX)

    STRUCT_FIELD(bool, copyFile, true, "if false, will not copy the file over. A hackaround for when you have multiple raytracing shaders in the same file. TODO: resolve this better.", 0)
    STRUCT_FIELD(SlangOptions, slangOptions, {}, "Settings for optionally processing shaders with slang", 0)

    STRUCT_FIELD(BackendRestriction, backends, {}, "The backends this file copy happens for.", SCHEMA_FLAG_UI_COLLAPSABLE)

    STRUCT_DYNAMIC_ARRAY(ShaderResource, resources, "SRVs, UAVs etc. that the shader wants to access. Generates code into the shader for their declarations.", SCHEMA_FLAG_UI_COLLAPSABLE | SCHEMA_FLAG_UI_ARRAY_FATITEMS)
    STRUCT_DYNAMIC_ARRAY(ShaderSampler, samplers, "SRVs, UAVs etc. that the shader wants to access. Generates code into the shader for their declarations.", SCHEMA_FLAG_UI_COLLAPSABLE | SCHEMA_FLAG_UI_ARRAY_FATITEMS)
    STRUCT_DYNAMIC_ARRAY(ShaderConstantBuffer, constantBuffers, "A list of constant buffers this shader wants.", SCHEMA_FLAG_NO_UI) // no UI since it's basically always automatic

    STRUCT_FIELD(std::vector<LoadedTextureReference>, loadedTextureRefs, {}, "The list of loaded textures this shader references.", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(std::wstring, entryPointW, L"", "The wide string version of the entry point. Made for convinience.", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(std::string, originalName, "", "The name before renames and sanitization", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(std::string, scope, "", "The scope that the node lives in. A possibly nested list of subgraph node names, seperated by a dot.", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_DYNAMIC_ARRAY(std::string, Used_RTHitGroupIndex, "All RTHitGroupIndex names used in the shader", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_DYNAMIC_ARRAY(std::string, Used_RTMissIndex, "All RTMissIndex names used in the shader", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()
