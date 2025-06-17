///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "Visitors.h"

bool AdjustUniformStructForAlignment_DX12(Struct& s, const std::string& path)
{
    // automatically pad constant buffers per this documentation:
    // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules

    int paddingIndex = -1;
    size_t byteCount = 0;

    // Add padding fields where the types are padded for us
    size_t fieldIndex = 0;
    while (fieldIndex < s.fields.size())
    {
        size_t variableSize = DataFieldTypeToSize(s.fields[fieldIndex].type);
        if (byteCount > 0 && byteCount + variableSize > 16)
        {
            size_t bytesToPad = 16 - byteCount;
            while (bytesToPad > 0)
            {
                paddingIndex++;
                StructField padding;
                std::ostringstream paddingName;
                paddingName << "_padding" << paddingIndex;
                padding.name = paddingName.str();

                if (bytesToPad % 4 == 2)
                {
                    padding.type = DataFieldType::Uint_16;
                    padding.dflt = "0";

                    bytesToPad -= 2;
                }
                else
                {
                    switch (bytesToPad)
                    {
                        case 4:
                        {
                            padding.type = DataFieldType::Float;
                            padding.dflt = "0.0f";
                            break;
                        }
                        case 8:
                        {
                            padding.type = DataFieldType::Float2;
                            break;
                        }
                        case 12:
                        {
                            padding.type = DataFieldType::Float3;
                            break;
                        }
                        default:
                        {
                            Assert(false, "error while calculating padding.\nIn %s\n", path.c_str());
                        }
                    }
                    bytesToPad = 0;
                }
                padding.comment = "Padding";
                padding.sizeInBytes = DataFieldTypeInfo(padding.type).typeBytes;
                padding.isPadding = true;

                s.fields.insert(s.fields.begin() + fieldIndex, padding);
                fieldIndex++;
            }

            byteCount = 0;
        }
        byteCount = (byteCount + variableSize) % 16;
        fieldIndex++;
    }

    // Also, if the struct's final size isn't a multiple of 16 bytes, pad it to be.
    byteCount = byteCount % 16;
    if (byteCount != 0)
    {
        size_t bytesToPad = 16 - byteCount;

        while(bytesToPad > 0)
        {
            paddingIndex++;
            StructField padding;
            std::ostringstream paddingName;
            paddingName << "_padding" << paddingIndex;
            padding.name = paddingName.str();

            if (bytesToPad % 4 == 2)
            {
                padding.type = DataFieldType::Uint_16;
                padding.dflt = "0";

                bytesToPad -= 2;
            }
            else
            {
                switch (bytesToPad)
                {
                    case 4:
                    {
                        padding.type = DataFieldType::Float;
                        padding.dflt = "0.0f";
                        break;
                    }
                    case 8:
                    {
                        padding.type = DataFieldType::Float2;
                        break;
                    }
                    case 12:
                    {
                        padding.type = DataFieldType::Float3;
                        break;
                    }
                    default:
                    {
                        Assert(false, "error while calculating terminating padding.\nIn %s\n", path.c_str());
                    }
                }
                bytesToPad = 0;
            }
            padding.comment = "Padding";
            padding.sizeInBytes = DataFieldTypeInfo(padding.type).typeBytes;
            padding.isPadding = true;

            s.fields.push_back(padding);
        }
        byteCount = 0;
    }

    return paddingIndex >= 0;
}

static size_t DesiredAlignment(DataFieldType type, bool isUniformBuffer)
{
    // Using information from here:
    // https://www.w3.org/TR/WGSL/
    // - 14.4.1. Alignment and Size
    // - 14.4.5 "Address Space Layout Constraints"
    // 
    // 14.4.5 has info for uniform buffers vs not. array and struct have special rules but i don't think we ever use those.
    //
    // Online calculator:
    // https://webgpufundamentals.org/webgpu/lessons/resources/wgsl-offset-computer.html

    switch (type)
    {
        case DataFieldType::Int:
        case DataFieldType::Uint:
        case DataFieldType::Float:
        case DataFieldType::Bool:
            return 4;

        case DataFieldType::Int2:
        case DataFieldType::Uint2:
        case DataFieldType::Float2:
            return 8;

        case DataFieldType::Int3:
        case DataFieldType::Int4:
        case DataFieldType::Uint3:
        case DataFieldType::Uint4:
        case DataFieldType::Float3:
        case DataFieldType::Float4:
        case DataFieldType::Float4x4:
            return 16;

        default:
        {
            Assert(false, "Unhandled DataFieldType in " __FUNCTION__ ": %s (%i)", EnumToString(type), (int)type);
            return 0;
        }
    }
}

bool AdjustStructForAlignment_WebGPU(Struct& s, const std::string& path, bool isUniformBuffer)
{
    // Using information from here:
    // https://www.w3.org/TR/WGSL/
    // - 14.4.1. Alignment and Size
    // - 14.4.5 "Address Space Layout Constraints"
    //
    // Online calculator:
    // https://webgpufundamentals.org/webgpu/lessons/resources/wgsl-offset-computer.html

    int paddingIndex = -1;
    size_t byteCount = 0;

    // Add padding fields where the types are padded for us
    size_t fieldIndex = 0;
    while (fieldIndex < s.fields.size())
    {
        size_t variableSize = DataFieldTypeToSize(s.fields[fieldIndex].type);

        size_t desiredAlignment = DesiredAlignment(s.fields[fieldIndex].type, isUniformBuffer);

        size_t offset = byteCount % desiredAlignment;
        if (offset > 0)
        {
            size_t paddingSize = desiredAlignment - offset;

            for (int i = 0; i < int(paddingSize) / 4; ++i)
            {
                paddingIndex++;

                StructField padding;
                std::ostringstream paddingName;
                paddingName << "_padding" << paddingIndex;
                padding.name = paddingName.str();
                padding.type = DataFieldType::Float;
                padding.dflt = "0.0";
                padding.comment = "Padding";
                padding.sizeInBytes = DataFieldTypeInfo(padding.type).typeBytes;
                padding.isPadding = true;

                s.fields.insert(s.fields.begin() + fieldIndex, padding);

                byteCount = byteCount + 4;
                fieldIndex++;
            }
        }
        byteCount = byteCount + variableSize;
        fieldIndex++;
    }

    // Also, if the struct's final size isn't a multiple of 16 bytes, pad it to be.
    if ((byteCount % 16) != 0)
    {
        size_t paddingSize = 16 - (byteCount % 16);

        for (int i = 0; i < int(paddingSize) / 4; ++i)
        {
            paddingIndex++;

            StructField padding;
            std::ostringstream paddingName;
            paddingName << "_padding" << paddingIndex;
            padding.name = paddingName.str();
            padding.type = DataFieldType::Float;
            padding.dflt = "0.0";
            padding.comment = "Padding";
            padding.sizeInBytes = DataFieldTypeInfo(padding.type).typeBytes;
            padding.isPadding = true;

            s.fields.insert(s.fields.begin() + fieldIndex, padding);

            byteCount = byteCount + 4;
            fieldIndex++;
        }

        paddingIndex++;
        StructField padding;
        std::ostringstream paddingName;
        paddingName << "_padding" << paddingIndex;
        padding.name = paddingName.str();
    }
    s.sizeInBytes = byteCount;

    return paddingIndex >= 0;
}
