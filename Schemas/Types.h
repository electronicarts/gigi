///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

// Include this header to include the types defined by the schemas
#pragma once
#include "external/df_serialize/MakeTypes.h"
#include "schemas.h"

// EnumToString functions
#include "external/df_serialize/_common.h"
#define ENUM_BEGIN(_NAME, _DESCRIPTION) \
inline const char* EnumToString(_NAME value) \
{ \
    static const char* s_strings[] = {

#define ENUM_ITEM(_NAME, _DESCRIPTION) \
        #_NAME,

#define ENUM_END() \
    }; \
    return s_strings[(int)value]; \
}
#include "../external/df_serialize/_fillunsetdefines.h"
#include "schemas.h"

// StringToEnum functions
#include "external/df_serialize/_common.h"
#define ENUM_BEGIN(_NAME, _DESCRIPTION) \
inline bool StringToEnum(const char* value, _NAME& out) \
{ \
    typedef _NAME TheEnum;

#define ENUM_ITEM(_NAME, _DESCRIPTION) \
        if(!_stricmp(value, #_NAME)) \
        { \
            out = TheEnum::_NAME; \
            return true; \
        }

#define ENUM_END() \
    return false; \
}
#include "../external/df_serialize/_fillunsetdefines.h"
#include "schemas.h"

// EnumCount functions
template <typename T>
size_t EnumCount() { return 0; }
#include "external/df_serialize/_common.h"
#define ENUM_BEGIN(_NAME, _DESCRIPTION) template <> inline constexpr size_t EnumCount<_NAME>() { return 0
#define ENUM_ITEM(_NAME, _DESCRIPTION) + 1
#define ENUM_END() ; }
#include "../external/df_serialize/_fillunsetdefines.h"
#include "schemas.h"

// EnumDispatch functions
#include "external/df_serialize/_common.h"
#define ENUM_BEGIN(_NAME, _DESCRIPTION) \
template <typename TCallbackObject> \
void EnumDispatch(TCallbackObject& callbackObject, _NAME value) \
{ \
    typedef _NAME TheEnum; \
    switch(value) \
    {
#define ENUM_ITEM(_NAME, _DESCRIPTION) case TheEnum::##_NAME: callbackObject.Function_##_NAME(); break;
#define ENUM_END() \
    } \
}
#include "../external/df_serialize/_fillunsetdefines.h"
#include "schemas.h"

// EnumToDescription functions
#include "external/df_serialize/_common.h"
#define ENUM_BEGIN(_NAME, _DESCRIPTION) \
inline const char* EnumToDescription(_NAME value) \
{ \
    static const char* s_strings[] = {

#define ENUM_ITEM(_NAME, _DESCRIPTION) \
        _DESCRIPTION,

#define ENUM_END() \
    }; \
    return s_strings[(int)value]; \
}
#include "../external/df_serialize/_fillunsetdefines.h"
#include "schemas.h"
