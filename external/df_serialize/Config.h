// All STL usage is through these defined types.
// You can use different types with the same interfaces used if you'd like to avoid STL.

#define DFS_LOG(...) Assert(false, __VA_ARGS__);

#ifndef TDYNAMICARRAY
#define TDYNAMICARRAY std::vector
#endif

#ifndef TSTATICARRAY
#define TSTATICARRAY std::array
#endif

#ifndef TSTRING
#define TSTRING std::string
#endif

#ifndef TDYNAMICARRAY_SIZE
#define TDYNAMICARRAY_SIZE(x) x.size()
#endif

#ifndef TDYNAMICARRAY_RESIZE
#define TDYNAMICARRAY_RESIZE(x,y) x.resize(y)
#endif

#ifndef TDYNAMICARRAY_PUSHBACK
#define TDYNAMICARRAY_PUSHBACK(x,y) x.push_back(y)
#endif

#ifndef TSTRING_RESIZE
#define TSTRING_RESIZE(x,y) x.resize(y)
#endif

#include <string>
#include <vector>
#include <array>
#include <unordered_set>
#include "GigiAssert.h"

// flags used by struct fields
#define SCHEMA_FLAG_NONE					((size_t) 0)
#define SCHEMA_FLAG_NO_UI					((size_t) (1 << 0))  // Don't show this field in the UI
#define SCHEMA_FLAG_UI_COLLAPSABLE			((size_t) (1 << 1))  // This field should have a collapsable header
#define SCHEMA_FLAG_UI_ARRAY_FATITEMS		((size_t) (1 << 2))  // If true, puts all items on the same line.
#define SCHEMA_FLAG_NO_SERIALIZE			((size_t) (1 << 3))  // Don't load / save. Also don't show in UI, and don't generate equality tests.
#define SCHEMA_FLAG_SERIALIZE_DFLT			((size_t) (1 << 4))  // Write value, even if it's the default value
#define SCHEMA_FLAG_UI_MULTILINETEXT		((size_t) (1 << 5))  // if true, the text edit box is multiline
#define SCHEMA_FLAG_UI_ARRAY_HIDE_INDEX		((size_t) (1 << 6))  // If true, does not show the index of array items
#define SCHEMA_FLAG_UI_CONST				((size_t) (1 << 7))  // If true, does not allow field to be edited
#define SCHEMA_FLAG_UI_COLOR				((size_t) (1 << 8))  // If true, this value will be displayed as a color picker.
