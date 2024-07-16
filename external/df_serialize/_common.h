#include "Config.h"

// Undefine any macros that may be defined, so they can safely be defined. Lazy define cleanup.

#ifdef STRUCT_BEGIN
#undef STRUCT_BEGIN
#endif

#ifdef STRUCT_INHERIT_BEGIN
#undef STRUCT_INHERIT_BEGIN
#endif

#ifdef STRUCT_FIELD
#undef STRUCT_FIELD
#endif

#ifdef STRUCT_CONST
#undef STRUCT_CONST
#endif

#ifdef STRUCT_END
#undef STRUCT_END
#endif

#ifdef STRUCT_DYNAMIC_ARRAY
#undef STRUCT_DYNAMIC_ARRAY
#endif

#ifdef STRUCT_STATIC_ARRAY
#undef STRUCT_STATIC_ARRAY
#endif

#ifdef ENUM_BEGIN
#undef ENUM_BEGIN
#endif

#ifdef ENUM_ITEM
#undef ENUM_ITEM
#endif

#ifdef ENUM_END
#undef ENUM_END
#endif

#ifdef VARIANT_BEGIN
#undef VARIANT_BEGIN
#endif

#ifdef VARIANT_TYPE
#undef VARIANT_TYPE
#endif

#ifdef VARIANT_END
#undef VARIANT_END
#endif

// Needed to be able to provide defaults for fixed sized arrays
#ifndef COMMA
#define COMMA ,
#endif

#ifndef STRIPCOMMASTRING
#define STRIPCOMMASTRING(x) #x
#endif
