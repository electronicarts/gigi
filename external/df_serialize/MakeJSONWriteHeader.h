// Generates code to write data to json files.

#include "_common.h"

enum class JSONWriteOverrideResult
{
    Finished,
    Continue
};

// Enums

#define ENUM_BEGIN(_NAME, _DESCRIPTION) \
    inline rapidjson::Value MakeJSONValue(const _NAME& value, rapidjson::Document::AllocatorType& allocator) \
    { \
        { \
            rapidjson::Value ret; \
            JSONWriteOverrideResult overrideResult = JSONWriteOverride(value, allocator, ret); \
            if (overrideResult == JSONWriteOverrideResult::Finished) \
                return ret; \
        } \
        typedef _NAME EnumType; \
        switch(value) \
        {

#define ENUM_ITEM(_NAME, _DESCRIPTION) \
            case EnumType::_NAME: return MakeJSONValue(TSTRING(#_NAME), allocator); break;

#define ENUM_END() \
            default: return MakeJSONValue(TSTRING("<Unknown>"), allocator); break; \
        } \
    }

// Structs

#define STRUCT_BEGIN(_NAME, _DESCRIPTION) \
    inline rapidjson::Value MakeJSONValue(const _NAME& value, rapidjson::Document::AllocatorType& allocator) \
    { \
        rapidjson::Value ret; \
        ret.SetObject(); \
        { \
            JSONWriteOverrideResult overrideResult = JSONWriteOverride(value, allocator, ret); \
            if (overrideResult == JSONWriteOverrideResult::Finished) \
                return ret; \
        }

#define STRUCT_INHERIT_BEGIN(_NAME, _BASE, _DESCRIPTION) \
    inline rapidjson::Value MakeJSONValue(const _NAME& value, rapidjson::Document::AllocatorType& allocator) \
    { \
        rapidjson::Value ret; \
        ret.SetObject(); \
        { \
            JSONWriteOverrideResult overrideResult = JSONWriteOverride(value, allocator, ret); \
            if (overrideResult == JSONWriteOverrideResult::Finished) \
                return ret; \
        } \
        rapidjson::Value inheritedValues = MakeJSONValue(*(const _BASE*)&value, allocator); \
        for (auto member = inheritedValues.MemberBegin(); member != inheritedValues.MemberEnd(); ++member) \
            ret.AddMember(member->name, member->value, allocator);

#define STRUCT_FIELD(_TYPE, _NAME, _DEFAULT, _DESCRIPTION, _FLAGS) \
        if (((_FLAGS) & SCHEMA_FLAG_NO_SERIALIZE) == 0) \
        { \
            static const _TYPE c_defaultValue = _DEFAULT; \
            if (value._NAME != c_defaultValue) \
                ret.AddMember(#_NAME, MakeJSONValue(value._NAME, allocator), allocator); \
        }

#define STRUCT_CONST(_TYPE, _NAME, _DEFAULT, _DESCRIPTION, _FLAGS)

#define STRUCT_DYNAMIC_ARRAY(_TYPE, _NAME, _DESCRIPTION, _FLAGS) \
        if (((_FLAGS) & SCHEMA_FLAG_NO_SERIALIZE) == 0 && TDYNAMICARRAY_SIZE(value._NAME) > 0) \
        { \
            rapidjson::Value arr; \
            arr.SetArray(); \
            for (size_t index = 0; index < TDYNAMICARRAY_SIZE(value._NAME); ++index) \
                arr.PushBack(MakeJSONValue(value._NAME[index], allocator), allocator); \
            ret.AddMember(#_NAME, arr, allocator); \
        }

#define STRUCT_STATIC_ARRAY(_TYPE, _NAME, _SIZE, _DEFAULT, _DESCRIPTION, _FLAGS) \
        if (((_FLAGS) & SCHEMA_FLAG_NO_SERIALIZE) == 0) \
        { \
            static const TSTATICARRAY<_TYPE, _SIZE> c_default = _DEFAULT; \
            bool different = false; \
            for (int i = 0; i < _SIZE; ++i) \
            { \
                if (value._NAME[i] != c_default[i]) \
                { \
                    different = true; \
                    break; \
                } \
            } \
            if (different) \
            { \
                rapidjson::Value arr; \
                arr.SetArray(); \
                for (size_t index = 0; index < _SIZE; ++index) \
                    arr.PushBack(MakeJSONValue(value._NAME[index], allocator), allocator); \
                ret.AddMember(#_NAME, arr, allocator); \
            } \
        }

#define STRUCT_END() \
        return ret; \
    }

// Variants

#define VARIANT_BEGIN(_NAME, _DESCRIPTION) \
    inline rapidjson::Value MakeJSONValue(const _NAME& value, rapidjson::Document::AllocatorType& allocator) \
    { \
        typedef _NAME ThisType; \
        rapidjson::Value ret; \
        ret.SetObject(); \
        { \
            JSONWriteOverrideResult overrideResult = JSONWriteOverride(value, allocator, ret); \
            if (overrideResult == JSONWriteOverrideResult::Finished) \
                return ret; \
        }

// NOTE: always save out the variant object, so we know what _index is!
#define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
        if (value._index == ThisType::c_index_##_NAME /*&& value._NAME != _DEFAULT*/) \
            ret.AddMember(#_NAME, MakeJSONValue(value._NAME, allocator), allocator);

#define VARIANT_END() \
        return ret; \
    }

// A catch all template type to make compile errors about unsupported types easier to understand

template <typename T>
rapidjson::Value MakeJSONValue(const T& value, rapidjson::Document::AllocatorType& allocator)
{
    // an update on 3/31/22 makes vs2022 makes this not able to be a static assert ):
    Assert(false, __FUNCSIG__ ": Unsupported type encountered!");

    rapidjson::Value ret;
    ret.SetBool(false);
    return ret;
}

// Specialize this function to override the automatic serialization.
template <typename T>
JSONWriteOverrideResult JSONWriteOverride(T& value, rapidjson::Document::AllocatorType& allocator, rapidjson::Value& ret)
{
    return JSONWriteOverrideResult::Continue;
}

// Built in types

inline rapidjson::Value MakeJSONValue(const uint8_t& value, rapidjson::Document::AllocatorType& allocator)
{
    rapidjson::Value ret;
    ret.SetUint(value);
    return ret;
}

inline rapidjson::Value MakeJSONValue(const uint16_t& value, rapidjson::Document::AllocatorType& allocator)
{
    rapidjson::Value ret;
    ret.SetUint(value);
    return ret;
}

inline rapidjson::Value MakeJSONValue(const uint32_t& value, rapidjson::Document::AllocatorType& allocator)
{
    rapidjson::Value ret;
    ret.SetUint(value);
    return ret;
}

inline rapidjson::Value MakeJSONValue(const uint64_t& value, rapidjson::Document::AllocatorType& allocator)
{
    rapidjson::Value ret;
    ret.SetUint64(value);
    return ret;
}

inline rapidjson::Value MakeJSONValue(const int8_t& value, rapidjson::Document::AllocatorType& allocator)
{
    rapidjson::Value ret;
    ret.SetInt(value);
    return ret;
}

inline rapidjson::Value MakeJSONValue(const int16_t& value, rapidjson::Document::AllocatorType& allocator)
{
    rapidjson::Value ret;
    ret.SetInt(value);
    return ret;
}

inline rapidjson::Value MakeJSONValue(const int32_t& value, rapidjson::Document::AllocatorType& allocator)
{
    rapidjson::Value ret;
    ret.SetInt(value);
    return ret;
}

inline rapidjson::Value MakeJSONValue(const int64_t& value, rapidjson::Document::AllocatorType& allocator)
{
    rapidjson::Value ret;
    ret.SetInt64(value);
    return ret;
}

inline rapidjson::Value MakeJSONValue(const float& value, rapidjson::Document::AllocatorType& allocator)
{
    rapidjson::Value ret;
    ret.SetDouble(value);
    return ret;
}

inline rapidjson::Value MakeJSONValue(const bool& value, rapidjson::Document::AllocatorType& allocator)
{
    rapidjson::Value ret;
    ret.SetBool(value);
    return ret;
}

inline rapidjson::Value MakeJSONValue(const TSTRING& value, rapidjson::Document::AllocatorType& allocator)
{
    rapidjson::Value ret;
    ret.SetString(&value[0], allocator);
    return ret;
}

inline JSONWriteOverrideResult JSONWriteOverride(const RenderGraph& value, rapidjson::Document::AllocatorType& allocator, rapidjson::Value& ret)
{
    if (value.schema.empty())
        ret.AddMember("$schema", MakeJSONValue(std::string("gigischema.json"), allocator), allocator);
    else
        ret.AddMember("$schema", MakeJSONValue(value.schema, allocator), allocator);
    return JSONWriteOverrideResult::Continue;
}
