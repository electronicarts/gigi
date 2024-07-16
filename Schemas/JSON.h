///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// clang-format off
#include "external/rapidjson/rapidjson.h"
#include "external/rapidjson/document.h"
#include "external/rapidjson/error/en.h"
#include "external/rapidjson/stringbuffer.h"
#include "external/rapidjson/writer.h"
#include "external/rapidjson/prettywriter.h"

#include "external/df_serialize/MakeJSONReadHeader.h"
#include "Schemas.h"
#include "external/df_serialize/MakeJSONReadFooter.h"
// clang-format on

// JSON Read Override functions

template <typename T>
JSONReadOverrideResult JSONReadOverride(RenderGraph& value, T& document)
{
    if (!document.HasMember("$schema") || !document["$schema"].IsString())
        return JSONReadOverrideResult::Error;

    JSONRead(value.schema, document["$schema"]);

    // Before 0.91b, this is how the version was read in. We can get rid of this code when the upgrade path is no longer needed.
    size_t versionStart = value.schema.find("gigischema_", 0);
    if (versionStart == std::string::npos)
        return JSONReadOverrideResult::Continue;
    versionStart += 11;

    size_t versionEnd = value.schema.find(".json", versionStart);
    if (versionEnd == std::string::npos)
        return JSONReadOverrideResult::Continue;

    value.version = value.schema.substr(versionStart, versionEnd - versionStart);
    value.schema = "gigischema.json";

    return JSONReadOverrideResult::Continue;
}

// clang-format off
#include "external/df_serialize/MakeEqualityTests.h"
#include "Schemas.h"

#include "external/df_serialize/MakeJSONWriteHeader.h"
#include "Schemas.h"
#include "external/df_serialize/MakeJSONWriteFooter.h"
// clang-format on