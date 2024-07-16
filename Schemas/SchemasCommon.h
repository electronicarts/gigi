///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "BackendList.h"
#include "Version.h"

STRUCT_BEGIN(BackendRestriction, "Allows restriction to specific backends")
    STRUCT_DYNAMIC_ARRAY(Backend, backends, "A list of backends supported. Empty list means all backends", 0)
    STRUCT_FIELD(bool, isWhiteList, true, "If true, this is a list of allowed platforms. if false, it's a list of disallowed platforms.", 0)

    STRUCT_FIELD(unsigned int, backendFlags, 0, "Calculated for convenience.", SCHEMA_FLAG_NO_SERIALIZE)
STRUCT_END()