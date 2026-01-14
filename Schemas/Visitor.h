///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <type_traits>

// This creates a function that takes a templated type and will vall visit on that type,
// passing the render graph and data structure for each type visited.

#include "external/df_serialize/_common.h"

// SFINAE to detect if T has a static member 'name'
template<typename T, typename = void>
struct has_name : std::false_type {};

template<typename T>
struct has_name<T, std::void_t<decltype(std::declval<T>().name)>> : std::true_type {};

template <typename T>
std::string MakePathBufferIndexAndName(const T& object, const char* fieldName, int index)
{
    char pathBuffer[256];
    if constexpr (has_name<T>::value)
        sprintf_s(pathBuffer, ".%s[%i](\"%s\")", fieldName, index, object.name.c_str());
    else
        sprintf_s(pathBuffer, ".%s[%i]", fieldName, index);
    return std::string(pathBuffer);
}

// catch all for pods
template <typename TDATA, typename TVISITOR>
bool Visit(TDATA& data, TVISITOR& visitor, const std::string& path)
{
    return visitor.Visit(data, path);
}

// Structs

#define STRUCT_BEGIN(_NAME, _DESCRIPTION) \
    template <typename TVISITOR> \
    bool Visit(_NAME& data, TVISITOR& visitor, const std::string& path) \
    { \
        if(!visitor.Visit(data, path)) \
            return false;

#define STRUCT_INHERIT_BEGIN(_NAME, _BASE, _DESCRIPTION) \
    template <typename TVISITOR> \
    bool Visit(_NAME& data, TVISITOR& visitor, const std::string& path) \
    { \
        if (!Visit(*(_BASE*)&data, visitor, path)) \
            return false; \
        if (!visitor.Visit(data, path)) \
            return false;

#define STRUCT_FIELD(_TYPE, _NAME, _DEFAULT, _DESCRIPTION, _FLAGS) \
        if(!Visit(data._NAME, visitor, path + "." #_NAME)) \
            return false;

#define STRUCT_CONST(_TYPE, _NAME, _DEFAULT, _DESCRIPTION, _FLAGS)

#define STRUCT_DYNAMIC_ARRAY(_TYPE, _NAME, _DESCRIPTION, _FLAGS) \
        if (!Visit(data._NAME, visitor, path + "." #_NAME)) \
            return false; \
        for(int i = 0; i < (int)TDYNAMICARRAY_SIZE(data._NAME); ++i) \
        { \
            if (!Visit(data._NAME[i], visitor, path + MakePathBufferIndexAndName(data._NAME[i], #_NAME, i))) \
                return false; \
        }

#define STRUCT_STATIC_ARRAY(_TYPE, _NAME, _SIZE, _DEFAULT, _DESCRIPTION, _FLAGS) \
        if (!Visit(data._NAME, visitor, path + "." #_NAME)) \
            return false; \
        for(int i = 0; i < (int)_SIZE; ++i) \
        { \
            if (!Visit(data._NAME[i], visitor, path + MakePathBufferIndexAndName(data._NAME[i], #_NAME, i))) \
                return false; \
        }

#define STRUCT_END() \
        return true; \
    }

// Variants

#define VARIANT_BEGIN(_NAME, _DESCRIPTION) \
    template <typename TVISITOR> \
    bool Visit(_NAME& data, TVISITOR& visitor, const std::string& path) \
    { \
        typedef _NAME ThisType; \
        if (!visitor.Visit(data, path)) \
            return false;

#define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
    if (data._index == ThisType::c_index_##_NAME) \
        return Visit(data._NAME, visitor, path + "." #_NAME);

#define VARIANT_END() \
        return true; \
    }

// clang-format off
#include "external/df_serialize/_fillunsetdefines.h"
// clang-format on

#include "Schemas/Schemas.h"
