///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

// Moved to it's own file so macro expansion can be done on it

ENUM_BEGIN(DataFieldType, "The type of a data field")
    ENUM_ITEM(Int, "int")
    ENUM_ITEM(Int2, "int[2]")
    ENUM_ITEM(Int3, "int[3]")
    ENUM_ITEM(Int4, "int[4]")
    ENUM_ITEM(Uint, "uint")
    ENUM_ITEM(Uint2, "uint[2]")
    ENUM_ITEM(Uint3, "uint[3]")
    ENUM_ITEM(Uint4, "uint[4]")
    ENUM_ITEM(Float, "float")
    ENUM_ITEM(Float2, "float[2]")
    ENUM_ITEM(Float3, "float[3]")
    ENUM_ITEM(Float4, "float[4]")
    ENUM_ITEM(Bool, "bool")
    ENUM_ITEM(Float4x4, "float[4][4]")
    ENUM_ITEM(Uint_16, "a 16 bit uint")
    ENUM_ITEM(Int_64, "a 64 bit int")
    ENUM_ITEM(Uint_64, "a 64 bit uint")
    ENUM_ITEM(Float_16, "a 16 bit float")
    ENUM_ITEM(Count, "")
ENUM_END()
