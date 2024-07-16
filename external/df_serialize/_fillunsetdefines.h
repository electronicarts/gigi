// Define any undefined defines, useful for when you really only care about specific ones

// Enums

#ifndef ENUM_BEGIN
#define ENUM_BEGIN(_NAME, _DESCRIPTION) 
#endif

#ifndef ENUM_ITEM
#define ENUM_ITEM(_NAME, _DESCRIPTION) 
#endif

#ifndef ENUM_END
#define ENUM_END() 
#endif

// Structs

#ifndef STRUCT_BEGIN
#define STRUCT_BEGIN(_NAME, _DESCRIPTION) 
#endif

#ifndef STRUCT_INHERIT_BEGIN
#define STRUCT_INHERIT_BEGIN(_NAME, _BASE, _DESCRIPTION) 
#endif

#ifndef STRUCT_FIELD
#define STRUCT_FIELD(_TYPE, _NAME, _DEFAULT, _DESCRIPTION, _FLAGS) 
#endif

#ifndef STRUCT_CONST
#define STRUCT_CONST(_TYPE, _NAME, _DEFAULT, _DESCRIPTION, _FLAGS) 
#endif

#ifndef STRUCT_DYNAMIC_ARRAY
#define STRUCT_DYNAMIC_ARRAY(_TYPE, _NAME, _DESCRIPTION, _FLAGS) 
#endif

#ifndef STRUCT_STATIC_ARRAY
#define STRUCT_STATIC_ARRAY(_TYPE, _NAME, _SIZE, _DEFAULT, _DESCRIPTION, _FLAGS) 
#endif

#ifndef STRUCT_END
#define STRUCT_END() 
#endif

// Variants

#ifndef VARIANT_BEGIN
#define VARIANT_BEGIN(_NAME, _DESCRIPTION)
#endif

#ifndef VARIANT_TYPE
#define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION)
#endif

#ifndef VARIANT_END
#define VARIANT_END()
#endif
