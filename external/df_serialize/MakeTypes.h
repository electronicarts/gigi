// This creates structs and enums

#include "_common.h"
#include <stdint.h>

// Enums

#define ENUM_BEGIN(_NAME, _DESCRIPTION) \
        enum class _NAME : int32_t \
        {

#define ENUM_ITEM(_NAME, _DESCRIPTION) \
            _NAME,

#define ENUM_END() \
        };

// Structs

#define STRUCT_BEGIN(_NAME, _DESCRIPTION) \
        struct _NAME \
        {

#define STRUCT_INHERIT_BEGIN(_NAME, _BASE, _DESCRIPTION) \
        struct _NAME : public _BASE \
        { \
            typedef _BASE TBaseType; \
            TBaseType& GetBaseType() { return *(TBaseType*)this; } \
            const TBaseType& GetBaseType() const { return *(TBaseType*)this; }

#define STRUCT_FIELD(_TYPE, _NAME, _DEFAULT, _DESCRIPTION, _FLAGS) \
            _TYPE _NAME = _DEFAULT;

#define STRUCT_CONST(_TYPE, _NAME, _DEFAULT, _DESCRIPTION, _FLAGS) \
            inline static const _TYPE _NAME = _DEFAULT;

#define STRUCT_DYNAMIC_ARRAY(_TYPE, _NAME, _DESCRIPTION, _FLAGS) \
            TDYNAMICARRAY<_TYPE> _NAME;

#define STRUCT_STATIC_ARRAY(_TYPE, _NAME, _SIZE, _DEFAULT, _DESCRIPTION, _FLAGS) \
            TSTATICARRAY<_TYPE, _SIZE> _NAME = _DEFAULT;

#define STRUCT_END() \
        };

// Variants

#define VARIANT_BEGIN(_NAME, _DESCRIPTION) \
        struct _NAME \
        { \
            static const uint32_t c_index__None = __COUNTER__; \
            uint32_t _index = c_index__None;

#define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
            static const uint32_t c_index_##_NAME = __COUNTER__; \
            _TYPE _NAME = _DEFAULT;

#define VARIANT_END() \
            static const uint32_t c_index__Count = __COUNTER__ - c_index__None - 1; \
        };
