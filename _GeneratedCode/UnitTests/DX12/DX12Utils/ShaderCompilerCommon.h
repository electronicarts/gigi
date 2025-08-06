#pragma once

#include <filesystem>
#include <vector>

#define SETUP_ENUM_CLASS_OPERATORS(enumType) \
	inline           enumType& operator|=(enumType& Lhs, enumType Rhs) { return Lhs = (enumType)((std::underlying_type_t<enumType>)Lhs | (std::underlying_type_t<enumType>)Rhs); } \
	inline           enumType& operator&=(enumType& Lhs, enumType Rhs) { return Lhs = (enumType)((std::underlying_type_t<enumType>)Lhs & (std::underlying_type_t<enumType>)Rhs); } \
	inline           enumType& operator^=(enumType& Lhs, enumType Rhs) { return Lhs = (enumType)((std::underlying_type_t<enumType>)Lhs ^ (std::underlying_type_t<enumType>)Rhs); } \
	inline constexpr enumType  operator| (enumType  Lhs, enumType Rhs) { return (enumType)((std::underlying_type_t<enumType>)Lhs | (std::underlying_type_t<enumType>)Rhs); } \
	inline constexpr enumType  operator& (enumType  Lhs, enumType Rhs) { return (enumType)((std::underlying_type_t<enumType>)Lhs & (std::underlying_type_t<enumType>)Rhs); } \
	inline constexpr enumType  operator^ (enumType  Lhs, enumType Rhs) { return (enumType)((std::underlying_type_t<enumType>)Lhs ^ (std::underlying_type_t<enumType>)Rhs); } \
	inline constexpr bool  operator! (enumType e)             { return !(std::underlying_type_t<enumType>)e; } \
	inline constexpr enumType  operator~ (enumType e)             { return (enumType)~(std::underlying_type_t<enumType>)e; }

enum class ShaderCompilationFlags : uint32_t
{
	None = 0,
	Debug = 1 << 0,
	WarningsAsErrors = 1 << 1,
	HLSL2021 = 1 << 2,
	CreatePDBsAndBinaries = 1 << 3
};

SETUP_ENUM_CLASS_OPERATORS(ShaderCompilationFlags)

struct ShaderDefine
{
	ShaderDefine(const std::string& inName, const std::string& inValue)
		: name(inName), value(inValue)
	{}

	std::string name;
	std::string value;
};

struct ShaderCompilationInfo
{
	std::filesystem::path fileName;
	std::string entryPoint;
	std::string shaderModel;
	std::string debugName;
	std::vector<ShaderDefine> defines;

	ShaderCompilationFlags flags = ShaderCompilationFlags::None;
};