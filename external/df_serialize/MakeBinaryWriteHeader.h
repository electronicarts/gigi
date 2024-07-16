// Generates code to write binary data to memory and files

#include "_common.h"

// Enums

#define ENUM_BEGIN(_NAME, _DESCRIPTION) \
    void BinaryWrite(const _NAME& value, TDYNAMICARRAY<char>& output) \
    { \
        typedef _NAME EnumType; \
        switch(value) \
        { \

#define ENUM_ITEM(_NAME, _DESCRIPTION) \
            case EnumType::_NAME: BinaryWrite(TSTRING(#_NAME), output); break; \

#define ENUM_END() \
        } \
    }

// Structs

#define STRUCT_BEGIN(_NAME, _DESCRIPTION) \
    void BinaryWrite(const _NAME& value, TDYNAMICARRAY<char>& output) \
    { \

#define STRUCT_INHERIT_BEGIN(_NAME, _BASE, _DESCRIPTION) \
    void BinaryWrite(const _NAME& value, TDYNAMICARRAY<char>& output) \
    { \
        BinaryWrite(*(const _BASE*)&value, output);

#define STRUCT_FIELD(_TYPE, _NAME, _DEFAULT, _DESCRIPTION, _FLAGS) \
        if (((_FLAGS) & SCHEMA_FLAG_NO_SERIALIZE) == 0) \
            BinaryWrite(value._NAME, output);

#define STRUCT_CONST(_TYPE, _NAME, _DEFAULT, _DESCRIPTION, _FLAGS)

#define STRUCT_DYNAMIC_ARRAY(_TYPE, _NAME, _DESCRIPTION, _FLAGS)\
        if (((_FLAGS) & SCHEMA_FLAG_NO_SERIALIZE) == 0) \
        { \
            BinaryWrite((int32_t)TDYNAMICARRAY_SIZE(value._NAME), output); \
            for (size_t index = 0; index < TDYNAMICARRAY_SIZE(value._NAME); ++index) \
                BinaryWrite(value._NAME[index], output); \
        }

#define STRUCT_STATIC_ARRAY(_TYPE, _NAME, _SIZE, _DEFAULT, _DESCRIPTION, _FLAGS)\
        if (((_FLAGS) & SCHEMA_FLAG_NO_SERIALIZE) == 0) \
        { \
            for (size_t index = 0; index < _SIZE; ++index) \
                BinaryWrite(value._NAME[index], output); \
        }

#define STRUCT_END() \
    }

// Variants

#define VARIANT_BEGIN(_NAME, _DESCRIPTION) \
    void BinaryWrite(const _NAME& value, TDYNAMICARRAY<char>& output) \
    { \
        typedef _NAME ThisType; \
        BinaryWrite(value._index, output);

#define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
        if (value._index == ThisType::c_index_##_NAME) \
            BinaryWrite(value._NAME, output);

#define VARIANT_END() \
    }

// A catch all template type to make compile errors about unsupported types easier to understand

template <typename T>
bool BinaryWrite(const T& value, TDYNAMICARRAY<char>& output)
{
    static_assert(false, __FUNCSIG__ ": Unsupported type encountered!");
    return false;
}

// Built in types

void BinaryWrite(uint8_t value, TDYNAMICARRAY<char>& output)
{
    size_t offset = TDYNAMICARRAY_SIZE(output);
    TDYNAMICARRAY_RESIZE(output,offset + sizeof(value));
    *((decltype(&value))(&output[offset])) = value;
}

void BinaryWrite(uint16_t value, TDYNAMICARRAY<char>& output)
{
    size_t offset = TDYNAMICARRAY_SIZE(output);
    TDYNAMICARRAY_RESIZE(output, offset + sizeof(value));
    *((decltype(&value))(&output[offset])) = value;
}

void BinaryWrite(uint32_t value, TDYNAMICARRAY<char>& output)
{
    size_t offset = TDYNAMICARRAY_SIZE(output);
    TDYNAMICARRAY_RESIZE(output, offset + sizeof(value));
    *((decltype(&value))(&output[offset])) = value;
}

void BinaryWrite(uint64_t value, TDYNAMICARRAY<char>& output)
{
    size_t offset = TDYNAMICARRAY_SIZE(output);
    TDYNAMICARRAY_RESIZE(output, offset + sizeof(value));
    *((decltype(&value))(&output[offset])) = value;
}

void BinaryWrite(int8_t value, TDYNAMICARRAY<char>& output)
{
    size_t offset = TDYNAMICARRAY_SIZE(output);
    TDYNAMICARRAY_RESIZE(output, offset + sizeof(value));
    *((decltype(&value))(&output[offset])) = value;
}

void BinaryWrite(int16_t value, TDYNAMICARRAY<char>& output)
{
    size_t offset = TDYNAMICARRAY_SIZE(output);
    TDYNAMICARRAY_RESIZE(output, offset + sizeof(value));
    *((decltype(&value))(&output[offset])) = value;
}

void BinaryWrite(int32_t value, TDYNAMICARRAY<char>& output)
{
    size_t offset = TDYNAMICARRAY_SIZE(output);
    TDYNAMICARRAY_RESIZE(output, offset + sizeof(value));
    *((decltype(&value))(&output[offset])) = value;
}

void BinaryWrite(int64_t value, TDYNAMICARRAY<char>& output)
{
    size_t offset = TDYNAMICARRAY_SIZE(output);
    TDYNAMICARRAY_RESIZE(output, offset + sizeof(value));
    *((decltype(&value))(&output[offset])) = value;
}

void BinaryWrite(float value, TDYNAMICARRAY<char>& output)
{
    size_t offset = TDYNAMICARRAY_SIZE(output);
    TDYNAMICARRAY_RESIZE(output, offset + sizeof(value));
    *((float*)(&output[offset])) = value;
}

void BinaryWrite(bool value, TDYNAMICARRAY<char>& output)
{
    BinaryWrite(value ? 1 : 0, output);
}

void BinaryWrite(const TSTRING& value, TDYNAMICARRAY<char>& output)
{
    size_t offset = TDYNAMICARRAY_SIZE(output);
    int len = (int)strlen(&value[0]);
    TDYNAMICARRAY_RESIZE(output, TDYNAMICARRAY_SIZE(output) + len + 1);
    char* dest = (char*)&output[offset];
    const char* src = &value[0];
    for (int i = 0; i <= len; ++i)
        dest[i] = src[i];
}
