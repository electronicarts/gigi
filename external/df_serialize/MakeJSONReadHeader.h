// Generates code to read data from json files and strings.
// Code meant for use with rapidjson
// RapidJSON Website: https://rapidjson.org/
// RapidJSON Github:  https://github.com/Tencent/rapidjson/

#include "_common.h"

enum class JSONReadOverrideResult
{
    Error,
    Finished,
    Continue
};

// Enums

#define ENUM_BEGIN(_NAME, _DESCRIPTION) \
    template <typename DOCUMENT> \
    bool JSONRead(_NAME& value, DOCUMENT& document) \
    { \
        JSONReadOverrideResult overrideResult = JSONReadOverride(value, document); \
        if (overrideResult == JSONReadOverrideResult::Error) \
            return false; \
        else if (overrideResult == JSONReadOverrideResult::Finished) \
            return true; \
        typedef _NAME EnumType; \
        if (!document.IsString()) \
        { \
            DFS_LOG("Trying to read a " #_NAME " but it wasn't a string\n"); \
            return false; \
        } \
        const char* stringValue = document.GetString();

#define ENUM_ITEM(_NAME, _DESCRIPTION) \
        if (!_stricmp(stringValue, #_NAME)) \
        { \
            value = EnumType::_NAME; \
            return true; \
        }

#define ENUM_END() \
        DFS_LOG("Unknown Enum Value: \"%s\"", stringValue); \
        return false; \
    }

// Structs

#define STRUCT_BEGIN(_NAME, _DESCRIPTION) \
    template <typename DOCUMENT> \
    bool JSONRead(_NAME& value, DOCUMENT& document) \
    { \
        JSONReadOverrideResult overrideResult = JSONReadOverride(value, document); \
        if (overrideResult == JSONReadOverrideResult::Error) \
            return false; \
        else if (overrideResult == JSONReadOverrideResult::Finished) \
            return true; \
        if (!document.IsObject()) \
            return false;

#define STRUCT_INHERIT_BEGIN(_NAME, _BASE, _DESCRIPTION) \
    template <typename DOCUMENT> \
    bool JSONRead(_NAME& value, DOCUMENT& document) \
    { \
        JSONReadOverrideResult overrideResult = JSONReadOverride(value, document); \
        if (overrideResult == JSONReadOverrideResult::Error) \
            return false; \
        else if (overrideResult == JSONReadOverrideResult::Finished) \
            return true; \
        if (!document.IsObject()) \
            return false; \
        if (!JSONRead(*(_BASE*)&value, document)) \
            return false;

#define STRUCT_FIELD(_TYPE, _NAME, _DEFAULT, _DESCRIPTION, _FLAGS) \
        if (((_FLAGS) & SCHEMA_FLAG_NO_SERIALIZE) == 0 && document.HasMember(#_NAME) && !JSONRead(value.##_NAME, document[#_NAME])) \
        { \
            DFS_LOG("Could not read member " #_NAME "\n"); \
            return false; \
        }

#define STRUCT_CONST(_TYPE, _NAME, _DEFAULT, _DESCRIPTION, _FLAGS)

#define STRUCT_DYNAMIC_ARRAY(_TYPE, _NAME, _DESCRIPTION, _FLAGS) \
        if (((_FLAGS) & SCHEMA_FLAG_NO_SERIALIZE) == 0 && document.HasMember(#_NAME)) \
        { \
            if (!document[#_NAME].IsArray()) \
            { \
                DFS_LOG("'" #_NAME "' is not an array.\n"); \
                return false; \
            } \
            auto arr = document[#_NAME].GetArray(); \
            TDYNAMICARRAY_RESIZE(value._NAME, arr.Size()); \
            int index = 0; \
            for (const rapidjson::Value& item : arr) \
            { \
                if(!JSONRead(value._NAME[index], item)) \
                    return false; \
                index++; \
            } \
        }

#define STRUCT_STATIC_ARRAY(_TYPE, _NAME, _SIZE, _DEFAULT, _DESCRIPTION, _FLAGS) \
        if (((_FLAGS) & SCHEMA_FLAG_NO_SERIALIZE) == 0 && document.HasMember(#_NAME)) \
        { \
            if (!document[#_NAME].IsArray()) \
            { \
                DFS_LOG("'" #_NAME "' is not an array.\n"); \
                return false; \
            } \
            auto arr = document[#_NAME].GetArray(); \
            if (arr.Size() != _SIZE) \
            { \
                DFS_LOG("'" #_NAME "' array was not the correct size.\n"); \
                return false; \
            } \
            int index = 0; \
            for (const rapidjson::Value& item : arr) \
            { \
                if(!JSONRead(value._NAME[index], item)) \
                    return false; \
                index++; \
            } \
        }

#define STRUCT_END() \
        return true; \
    }

// Variants

#define VARIANT_BEGIN(_NAME, _DESCRIPTION) \
    template <typename DOCUMENT> \
    bool JSONRead(_NAME& value, DOCUMENT& document) \
    { \
        JSONReadOverrideResult overrideResult = JSONReadOverride(value, document); \
        if (overrideResult == JSONReadOverrideResult::Error) \
            return false; \
        else if (overrideResult == JSONReadOverrideResult::Finished) \
            return true; \
        typedef _NAME ThisType; \
        if (!document.IsObject()) \
            return false;

#define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
        if (document.HasMember(#_NAME)) \
        { \
            value._index = ThisType::c_index_##_NAME; \
            if (!JSONRead(value._NAME, document[#_NAME])) \
                return false; \
        }

#define VARIANT_END() \
        return true; \
    }

// A catch all template type to make compile errors about unsupported types easier to understand

template <typename T, typename U>
bool JSONRead(T& value, U& document)
{
    // an update on 3/31/22 makes vs2022 makes this not able to be a static assert ):
    Assert(false, __FUNCSIG__ ": Unsupported type encountered!");
    return false;
}

// Specialize this function to override the automatic serialization.
template <typename T, typename U>
JSONReadOverrideResult JSONReadOverride(T& value, U& document)
{
    return JSONReadOverrideResult::Continue;
}

// Built in types

template <typename T>
bool JSONRead(uint8_t& value, T& document)
{
    if (!document.IsInt())
    {
        DFS_LOG("Trying to read a uint8 but it wasn't an int\n");
        return false;
    }

    value = document.GetInt();
    return true;
}

template <typename T>
bool JSONRead(uint16_t& value, T& document)
{
    if (!document.IsInt())
    {
        DFS_LOG("Trying to read a uint16 but it wasn't an int\n");
        return false;
    }

    value = document.GetInt();
    return true;
}

template <typename T>
bool JSONRead(uint32_t& value, T& document)
{
    if (!document.IsInt())
    {
        DFS_LOG("Trying to read a uint32 but it wasn't an int\n");
        return false;
    }

    value = document.GetInt();
    return true;
}

template <typename T>
bool JSONRead(uint64_t& value, T& document)
{
    if (!document.IsInt64())
    {
        DFS_LOG("Trying to read a uint64 but it wasn't an int64\n");
        return false;
    }

    value = document.GetInt64();
    return true;
}

template <typename T>
bool JSONRead(int8_t& value, T& document)
{
    if (!document.IsInt())
    {
        DFS_LOG("Trying to read an int8 but it wasn't an int\n");
        return false;
    }

    value = document.GetInt();
    return true;
}

template <typename T>
bool JSONRead(int16_t& value, T& document)
{
    if (!document.IsInt())
    {
        DFS_LOG("Trying to read an int16 but it wasn't an int\n");
        return false;
    }

    value = document.GetInt();
    return true;
}

template <typename T>
bool JSONRead(int32_t& value, T& document)
{
    if (!document.IsInt())
    {
        DFS_LOG("Trying to read an int32 but it wasn't an int\n");
        return false;
    }

    value = document.GetInt();
    return true;
}

template <typename T>
bool JSONRead(int64_t& value, T& document)
{
    if (!document.IsInt64())
    {
        DFS_LOG("Trying to read an int64 but it wasn't an int64\n");
        return false;
    }

    value = document.GetInt64();
    return true;
}

template <typename T>
bool JSONRead(float& value, T& document)
{
    if (!document.IsDouble())
    {
        int intValue = 0;
        if (JSONRead(intValue, document))
        {
            value = (float)intValue;
            return true;
        }

        DFS_LOG("Trying to read a float but it wasn't a float\n");
        return false;
    }

    value = (float)document.GetDouble();
    return true;
}

template <typename T>
bool JSONRead(bool& value, T& document)
{
    if (!document.IsBool())
    {
        DFS_LOG("Trying to read a bool but it wasn't a bool\n");
        return false;
    }

    value = document.GetBool();
    return true;
}

template <typename T>
bool JSONRead(TSTRING& value, T& document)
{
    if (!document.IsString())
    {
        DFS_LOG("Trying to read a string but it wasn't a string\n");
        return false;
    }

    value = document.GetString();
    return true;
}
