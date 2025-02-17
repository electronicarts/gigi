///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

// clang-format off
#include "SchemasCommon.h"
#include "SchemasShaders.h"
#include "SchemasVariables.h"
#include "RenderGraphNodes.h"
#include "PreviewWindow/PreviewWindowSchemas.h"
#include "Browser/BrowserSchemas.h"
#include "Misc/BackendTemplateSchemas.h"
// clang-format on

ENUM_BEGIN(GigiCompileResult, "")
    ENUM_ITEM(OK, "")
    ENUM_ITEM(WrongVersion, "")
    ENUM_ITEM(WrongParams, "")
    ENUM_ITEM(CantLoadRenderGraph, "")
    ENUM_ITEM(ShaderAsserts, "")
    ENUM_ITEM(ShaderReflection, "")
    ENUM_ITEM(Validation, "")
    ENUM_ITEM(ReferenceFixup, "")
    ENUM_ITEM(DepluralizeFileCopies, "")
    ENUM_ITEM(NoBackend, "")
    ENUM_ITEM(BackendData, "")
    ENUM_ITEM(Sanitize, "")
    ENUM_ITEM(NotCompiledYet, "")
    ENUM_ITEM(InterpreterError, "")
    ENUM_ITEM(InlineSubGraphs, "")
    ENUM_ITEM(ErrorCheck, "")
    ENUM_ITEM(ShaderFileDuplication, "")
    ENUM_ITEM(AddNodeInfoToShaders, "")
    ENUM_ITEM(DataFixup, "")
    ENUM_ITEM(DfltFixup, "")
    ENUM_ITEM(HandleOutputsToMultiInput, "")
ENUM_END()

ENUM_BEGIN(GigiCompileWarning, "Gigi compilation warnings")
    ENUM_ITEM(ShaderUnusedResource, "A declared resource does not appear to be used in a shader. This can lead to additional transitions and unnecessary ordering constraints.")
    ENUM_ITEM(Count, "")
ENUM_END()

ENUM_BEGIN(DXShaderCompiler, "Which directx shader compiler to use")
    ENUM_ITEM(FXC, "The old shader compilation path.")
    ENUM_ITEM(DXC, "The newer shader compilation path, required for raytracing.")
ENUM_END()

STRUCT_BEGIN(BackendSettings_DX12, "DX12 Settings")
    STRUCT_FIELD(int, numSRVDescriptors, 256, "The number of descriptors in the internal SRV heap", 0)
    STRUCT_FIELD(int, numRTVDescriptors, 256, "The number of descriptors in the internal RTV heap", 0)
    STRUCT_FIELD(int, numDSVDescriptors, 256, "The number of descriptors in the internal DSV heap", 0)
    STRUCT_FIELD(DXShaderCompiler, shaderCompiler, DXShaderCompiler::DXC, "The shader compiler to use", 0)
    STRUCT_FIELD(std::string, shaderModelCs, "cs_6_1", "The default shader model to use for compute shaders", 0)
    STRUCT_FIELD(std::string, shaderModelVs, "vs_6_1", "The default shader model to use for vertex shaders", 0)
    STRUCT_FIELD(std::string, shaderModelPs, "ps_6_1", "The default shader model to use for pixel shaders", 0)
    STRUCT_FIELD(std::string, shaderModelRayShaders, "lib_6_3", "The default shader model to use for ray shaders", 0)
    STRUCT_FIELD(std::string, shaderModelAs, "as_6_5", "The default shader model to use for amplification shaders", 0)
    STRUCT_FIELD(std::string, shaderModelMs, "ms_6_5", "The default shader model to use for mesh shaders", 0)
    STRUCT_FIELD(bool, DXC_HLSL_2021, false, "When using DXC, use HLSL 2021.  https://github.com/microsoft/DirectXShaderCompiler/wiki/HLSL-2021", 0)
    STRUCT_FIELD(bool, AgilitySDKRequired, false, "True if the agility SDK is required in DX12. Can be set to true in the editor, but can also be set to true by the compiler.", 0)
STRUCT_END()


