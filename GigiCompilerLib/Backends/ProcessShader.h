///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "Schemas/Types.h"

struct Shader;
struct RenderGraph;
struct Variable;
struct ShaderSampler;
struct ShaderResource;
struct RenderGraphNode;

enum class ShaderLanguage;

struct ProcessShaderOptions_HLSL
{
	ProcessShaderOptions_HLSL(const Shader& shader)
		: m_shader(shader)
	{

	}
	Shader m_shader;

	std::string m_rayTraceFnName = "TraceRay";

	std::string m_topOfShader;

	bool m_writeOriginalLineNumbers = true;

	std::unordered_set<std::string> m_attribStructsUsed;

	// How to write variable references
	static void WriteVariableReference_InStruct(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const Variable& variable);
	static void WriteVariableReference_NotInStruct(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const Variable& variable);
	void (*m_writeVariableReference)(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const Variable& variable) = WriteVariableReference_InStruct;

    // How to write variable aliases
    static void WriteVariableAlias_InStruct(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const ShaderVariableAliasDeclaration& variable);
    static void WriteVariableAlias_NotInStruct(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const ShaderVariableAliasDeclaration& variable);
    void (*m_writeVariableAlias)(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const ShaderVariableAliasDeclaration& variable) = WriteVariableAlias_InStruct;

	// How to write samplers
	static void WriteSamplerDefinition_Register(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const ShaderSampler& sampler);
	static void WriteSamplerDefinition_NoRegister(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const ShaderSampler& sampler);
	void (*m_writeSamplerDefinition)(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const ShaderSampler& sampler) = WriteSamplerDefinition_Register;

	// How to write resource references
	static void WriteResourceDefinition(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const RenderGraph& renderGraph, const ShaderResource& resource, bool writeRegister);
	inline static void WriteResourceDefinition_Register(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const RenderGraph& renderGraph, const ShaderResource& resource) { WriteResourceDefinition(options, stream, renderGraph, resource, true); }
	inline static void WriteResourceDefinition_NoRegister(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const RenderGraph& renderGraph, const ShaderResource& resource) { WriteResourceDefinition(options, stream, renderGraph, resource, false); }
	void (*m_writeResourceDefinition)(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const RenderGraph& renderGraph, const ShaderResource& resource) = WriteResourceDefinition_Register;

	// How to write a constant buffer resource
	static void WriteConstantBufferDefinition_NotInStruct(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const RenderGraph& renderGraph, const ShaderResource& resource);
	static void WriteConstantBufferDefinition_CBuffer(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const RenderGraph& renderGraph, const ShaderResource& resource);
	static void WriteConstantBufferDefinition_ConstantBuffer(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const RenderGraph& renderGraph, const ShaderResource& resource);
	void (*m_writeConstantBufferDefinition)(const ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const RenderGraph& renderGraph, const ShaderResource& resource) = WriteConstantBufferDefinition_ConstantBuffer;

	// How to handle shader tokens
	static bool HandleShaderToken_Noop(ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const std::string& token, size_t lineNumber, const RenderGraph& renderGraph, const std::string& declareLineNumber) { return false; }
	bool (*m_handleShaderToken)(ProcessShaderOptions_HLSL& options, std::ostringstream& stream, const std::string& token, size_t lineNumber, const RenderGraph& renderGraph, const std::string& declareLineNumber) = HandleShaderToken_Noop;

	// Any free form string replacement
	using THandleGeneralStringReplacement = std::function<void(std::string& shaderCode, std::unordered_map<std::string, std::ostringstream>& shaderSpecificStringReplacementMap, const ProcessShaderOptions_HLSL& options, const RenderGraph& renderGraph)>;
	static void HandleGeneralStringReplacement_Noop(std::string& shaderCode, std::unordered_map<std::string, std::ostringstream>& shaderSpecificStringReplacementMap, const ProcessShaderOptions_HLSL& options, const RenderGraph& renderGraph) { }
	THandleGeneralStringReplacement m_handleGeneralStringReplacement = HandleGeneralStringReplacement_Noop;
};

bool ProcessShader_HLSL(const Shader& shader, const char* entryPoint, ShaderLanguage targetShaderLanguage, const std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, const RenderGraph& renderGraph, ProcessShaderOptions_HLSL& options, const char* outFileName);
bool ProcessShaderToMemory_HLSL(const Shader& shader, const char* entryPoint, ShaderLanguage targetShaderLanguage, const std::unordered_map<std::string, std::ostringstream>& stringReplacementMap, const RenderGraph& renderGraph, ProcessShaderOptions_HLSL& options, const std::vector<std::string>& includeDirectories, std::string& shaderCode);

void ProcessShader_ReplaceSingleCharacterConstants(std::string& shaderCode);
