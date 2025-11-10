///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// This creates a function "bool WriteJSONSchema(const char* fileName)"

// clang-format off
#include "Schemas/Types.h"
#include <stdio.h>
#include <sstream>
#include <string>
#include <typeinfo>
// clang-format on

namespace JSONSchemaMaker
{

// Basic types
inline const char* GetPodType(std::string* type)
{
    return "string";
}
inline const char* GetPodType(int* type)
{
    return "integer";
}
inline const char* GetPodType(float* type)
{
    return "number";
}
inline const char* GetPodType(unsigned int* type)
{
    return "integer";
}
inline const char* GetPodType(bool* type)
{
    return "boolean";
}
inline const char* GetPodType(unsigned char* type)
{
    return "integer";
}

// make enums be pod type string
#include "external/df_serialize/_common.h"
#define ENUM_BEGIN(_NAME, _DESCRIPTION) \
inline const char* GetPodType(_NAME* type) \
{ \
    return "string"; \
}
// clang-format off
#include "external/df_serialize/_fillunsetdefines.h"
#include "Schemas.h"
#include "external/df_serialize/_common.h"
// clang-format on

// Catch alls
template <typename T>
void WriteJSONSchema(std::ostringstream& out, std::string indent, T* dummy)
{
    const char* typeName = typeid(T).name();
    GigiAssert(false, "Unhandled type encountered (%s)", typeName);
}

template <typename T>
const char* GetPodType(T* type)
{
    return nullptr;
}

template <typename T>
void WriteEnumValues(std::ostringstream& out, std::string indent, T* dummy)\
{
}

// Enums

#define ENUM_BEGIN(_NAME, _DESCRIPTION) \
inline void WriteEnumValues(std::ostringstream& out, std::string indent, _NAME* dummy) \
{ \
    out << ",\n" << indent << "\"enum\": ["; \
    bool first = true;

#define ENUM_ITEM(_NAME, _DESCRIPTION) \
    if(first) \
        out << "\"" #_NAME "\""; \
    else \
        out << ", \"" #_NAME "\""; \
    first = false;

#define ENUM_END() \
    out << "]"; \
}

// Structs

#define STRUCT_BEGIN(_NAME, _DESCRIPTION) \
inline bool WriteJSONSchema(std::ostringstream& out, std::string indent, _NAME* dummy, bool writePropertiesLabel = true) \
{ \
    const char* podType = nullptr; \
    if(writePropertiesLabel) \
        out << "\n" << indent << "\"properties\": {"; \
    bool first = true;

#define STRUCT_INHERIT_BEGIN(_NAME, _BASE, _DESCRIPTION) \
inline bool WriteJSONSchema(std::ostringstream& out, std::string indent, _NAME* dummy, bool writePropertiesLabel = true) \
{ \
    const char* podType = nullptr; \
    if(writePropertiesLabel) \
        out << "\n" << indent << "\"properties\": {"; \
    bool first = WriteJSONSchema(out, indent, (_BASE*)nullptr, false);

#define STRUCT_FIELD(_TYPE, _NAME, _DEFAULT, _DESCRIPTION, _FLAGS) \
    if (((_FLAGS) & SCHEMA_FLAG_NO_SERIALIZE) == 0) \
    { \
        if (!first) \
            out << ","; \
        out << "\n" << indent << "  \"" #_NAME "\": {"; \
        out << "\n" << indent << "    \"description\": \"" _DESCRIPTION "\","; \
        podType = GetPodType((_TYPE*)nullptr); \
        if (podType) \
        { \
            out << "\n" << indent << "    \"type\": \"" << podType <<  "\""; \
            WriteEnumValues(out, indent + "    ", (_TYPE*)nullptr); \
        } \
        else \
        { \
            out << "\n" << indent << "    \"type\": \"object\","; \
            WriteJSONSchema(out, indent + "    ", (_TYPE*)nullptr); \
        } \
        out << "\n" << indent << "  }"; \
        first = false; \
    }

#define STRUCT_CONST(_TYPE, _NAME, _DEFAULT, _DESCRIPTION, _FLAGS)

#define STRUCT_DYNAMIC_ARRAY(_TYPE, _NAME, _DESCRIPTION, _FLAGS) \
    if (((_FLAGS) & SCHEMA_FLAG_NO_SERIALIZE) == 0) \
    { \
        if (!first) \
            out << ","; \
        out << "\n" << indent << "  \"" #_NAME "\": {"; \
        out << "\n" << indent << "    \"description\": \"" _DESCRIPTION "\","; \
        out << "\n" << indent << "    \"type\": \"array\","; \
        out << "\n" << indent << "    \"items\": {"; \
        podType = GetPodType((_TYPE*)nullptr); \
        if (podType) \
        { \
            out << "\n" << indent << "      \"type\": \"" << podType <<  "\""; \
            WriteEnumValues(out, indent + "    ", (_TYPE*)nullptr); \
        } \
        else \
        { \
            out << "\n" << indent << "      \"type\": \"object\","; \
            WriteJSONSchema(out, indent + "      ", (_TYPE*)nullptr); \
        } \
        out << "\n" << indent << "    }"; \
        out << "\n" << indent << "  }"; \
        first = false; \
    }

#define STRUCT_STATIC_ARRAY(_TYPE, _NAME, _SIZE, _DEFAULT, _DESCRIPTION, _FLAGS) \
    if (((_FLAGS) & SCHEMA_FLAG_NO_SERIALIZE) == 0) \
    { \
        if (!first) \
            out << ","; \
        out << "\n" << indent << "  \"" #_NAME "\": {"; \
        out << "\n" << indent << "    \"description\": \"" _DESCRIPTION "\","; \
        out << "\n" << indent << "    \"type\": \"array\","; \
        out << "\n" << indent << "    \"minItems\": " #_SIZE ","; \
        out << "\n" << indent << "    \"maxItems\": " #_SIZE ","; \
        out << "\n" << indent << "    \"items\": {"; \
        podType = GetPodType((_TYPE*)nullptr); \
        if (podType) \
            out << "\n" << indent << "      \"type\": \"" << podType <<  "\""; \
        else \
        { \
            out << "\n" << indent << "      \"type\": \"object\","; \
            WriteJSONSchema(out, indent + "      ", (_TYPE*)nullptr); \
        } \
        out << "\n" << indent << "    }"; \
        out << "\n" << indent << "  }"; \
        first = false; \
    }

#define STRUCT_END() \
    if(writePropertiesLabel) \
        out << "\n" << indent << "}"; \
    return first; \
}

// Variants

#define VARIANT_BEGIN(_NAME, _DESCRIPTION) \
inline void WriteJSONSchema(std::ostringstream& out, std::string indent, _NAME* dummy) \
{ \
    const char* podType = nullptr; \
    out << "\n" << indent << "\"properties\": {"; \
    bool first = true;

#define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
    if (!first) \
        out << ","; \
    out << "\n" << indent << "  \"" #_NAME "\": {"; \
    out << "\n" << indent << "    \"description\": \"" _DESCRIPTION "\","; \
    podType = GetPodType((_TYPE*)nullptr); \
    if (podType) \
    { \
        out << "\n" << indent << "    \"type\": \"" << podType <<  "\""; \
        WriteEnumValues(out, indent + "    ", (_TYPE*)nullptr); \
    } \
    else \
    { \
        out << "\n" << indent << "    \"type\": \"object\","; \
        WriteJSONSchema(out, indent + "    ", (_TYPE*)nullptr); \
    } \
    out << "\n" << indent << "  }"; \
    first = false;

#define VARIANT_END() \
    out << "\n" << indent << "}"; \
}

// Expand macros
// clang-format off
#include "external/df_serialize/_fillunsetdefines.h"
#include "Schemas.h"
// clang-format on

} // namespace JSONSchemaMaker

// Root function
inline bool WriteJSONSchema(const char* fileName)
{
    // make the output string
    std::ostringstream out;
    {
        out <<
            "{"
            "\n  \"$schema\": \"http://json-schema.org/draft-07/schema\","
            "\n  \"title\": \"" << fileName << "\","
            "\n  \"type\": \"object\",";

        JSONSchemaMaker::WriteJSONSchema(out, "  ", (RenderGraph*)nullptr);

        out << "\n}";
    }

    // write the string out into the file
    FILE* file = nullptr;
    fopen_s(&file, fileName, "w+b");
    if (!file)
        return false;
    fprintf(file, "%s", out.str().c_str());
    fclose(file);
    return true;
}