STRUCT_BEGIN(BackendSettings_Common, "Common Settings")
    STRUCT_FIELD(bool, debugNames, true, "If true, sets debug names to GPU objects on available platforms.", 0)
    STRUCT_FIELD(bool, debugShaders, true, "If true, compiles shaders with debug options turned on, on available platforms.", 0)
STRUCT_END()

STRUCT_BEGIN(BackendSettings, "Backend settings")
    STRUCT_FIELD(BackendSettings_DX12, dx12, {}, "", SCHEMA_FLAG_UI_COLLAPSABLE)
    STRUCT_FIELD(BackendSettings_Common, common, {}, "", SCHEMA_FLAG_UI_COLLAPSABLE)
STRUCT_END()

STRUCT_BEGIN(BuildSettings, "Backend settings")
    STRUCT_DYNAMIC_ARRAY(GigiCompileWarning, disableWarnings, "Warnings listed here will be suppressed", 0)

    // Only used by editor
    STRUCT_FIELD(std::string, outDX12, "out/dx12/", "The output location for DX12", 0)

    STRUCT_FIELD(std::string, outInterpreter, "out/interpreter/", "The output location for the interpreter backend", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()

STRUCT_BEGIN(ConfigFromBackend, "Frontend configuration set by the backend.")
    STRUCT_FIELD(bool, RTSceneTakesSRVSlot, true, "If true, RT scenes will take an SRV register slot.", 0)
STRUCT_END()

STRUCT_BEGIN(ResourceTransition, "A single resource transition")
    STRUCT_FIELD(int, nodeIndex, -1, "The node for the resource being transitioned.", 0)
    STRUCT_FIELD(ShaderResourceAccessType, oldState, ShaderResourceAccessType::Count, "The previous state", 0)
    STRUCT_FIELD(ShaderResourceAccessType, newState, ShaderResourceAccessType::Count, "The next state", 0)
STRUCT_END()

STRUCT_BEGIN(ResourceTransitions, "A list of resource transitions")
    STRUCT_DYNAMIC_ARRAY(ResourceTransition, transitions, "A list of resource transitions", 0)
STRUCT_END()

ENUM_BEGIN(FileCopyType, "")
    ENUM_ITEM(Private, "Provided as input by the host application")
    ENUM_ITEM(Shader, "Used internally to the technique only")
    ENUM_ITEM(Asset, "An asset used by the technique")
    ENUM_ITEM(Count, "")
ENUM_END()

STRUCT_BEGIN(FileCopy, "A description of a file to copy into the output package")
    STRUCT_FIELD(std::string, fileName, "", "The file to copy.", 0)
    STRUCT_FIELD(FileCopyType, type, FileCopyType::Private, "The type of file it is", 0)
    STRUCT_FIELD(std::string, destFileName, "", "If empty, uses fileName", 0)
    STRUCT_FIELD(BackendRestriction, backends, {}, "The backends this file copy happens for.", SCHEMA_FLAG_UI_COLLAPSABLE)
    STRUCT_FIELD(bool, binary, false, "If false, it will be treated as a text file, undergo string replacement, and line ending normalization. If true, it will be copied without modification.", 0)
    STRUCT_FIELD(bool, plural, false, "If true, this file copy is a file pattern for possibly several files to copy.  The filename should contain a %i and all files from 0 to N at that location in the file name will be copied.", 0)
STRUCT_END()

ENUM_BEGIN(SetVariableOperator, "")
    ENUM_ITEM(Add, "+")
    ENUM_ITEM(Subtract, "-")
    ENUM_ITEM(Multiply, "*")
    ENUM_ITEM(Divide, "/")
    ENUM_ITEM(Modulo, "%")

    ENUM_ITEM(PowerOf2GE, "The next power of two, greater or equal to the current value")

    ENUM_ITEM(Minimum, "min(A,B)")
    ENUM_ITEM(Maximum, "max(A,B)")

    ENUM_ITEM(BitwiseOr, "A | B")
    ENUM_ITEM(BitwiseAnd, "A & B")
    ENUM_ITEM(BitwiseXor, "A ^ B")
    ENUM_ITEM(BitwiseNot, "~A")

    ENUM_ITEM(Noop, "Dont do anything, returns the left value")
ENUM_END()

STRUCT_BEGIN(SetVariable, "A variable modification")
    STRUCT_FIELD(VariableReferenceNoConst, destination, {}, "The variable to set.  destination = A (operator) B.", 0)
    STRUCT_FIELD(int, destinationIndex, -1, "The index of the field in the destination variable. -1 means no index.", 0)

    STRUCT_FIELD(VariableReference, AVar, {}, "The variable on the left side of the operator", 0)
    STRUCT_FIELD(int, AVarIndex, -1, "The index of the field in the A variable. -1 means no index.", 0)
    STRUCT_FIELD(TextureOrBufferNodeReference, ANode, {}, "If specified, the size of this resource will be used. AVarIndex will index into it if multi dimensional.", 0)

    STRUCT_FIELD(std::string, ALiteral, "", "The value to use, if no variable is set", 0)

    STRUCT_FIELD(SetVariableOperator, op, SetVariableOperator::Add, "", 0)

    STRUCT_FIELD(VariableReference, BVar, {}, "The variable on the right side of the operator", 0)
    STRUCT_FIELD(int, BVarIndex, -1, "The index of the field in the B variable. -1 means no index.", 0)
    STRUCT_FIELD(TextureOrBufferNodeReference, BNode, {}, "If specified, the size of this resource will be used. BVarIndex will index into it if multi dimensional.", 0)

    STRUCT_FIELD(std::string, BLiteral, "", "The value to use, if no variable is set", 0)

    STRUCT_FIELD(bool, setBefore, true, "If true, this happens before execution. If false, happens after execution.", 0)

    STRUCT_FIELD(Condition, condition, {}, "An optional condition added for the action to happen", 0)
STRUCT_END()

STRUCT_BEGIN(VariableReplacement, "When subgraph variables are replaced by parent variables, this gives the breadcrumbs needed to find that.")
    STRUCT_FIELD(std::string, srcScope, "", "", 0)
    STRUCT_FIELD(std::string, srcName, "", "", 0)
    STRUCT_FIELD(std::string, destName, "", "", 0)
STRUCT_END()

STRUCT_BEGIN(CustomGigiToken, "Allows you to give values for custom gigi tokens, such as /*$(CopyrightHeader)*/. All unknown Gigi tokens are replaced with empty string by default.")
    STRUCT_FIELD(std::string, key, "", "The name of the gigi token without markup, such as CopyrightHeader for /*$(CopyrightHeader)*/", 0)
    STRUCT_FIELD(std::string, value, "", "The value to replace the token with", SCHEMA_FLAG_UI_MULTILINETEXT)
STRUCT_END()

STRUCT_BEGIN(EditorGroupNode, "Data for group nodes in the editor")
    STRUCT_FIELD(std::string, name, "", "Name of the group", 0)
    STRUCT_FIELD(int32_t, id, 0, "Id of the group", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_STATIC_ARRAY(float, position, 2, { 0.f COMMA 0.f }, "Position of the group", 0)
    STRUCT_STATIC_ARRAY(float, size, 2, { 0.f COMMA 0.f }, "Size of the group", 0)
    STRUCT_STATIC_ARRAY(float, color, 4, { 1.f COMMA 1.f COMMA 1.f COMMA 0.25f }, "Color of the group", 0)
STRUCT_END()

STRUCT_BEGIN(RenderGraph, "The root type of the render graph")
    STRUCT_FIELD(std::string, name, "Unnamed", "The name of the render graph.", 0)
    STRUCT_FIELD(std::string, comment, "", "Put author information, links, etc here.", SCHEMA_FLAG_UI_MULTILINETEXT)
    STRUCT_FIELD(std::string, version, "", "The gigi version of the render graph.", SCHEMA_FLAG_NO_UI)
    STRUCT_FIELD(std::string, schema, "", "The schema file read from the file originally, so we can put it back as it was.", SCHEMA_FLAG_NO_UI | SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_DYNAMIC_ARRAY(Variable, variables, "The render graph variables. Used for getting parameters from the host app and user. Can be referenced by many things in the render graph such as texture size and shader constants.", SCHEMA_FLAG_NO_UI)
    STRUCT_DYNAMIC_ARRAY(Shader, shaders, "The shaders used by the render graph.", SCHEMA_FLAG_NO_UI)
    STRUCT_DYNAMIC_ARRAY(Struct, structs, "The structs used by the render graph.", SCHEMA_FLAG_NO_UI)
    STRUCT_DYNAMIC_ARRAY(FileCopy, fileCopies, "Files to copy during the building process.", SCHEMA_FLAG_NO_UI)
    STRUCT_DYNAMIC_ARRAY(RenderGraphNode, nodes, "The render graph nodes.", SCHEMA_FLAG_NO_UI)
    STRUCT_DYNAMIC_ARRAY(Enum, enums, "Enums", SCHEMA_FLAG_NO_UI)
    STRUCT_DYNAMIC_ARRAY(SetVariable, setVars, "Variables to modify at the beginning or end of the render graph", SCHEMA_FLAG_NO_UI)
    STRUCT_DYNAMIC_ARRAY(RTHitGroup, hitGroups, "Ray tracing hit group definitions", SCHEMA_FLAG_NO_UI)

    STRUCT_FIELD(BackendSettings, settings, {}, "Backend settings", SCHEMA_FLAG_UI_COLLAPSABLE)

    STRUCT_FIELD(BuildSettings, buildSettings, {}, "Build settings", SCHEMA_FLAG_UI_COLLAPSABLE)

    STRUCT_DYNAMIC_ARRAY(CustomGigiToken, customTokens, "Allows you to give values for custom gigi tokens, such as /*$(CopyrightHeader)*/. All unknown Gigi tokens are replaced with empty string by default.", 0)

    STRUCT_FIELD(TextureNodeReference, PrimaryOutput, {}, "A hint to anything that might be able to use this information, such as generated code or the viewer.", 0)

    STRUCT_DYNAMIC_ARRAY(EditorGroupNode, editorGroupNodes, "Editor group nodes", SCHEMA_FLAG_NO_UI)

    // Non serialized things below

    STRUCT_FIELD(std::string, baseDirectory, "", "The relative location of the render graph file.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(std::string, outputDirectory, "", "Where the render graph output should go (this field used by the compiler).", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(std::vector<int>, flattenedNodeList, {}, "The flattened list of nodes, in the order they should be executed in. Calculated before being given to back end code.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(std::vector<ResourceTransitions>, transitions, {}, "The resource transitions that want to happen before each node executes. Calculated before being given to back end code.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(Backend, backend, Backend::DX12, "The backend currently being ran", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(ConfigFromBackend, configFromBackend, {}, "Information communicated to the front end, by the back end.", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(bool, usesRaytracing, false, "True if this render graph uses ray tracing.", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(std::string, editorFileName, "", "The name and path of the file name in the editor", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_DYNAMIC_ARRAY(std::string, subGGGraphFileNames, "The list of absolute path sugraph gg files this render graph relies on. Useful for file watches from the viewer.", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_DYNAMIC_ARRAY(VariableReplacement, variableReplacements, "When subgraph variables are replaced by parent variables, this gives the breadcrumbs needed to find that.", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(bool, versionUpgraded, false, "Set to true if the loading code did a version upgrade. The editor uses this to mark the document as dirty.", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(std::string, versionUpgradedFrom, "", "The version it was upgraded from", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(std::string, versionUpgradedMessage, "", "Text to show about the version upgrade", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(BackendTemplateConfig, templateConfig, {}, "Code generation template config", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(bool, generateGraphVizFlag, false, "Set to true if the generating GraphViz. Should be set to true from a command line parameter", SCHEMA_FLAG_NO_SERIALIZE)

    STRUCT_FIELD(std::vector<std::string>, assertsFormatStrings, {}, "The unique formatting strings of the asserts messages", SCHEMA_FLAG_NO_SERIALIZE)
    STRUCT_FIELD(std::unordered_set<std::string>, firedAssertsIdentifiers, {}, "The identifiers of the fired asserts to ignore them later on", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()
