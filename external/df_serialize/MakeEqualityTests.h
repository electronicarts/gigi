// This makes member wise equality testing

#pragma once

#include "_common.h"

// Enums - they already have a == operator built in

#define ENUM_BEGIN(_NAME, _DESCRIPTION)

#define ENUM_ITEM(_NAME, _DESCRIPTION)

#define ENUM_END()

// Structs

#define STRUCT_BEGIN(_NAME, _DESCRIPTION) \
    inline bool operator == (const _NAME& A, const _NAME& B); \
    inline bool operator != (const _NAME& A, const _NAME& B) \
    { \
        return !(A==B); \
    } \
    bool operator == (const _NAME& A, const _NAME& B) \
    {

#define STRUCT_INHERIT_BEGIN(_NAME, _BASE, _DESCRIPTION) \
    inline bool operator == (const _NAME& A, const _NAME& B); \
    inline bool operator != (const _NAME& A, const _NAME& B) \
    { \
        return !(A==B); \
    } \
    bool operator == (const _NAME& A, const _NAME& B) \
    { \
        if (*(const _BASE*)&A != *(const _BASE*)&B) \
            return false;

#define STRUCT_FIELD(_TYPE, _NAME, _DEFAULT, _DESCRIPTION, _FLAGS) \
        if (((_FLAGS) & SCHEMA_FLAG_NO_SERIALIZE) == 0 && A._NAME != B._NAME) \
            return false;

#define STRUCT_CONST(_TYPE, _NAME, _DEFAULT, _DESCRIPTION, _FLAGS)

#define STRUCT_DYNAMIC_ARRAY(_TYPE, _NAME, _DESCRIPTION, _FLAGS) \
        if (((_FLAGS) & SCHEMA_FLAG_NO_SERIALIZE) == 0) \
        { \
            if (TDYNAMICARRAY_SIZE(A._NAME) != TDYNAMICARRAY_SIZE(B._NAME)) \
                return false; \
            for (size_t index = 0; index < TDYNAMICARRAY_SIZE(A._NAME); ++index) \
            { \
                if (A._NAME[index] != B._NAME[index]) \
                    return false; \
            } \
        }

#define STRUCT_STATIC_ARRAY(_TYPE, _NAME, _SIZE, _DEFAULT, _DESCRIPTION, _FLAGS) \
        if (((_FLAGS) & SCHEMA_FLAG_NO_SERIALIZE) == 0) \
        { \
            for (size_t index = 0; index < _SIZE; ++index) \
            { \
                if (A._NAME[index] != B._NAME[index]) \
                    return false; \
            } \
        }

#define STRUCT_END() \
        return true; \
    }

// Variants

#define VARIANT_BEGIN(_NAME, _DESCRIPTION) \
    inline bool operator == (const _NAME& A, const _NAME& B); \
    inline bool operator != (const _NAME& A, const _NAME& B) \
    { \
        return !(A==B); \
    } \
    bool operator == (const _NAME& A, const _NAME& B) \
    { \
        typedef _NAME ThisType; \
        if (A._index != B._index) \
            return false;

#define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
        if (A._index == ThisType::c_index_##_NAME) \
            return  A._NAME == B._NAME;

#define VARIANT_END() \
        return true; \
    }