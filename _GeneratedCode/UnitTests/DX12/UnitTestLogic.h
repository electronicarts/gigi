// This file was created manually for DX12 unit tests, to write the logic for each unit test

#pragma once

#include "DX12Utils/dxutils.h"
#include "DX12Utils/TextureCache.h"
#include "DX12Utils/FileCache.h"
#include "DX12Utils/ParseCSV.h"

#include <sstream>
#include <stdarg.h>
#include <filesystem>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "DX12Utils/stb/stb_image_write.h"

enum class UnitTestEvent
{
    PreExecute,
    PostExecute
};

// A unit test will not end until at least one "VerifyReadback" has been issued.
// It will end when all of the readbacks are resolved after that.
// If any readback didnt match what it should, the test will fail, else it will pass.
struct UnitTestContext
{
public:
    UnitTestContext(const char* techniqueName, DX12Utils::ReadbackHelper& readbackHelper, TLogFn logFn)
        : m_techniqueName(techniqueName)
        , m_readbackHelper(readbackHelper)
        , m_logFn(logFn)
    {
        s_unitTestsInFlight++;
    }

    bool IsFirstPreExecute(UnitTestEvent event)
    {
        if (event != UnitTestEvent::PreExecute)
            return false;

        if (!m_firstPreExecute)
            return false;

        m_firstPreExecute = false;
        return true;
    }

    bool IsFirstPostExecute(UnitTestEvent event)
    {
        if (event != UnitTestEvent::PostExecute)
            return false;

        if (!m_firstPostExecute)
            return false;

        m_firstPostExecute = false;
        return true;
    }

    void Fail(const char* reason, ...)
    {
        if (m_done)
            return;

        char buffer[4096];
        va_list args;
        va_start(args, reason);
        vsprintf_s(buffer, reason, args);
        va_end(args);

        TestResult result;
        result.testName = m_techniqueName;
        result.fail = true;
        result.text = buffer;
        s_testResults.push_back(result);

        s_unitTestsFailed++;
        OnUnitTestFinished();
    }

    void Pass()
    {
        if (m_done)
            return;

        TestResult result;
        result.testName = m_techniqueName;
        result.fail = false;
        s_testResults.push_back(result);

        s_unitTestsPassed++;
        OnUnitTestFinished();
    }

    void VerifyReadbackPNG(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES resourceState, int arrayIndex, int mipIndex, const char* fileName)
    {
        m_hasAddedReadback = true;
        VerifyReadback newReadback;
        newReadback.type = ReadbackType::PNG;
        newReadback.fileName = fileName;
        newReadback.readbackId = m_readbackHelper.RequestReadback(device, commandList, resource, resourceState, arrayIndex, mipIndex, m_logFn);
        m_readbacks.push_back(newReadback);
    }

    void VerifyReadbackBinary(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES resourceState, int arrayIndex, int mipIndex, const char* fileName)
    {
        m_hasAddedReadback = true;
        VerifyReadback newReadback;
        newReadback.type = ReadbackType::Binary;
        newReadback.fileName = fileName;
        newReadback.readbackId = m_readbackHelper.RequestReadback(device, commandList, resource, resourceState, arrayIndex, mipIndex, m_logFn);
        m_readbacks.push_back(newReadback);
    }

    bool Tick(UnitTestEvent event)
    {
        if (m_done)
            return false;

        // Only Tick in PostExecute, so we only check once a frame
        if (event != UnitTestEvent::PostExecute)
            return true;

        // Don't process until we have added readbacks (this is to keep from hitting the Pass() at the end of the function)
        if (!m_hasAddedReadback)
            return true;

        // TODO: The viewer has much better code for this (ImageReadback.h etc). We should make that be part of dx12 utils and share it between the two.

        // Process readbacks
        m_readbacks.erase(
            std::remove_if(m_readbacks.begin(), m_readbacks.end(), 
                [&] (VerifyReadback& readback)
                {
                    if (m_readbackHelper.ReadbackReady(readback.readbackId))
                    {
                        // Get the readback data
                        D3D12_RESOURCE_DESC resourceDesc;
                        int arrayIndex = 0;
                        int mipIndex = 0;
                        std::vector<unsigned char> data = m_readbackHelper.GetReadbackData(readback.readbackId, resourceDesc, arrayIndex, mipIndex);

                        // calculate readback size, taking into account mipIndex
                        int readbackWidth = resourceDesc.Width;
                        int readbackHeight = resourceDesc.Height;
                        int readbackDepth = (resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D) ? resourceDesc.DepthOrArraySize : 1;
                        {
                            for (int i = 0; i < mipIndex; ++i)
                            {
                                readbackWidth = max(readbackWidth / 2, 1);
                                readbackHeight = max(readbackHeight / 2, 1);
                                readbackDepth = max(readbackDepth / 2, 1);
                            }
                        }

                        switch (readback.type)
                        {
                            case ReadbackType::PNG:
                            {
                                DX12Utils::DXGI_FORMAT_Info formatInfo = DX12Utils::Get_DXGI_FORMAT_Info(resourceDesc.Format, m_logFn);

                                // Need to keep only the desired z slice (arrayIndex) of a 3d texture
                                if (resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
                                {
                                    int imageSize = readbackWidth * readbackHeight * formatInfo.bytesPerPixel;
                                    unsigned char* sliceStart = data.data() + arrayIndex * imageSize;
                                    unsigned char* sliceEnd = sliceStart + imageSize;
                                    data = std::vector<unsigned char>(sliceStart, sliceEnd);
                                    readbackDepth = 1;
                                }

                                std::vector<DX12Utils::TextureCache::Texture> textures;
                                if (strstr(readback.fileName, "%i"))
                                {
                                    char fileName[1024];
                                    int i = 0;
                                    while (1)
                                    {
                                        sprintf(fileName, readback.fileName, i);
                                        i++;
                                        textures.push_back(DX12Utils::TextureCache::Get(fileName));
                                        if (!textures.rbegin()->Valid())
                                            break;
                                    }
                                }
                                else
                                {
                                    textures.push_back(DX12Utils::TextureCache::Get(readback.fileName));
                                }

                                size_t totalLoadedTextureSize = 0;
                                for (const auto& texture : textures)
                                    totalLoadedTextureSize += texture.pixels.size();

                                if (!textures.rbegin()->Valid())
                                    textures.pop_back();

                                if (textures.empty())
                                    Fail("Could not load gold image %s", readback.fileName);

                                // compare dimensions
                                else if (textures[0].width != readbackWidth || textures[0].height != readbackHeight || textures.size() != readbackDepth || textures[0].channels != formatInfo.channelCount || totalLoadedTextureSize != data.size())
                                    Fail("Data size mismatch in %s", readback.fileName);

                                // make sure the format type is correct
                                else if (formatInfo.channelType != DX12Utils::DXGI_FORMAT_Info::ChannelType::_uint8_t)
                                    Fail("Data is wrong format in %s", readback.fileName);

                                else if(textures[0].type != DX12Utils::TextureCache::Type::U8)
                                    Fail("Loaded texture is wrong format in %s", readback.fileName);

                                // compare data
                                else
                                {
                                    size_t offset = 0;
                                    for (int i = 0; i < textures.size(); ++i)
                                    {
                                        DX12Utils::TextureCache::Texture& texture = textures[i];

                                        if (memcmp(texture.pixels.data(), &data[offset], texture.pixels.size()))
                                        {
                                            Fail("Data is different in %s [%i]", readback.fileName, i);

                                            // Uncomment to help debugging
                                            /*
                                            for (size_t i = 0; i < data.size(); ++i)
                                            {
                                                if (texture.pixels[i] != data[i])
                                                {
                                                    int ijkl = 0;
                                                }
                                            }

                                            // Uncomment to help debugging
                                            stbi_write_png("_gold.png", texture.width, texture.height, texture.channels, texture.pixels.data(), 0);
                                            stbi_write_png("_readback.png", texture.width, texture.height, texture.channels, data.data(), 0);
                                            */
                                            break;
                                        }
                                        offset += textures[i].pixels.size();
                                    }
                                }

                                break;
                            }
                            case ReadbackType::Binary:
                            {
                                DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get(readback.fileName);
                                if (!file.Valid())
                                    Fail("Could not load gold image %s", readback.fileName);

                                // compare dimensions
                                else if (file.GetSize() != data.size())
                                    Fail("Data size mismatch in %s", readback.fileName);

                                // compare data
                                else if (memcmp(file.GetBytes(), data.data(), data.size()))
                                {
                                    Fail("Data is different in %s", readback.fileName);

                                    // Uncomment to help debugging
                                    /*
                                    const unsigned char* fileData = (const unsigned char*)file.GetBytes();
                                    for (size_t i = 0; i < data.size(); ++i)
                                    {
                                        if (fileData[i] != data[i])
                                        {
                                            int ijkl = 0;
                                        }
                                    }
                                    */
                                }

                                break;
                            }
                            default:
                            {
                                Fail("Unhandled readback type");
                                break;
                            }
                        }

                        // We are done verifying this readback entry
                        return true;
                    }

                    // we haven't verified this readback entry yet
                    return false;
                }
            ),
            m_readbacks.end()
        );

        // If we don't have any more readbacks to verify, and we haven't already failed, that is a test success
        if (m_readbacks.size() == 0 && !m_done)
            Pass();

        return true;
    }

    bool                        m_hasAddedReadback = false;

private:
    void OnUnitTestFinished()
    {
        s_unitTestsInFlight--;
        if (s_unitTestsInFlight == 0)
        {
            for (const TestResult& testResult : s_testResults)
            {
                if (testResult.fail)
                    m_logFn(LogLevel::Error, "[%s] failed: %s", testResult.testName.c_str(), testResult.text.c_str());
                else
                    m_logFn(LogLevel::Info, "[%s] passed", testResult.testName.c_str());
            }

            m_logFn((s_unitTestsFailed > 0) ? LogLevel::Error : LogLevel::Info, "%i passed, %i failed.\n", s_unitTestsPassed, s_unitTestsFailed);
            PostQuitMessage(s_unitTestsFailed > 0 ? 1 : 0);
        }
        m_done = true;
    }

private:
    enum class ReadbackType
    {
        PNG,
        Binary
    };

    struct VerifyReadback
    {
        ReadbackType type;
        const char* fileName;
        int readbackId;
    };

    bool                            m_firstPreExecute = true;
    bool                            m_firstPostExecute = true;
    bool                            m_done = false;
    const char*                     m_techniqueName = nullptr;
    std::vector<VerifyReadback>     m_readbacks;
    DX12Utils::ReadbackHelper&      m_readbackHelper;
    TLogFn                          m_logFn;

private:
    struct TestResult
    {
        std::string testName;
        bool fail = false;
        std::string text;
    };

    static std::vector<TestResult>  s_testResults;
    static int                      s_unitTestsInFlight;
    static int                      s_unitTestsPassed;
    static int                      s_unitTestsFailed;
};

std::vector<UnitTestContext::TestResult> UnitTestContext::s_testResults;
int UnitTestContext::s_unitTestsInFlight = 0;
int UnitTestContext::s_unitTestsPassed = 0;
int UnitTestContext::s_unitTestsFailed = 0;

template <typename TContextType>
void UnitTest(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, TContextType* context, UnitTestEvent event)
{
    static UnitTestContext s_testContext(context->GetTechniqueName(), readbackHelper, context->LogFn);
    if (!s_testContext.Tick(event))
        return;

    UnitTestImpl(s_testContext, device, commandList, readbackHelper, context, event);
}

template <typename TContextType>
void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, TContextType* context, UnitTestEvent event)
{
    // Wait until post execute to fail, so we don't reach 0 tests in flight before everything has had a chance to increment tests in flight
    if (testContext.IsFirstPostExecute(event))
        testContext.Fail("Not implemented");
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, BarrierTest::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPostExecute(event))
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Output, context->m_output.c_texture_Output_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Barrier\\BarrierTest\\0.png");
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, IndirectDispatch::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPostExecute(event))
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Render_Target, context->m_output.c_texture_Render_Target_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Compute\\IndirectDispatch\\out.png");
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, buffertest::Context* context, UnitTestEvent event)
{
    // Create buffers and put them into the context as inputs, as appropriate
    if (testContext.IsFirstPreExecute(event))
    {
        // Load, create and set buffer_InputStructuredBuffer
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Buffers\\buffertest_structuredbuffer.csv");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Buffers\\buffertest_structuredbuffer.csv");
                return;
            }

            buffertest::Struct_TestStruct inputStructuredBufferData;
            bool result = DX12Utils::ParseCSV::ForEachValue(file.GetBytes(), true,
                [&inputStructuredBufferData] (int tokenIndex, const char* token)
                {
                    // skip empty tokens, caused by trailing commas
                    if (token[0] == 0)
                        return true;

                    switch (tokenIndex)
                    {
                        // float4 TheFloat4 = {0.0, 0.0, 0.0, 0.0};
                        case 0:
                        case 1:
                        case 2:
                        case 3:
                        {
                            sscanf(token, "%f", &inputStructuredBufferData.TheFloat4[tokenIndex]);
                            break;
                        }
                        // int4 TheInt4 = {0, 0, 0, 0};
                        case 4:
                        case 5:
                        case 6:
                        case 7:
                        {
                            sscanf(token, "%i", &inputStructuredBufferData.TheInt4[tokenIndex - 4]);
                            break;
                        }
                        // unsigned int TheBool = false;
                        case 8:
                        {
                            if (!stricmp(token, "false"))
                                inputStructuredBufferData.TheBool = false;
                            if (!stricmp(token, "true"))
                                inputStructuredBufferData.TheBool = true;
                            else
                                return false;
                            break;
                        }
                        default: return false;
                    }
                    return true;
                }
            );

            if (!result)
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Buffers\\buffertest_structuredbuffer.csv");

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_InputStructuredBuffer_format = DXGI_FORMAT_UNKNOWN;
            context->m_input.buffer_InputStructuredBuffer_stride = sizeof(inputStructuredBufferData);
            context->m_input.buffer_InputStructuredBuffer_count = 1;
            context->m_input.buffer_InputStructuredBuffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_InputStructuredBuffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_InputStructuredBuffer_flags, inputStructuredBufferData, context->GetTechniqueNameW());
        }

        // Load, create and set buffer_InputTypedBuffer
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Buffers\\buffertest_typedbuffer.csv");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Buffers\\buffertest_typedbuffer.csv");
                return;
            }

            std::vector<float> inputTypedBufferData;

            bool result = DX12Utils::ParseCSV::ForEachValue(file.GetBytes(), true,
                [&inputTypedBufferData](int tokenIndex, const char* token)
                {
                    // skip empty tokens, caused by trailing commas
                    if (token[0] == 0)
                        return true;

                    float f;
                    sscanf(token, "%f", &f);
                    inputTypedBufferData.push_back(f);
                    return true;
                }
            );

            if (!result)
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Buffers\\buffertest_typedbuffer.csv");

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_InputTypedBuffer_format = DXGI_FORMAT_R32_FLOAT;
            context->m_input.buffer_InputTypedBuffer_stride = 0;
            context->m_input.buffer_InputTypedBuffer_count = inputTypedBufferData.size();
            context->m_input.buffer_InputTypedBuffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_InputTypedBuffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_InputTypedBuffer_flags, inputTypedBufferData, context->GetTechniqueNameW());
        }

        // Load, create and set buffer_InputTypedBufferRaw
        {

            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Buffers\\buffertest_typedbuffer.csv");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Buffers\\buffertest_typedbuffer.csv");
                return;
            }

            std::vector<float> inputTypedBufferRawData;

            bool result = DX12Utils::ParseCSV::ForEachValue(file.GetBytes(), true,
                [&inputTypedBufferRawData](int tokenIndex, const char* token)
                {
                    // skip empty tokens, caused by trailing commas
                    if (token[0] == 0)
                        return true;

                    float f;
                    sscanf(token, "%f", &f);
                    inputTypedBufferRawData.push_back(f);
                    return true;
                }
            );

            if (!result)
                testContext.Fail("Could not load %s ..\\..\\..\\Techniques\\UnitTests\\Buffers\\buffertest_typedbuffer.csv");

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_InputTypedBufferRaw_format = DXGI_FORMAT_R32_FLOAT;
            context->m_input.buffer_InputTypedBufferRaw_stride = 0;
            context->m_input.buffer_InputTypedBufferRaw_count = inputTypedBufferRawData.size();
            context->m_input.buffer_InputTypedBufferRaw_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_InputTypedBufferRaw = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_InputTypedBufferRaw_flags, inputTypedBufferRawData, context->GetTechniqueNameW());
        }
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.buffer_OutputTypedBuffer, D3D12_RESOURCE_STATE_COPY_DEST, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Buffers\\buffertest\\0.bin");
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.buffer_OutputStructuredBuffer, D3D12_RESOURCE_STATE_COPY_DEST, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Buffers\\buffertest\\1.bin");
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.buffer_OutputTypedBufferRaw, D3D12_RESOURCE_STATE_COPY_DEST, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Buffers\\buffertest\\2.bin");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, StructuredBuffer::Context* context, UnitTestEvent event)
{
    // Create buffers and put them into the context as inputs, as appropriate
    if (testContext.IsFirstPreExecute(event))
    {
        StructuredBuffer::Struct_TheStructure data[2];

        // Create a buffer and set the buffer data on the technique context
        context->m_input.buffer_buff_format = DXGI_FORMAT_UNKNOWN;
        context->m_input.buffer_buff_stride = sizeof(data[0]);
        context->m_input.buffer_buff_count = _countof(data);
        context->m_input.buffer_buff_state = D3D12_RESOURCE_STATE_COPY_DEST;
        context->m_input.buffer_buff = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_buff_flags, data, _countof(data), context->GetTechniqueNameW());
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackBinary(device, commandList, context->m_input.buffer_buff, D3D12_RESOURCE_STATE_COPY_DEST, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Buffers\\StructuredBuffer\\0.bin");
        // Note: not verifying contents of constant buffer, like the python unit test does. We don't have access to the constant buffer here, and it's a low enough risk thing that i'm not going to expose it right now.
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, boxblur::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        context->m_input.texture_InputTexture_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        context->m_input.texture_InputTexture_state = D3D12_RESOURCE_STATE_COPY_DEST;
        context->m_input.texture_InputTexture = context->CreateManagedTextureFromFile(
            device,
            commandList,
            context->m_input.texture_InputTexture_flags,
            context->m_input.texture_InputTexture_format,
            DX12Utils::ResourceType::Texture2D,
            "..\\..\\..\\Techniques\\UnitTests\\cabinsmall.png",
            true,
            context->m_input.texture_InputTexture_size,
            context->GetTechniqueNameW()
        );

        context->m_input.variable_radius = 20;
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_InputTexture, context->m_input.texture_InputTexture_state, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Compute\\boxblur\\0.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, ReadbackSequence::Context* context, UnitTestEvent event)
{
    static int s_frameIndex = 0;

    static const char* fileNames[] =
    {
        "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Compute\\ReadbackSequence\\0.png",
        "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Compute\\ReadbackSequence\\1.png",
        "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Compute\\ReadbackSequence\\2.png",
        "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Compute\\ReadbackSequence\\3.png",
        "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Compute\\ReadbackSequence\\4.png",
        "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Compute\\ReadbackSequence\\5.png",
        "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Compute\\ReadbackSequence\\6.png",
        "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Compute\\ReadbackSequence\\7.png",
        "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Compute\\ReadbackSequence\\8.png",
        "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Compute\\ReadbackSequence\\9.png",
    };

    if (testContext.IsFirstPreExecute(event))
    {
        uint32_t clearValue = (uint32_t)0xFFFFFFFF;

        context->m_input.texture_Output_format = DXGI_FORMAT_R8G8B8A8_UNORM;
        context->m_input.texture_Output_state = D3D12_RESOURCE_STATE_COPY_DEST;
        context->m_input.texture_Output_size[0] = 64;
        context->m_input.texture_Output_size[1] = 64;
        context->m_input.texture_Output_size[2] = 1;
        context->m_input.texture_Output_numMips = 1;
        context->m_input.texture_Output = context->CreateManagedTextureAndClear(
            device,
            commandList,
            context->m_input.texture_Output_flags,
            context->m_input.texture_Output_format,
            context->m_input.texture_Output_size,
            context->m_input.texture_Output_numMips,
            DX12Utils::ResourceType::Texture2D,
            &clearValue, sizeof(clearValue),
            context->GetTechniqueNameW()
        );
    }

    if (event == UnitTestEvent::PreExecute)
    {
        context->m_input.variable_frameIndex = s_frameIndex;
    }

    if (testContext.IsFirstPostExecute(event) || event == UnitTestEvent::PostExecute)
    {
        if (s_frameIndex < 10)
        {
            testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_Output, context->m_input.texture_Output_state, 0, 0, fileNames[s_frameIndex]);
            s_frameIndex++;
        }
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, simple::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        int width, height, depth;
        context->m_input.texture_Input_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        context->m_input.texture_Input_state = D3D12_RESOURCE_STATE_COPY_DEST;
        context->m_input.texture_Input = context->CreateManagedTextureFromFile(
            device,
            commandList,
            context->m_input.texture_Input_flags,
            context->m_input.texture_Input_format,
            DX12Utils::ResourceType::Texture2D,
            "..\\..\\..\\Techniques\\UnitTests\\cabinsmall.png",
            true,
            context->m_input.texture_Input_size,
            context->GetTechniqueNameW()
        );
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_Input, context->m_input.texture_Input_state, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Compute\\simple\\0.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, SlangAutoDiff::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        uint32_t clearValue = (uint32_t)0xFFFFFFFF;

        context->m_input.texture_Output_format = DXGI_FORMAT_R8G8B8A8_UNORM;
        context->m_input.texture_Output_state = D3D12_RESOURCE_STATE_COPY_DEST;
        context->m_input.texture_Output_size[0] = 512;
        context->m_input.texture_Output_size[1] = 512;
        context->m_input.texture_Output_size[2] = 1;
        context->m_input.texture_Output_numMips = 1;
        context->m_input.texture_Output = context->CreateManagedTextureAndClear(
            device,
            commandList,
            context->m_input.texture_Output_flags,
            context->m_input.texture_Output_format,
            context->m_input.texture_Output_size,
            context->m_input.texture_Output_numMips,
            DX12Utils::ResourceType::Texture2D,
            &clearValue, sizeof(clearValue),
            context->GetTechniqueNameW()
        );

        context->m_input.variable_NumGaussians = 10;
        context->m_input.variable_LearningRate = 0.1f;
        context->m_input.variable_MaximumStepSize = 0.01f;
        context->m_input.variable_UseBackwardAD = true;
        context->m_input.variable_QuantizeDisplay = true;
        context->m_input.variable_Reset = true;
        context->m_input.variable_initialized = false;

        context->m_input.variable_FrameIndex = 0;
        context->m_input.variable_iResolution[0] = 512.0f;
        context->m_input.variable_iResolution[1] = 512.0f;
        context->m_input.variable_iResolution[2] = 1.0f;
    }

    if (testContext.IsFirstPostExecute(event) || event == UnitTestEvent::PostExecute)
    {
        if (context->m_input.variable_FrameIndex == 1)
        {
            testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_Output, context->m_input.texture_Output_state, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Compute\\SlangAutoDiff\\0.png");
            // Note: not looking at 1.npy.  That has derivatives and ball position, but those are used to render 0.png so are already covered.
        }

        context->m_input.variable_FrameIndex++;
        context->m_input.variable_initialized = true;
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, CopyResourceTest::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        // Load and create "Source Buffer"
        {
            // Load the data as a bunch of floats
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\CopyResource\\out.csv");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\CopyResource\\out.csv");
                return;
            }

            std::vector<float> bufferData;

            bool result = DX12Utils::ParseCSV::ForEachValue(file.GetBytes(), true,
                [&bufferData](int tokenIndex, const char* token)
                {
                    // skip empty tokens, caused by trailing commas
                    if (token[0] == 0)
                        return true;

                    float f;
                    sscanf(token, "%f", &f);
                    bufferData.push_back(f);
                    return true;
                }
            );

            if (!result)
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\CopyResource\\out.csv");

            if (bufferData.size() % 4 != 0)
                testContext.Fail("Does not contain a whole number of float4s! ..\\..\\..\\Techniques\\UnitTests\\CopyResource\\out.csv");

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_Source_Buffer_format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            context->m_input.buffer_Source_Buffer_stride = 0;
            context->m_input.buffer_Source_Buffer_count = bufferData.size() / 4;
            context->m_input.buffer_Source_Buffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_Source_Buffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_Source_Buffer_flags, bufferData, context->GetTechniqueNameW());
        }

        // Load and create "Source Texture"
        {
            context->m_input.texture_Source_Texture_format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            context->m_input.texture_Source_Texture_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_Source_Texture = context->CreateManagedTextureFromFile(
                device,
                commandList,
                context->m_input.texture_Source_Texture_flags,
                context->m_input.texture_Source_Texture_format,
                DX12Utils::ResourceType::Texture2D,
                "..\\..\\..\\Techniques\\UnitTests\\CopyResource\\img.png",
                true,
                context->m_input.texture_Source_Texture_size,
                context->GetTechniqueNameW()
            );
        }
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.texture_Texture_From_Texture, context->m_output.c_texture_Texture_From_Texture_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\CopyResource\\CopyResourceTest\\0.bin");
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.texture_Texture_From_Buffer, context->m_output.c_texture_Texture_From_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\CopyResource\\CopyResourceTest\\1.bin");
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.buffer_Buffer_From_Texture, context->m_output.c_buffer_Buffer_From_Texture_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\CopyResource\\CopyResourceTest\\2.bin");
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.buffer_Buffer_From_Buffer, context->m_output.c_buffer_Buffer_From_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\CopyResource\\CopyResourceTest\\3.bin");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, CopyResourceTest_FB::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        // Load and create "Source Buffer"
        {
            // Load the data as a bunch of floats
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\CopyResource\\out.csv");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\CopyResource\\out.csv");
                return;
            }

            std::vector<float> bufferData;

            bool result = DX12Utils::ParseCSV::ForEachValue(file.GetBytes(), true,
                [&bufferData](int tokenIndex, const char* token)
                {
                    // skip empty tokens, caused by trailing commas
                    if (token[0] == 0)
                        return true;

                    float f;
                    sscanf(token, "%f", &f);
                    bufferData.push_back(f);
                    return true;
                }
            );

            if (!result)
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\CopyResource\\out.csv");

            if (bufferData.size() % 4 != 0)
                testContext.Fail("Does not contain a whole number of float4s! ..\\..\\..\\Techniques\\UnitTests\\CopyResource\\out.csv");

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_Source_Buffer_format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            context->m_input.buffer_Source_Buffer_stride = 0;
            context->m_input.buffer_Source_Buffer_count = bufferData.size() / 4;
            context->m_input.buffer_Source_Buffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_Source_Buffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_Source_Buffer_flags, bufferData, context->GetTechniqueNameW());
        }

        // Load and create "Source Texture"
        {
            context->m_input.texture_Source_Texture_format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            context->m_input.texture_Source_Texture_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_Source_Texture = context->CreateManagedTextureFromFile(
                device,
                commandList,
                context->m_input.texture_Source_Texture_flags,
                context->m_input.texture_Source_Texture_format,
                DX12Utils::ResourceType::Texture2D,
                "..\\..\\..\\Techniques\\UnitTests\\CopyResource\\img.png",
                true,
                context->m_input.texture_Source_Texture_size,
                context->GetTechniqueNameW()
            );
        }
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.texture_Texture_From_Texture, context->m_output.c_texture_Texture_From_Texture_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\CopyResource\\CopyResourceTest_FB\\0.bin");
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.buffer_Buffer_From_Buffer, context->m_output.c_buffer_Buffer_From_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\CopyResource\\CopyResourceTest_FB\\1.bin");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, VRS::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        // Load, create and set VertexBuffer
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Raster\\simpleRasterVB.bin");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Raster\\simpleRasterVB.bin");
                return;
            }

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_VertexBuffer_format = DXGI_FORMAT_UNKNOWN;
            context->m_input.buffer_VertexBuffer_stride = sizeof(simpleRaster::Struct_VertexFormat);
            context->m_input.buffer_VertexBuffer_count = file.GetSize() / sizeof(simpleRaster::Struct_VertexFormat);
            context->m_input.buffer_VertexBuffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_VertexBuffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_VertexBuffer_flags, file.GetBytes(), file.GetSize(), context->GetTechniqueNameW());

            // Set up the vertex input layout for the vertex buffer
            context->m_input.buffer_VertexBuffer_vertexInputLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
            context->m_input.buffer_VertexBuffer_vertexInputLayout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
        }

        // Making sure the matrix is the exact same as in the test.
        // You can save as hex in the gigi viewer
        uint32_t viewProjMtx[] = {
            0xC01A35C1, 0x00000000, 0x3E19E5DC, 0xBF2D3533,
            0xBA050FAE, 0x401A823F, 0xBC0554C1, 0xBF5DB8D7,
            0x36D0E790, 0x34B556E5, 0x38D15403, 0x3DCB589F,
            0xBD7EFC24, 0xBB5D56E8, 0xBF7F8083, 0x40E6563E
        };
        memcpy(&context->m_input.variable_ViewProjMtx, viewProjMtx, sizeof(viewProjMtx));
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.texture_Depth_Buffer, context->m_output.c_texture_Depth_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\VRS\\0.bin");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Color_Buffer, context->m_output.c_texture_Color_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\VRS\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Mesh::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        // Load, create and set VertexBuffer
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Raster\\simpleRasterVB.bin");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Raster\\simpleRasterVB.bin");
                return;
            }

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_VertexBuffer_format = DXGI_FORMAT_UNKNOWN;
            context->m_input.buffer_VertexBuffer_stride = sizeof(simpleRaster::Struct_VertexFormat);
            context->m_input.buffer_VertexBuffer_count = file.GetSize() / sizeof(simpleRaster::Struct_VertexFormat);
            context->m_input.buffer_VertexBuffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_VertexBuffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_VertexBuffer_flags, file.GetBytes(), file.GetSize(), context->GetTechniqueNameW());
        }

        // Making sure the matrix is the exact same as in the test.
        // You can save as hex in the gigi viewer
        uint32_t viewProjMtx[] = {
            0xBFFD3CFB, 0x00000000, 0xBFB118D6, 0xBF7C1BFF,
            0xBDA6AEC6, 0x401A3DFA, 0x3DEE58AD, 0xBFE3F7B9,
            0xB86FFB0C, 0xB6C5670B, 0x38AB943C, 0x3DCC21FA,
            0x3F12753A, 0x3D70F21E, 0xBF516D16, 0x4056E7BA
        };
        memcpy(&context->m_input.variable_ViewProjMtx, viewProjMtx, sizeof(viewProjMtx));
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.texture_Depth_Buffer, context->m_output.c_texture_Depth_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\MeshShaders\\Mesh\\0.bin");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Color_Buffer, context->m_output.c_texture_Color_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\MeshShaders\\Mesh\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, MeshAmplification::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        // Load, create and set VertexBuffer
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Raster\\simpleRasterVB.bin");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Raster\\simpleRasterVB.bin");
                return;
            }

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_VertexBuffer_format = DXGI_FORMAT_UNKNOWN;
            context->m_input.buffer_VertexBuffer_stride = sizeof(simpleRaster::Struct_VertexFormat);
            context->m_input.buffer_VertexBuffer_count = file.GetSize() / sizeof(simpleRaster::Struct_VertexFormat);
            context->m_input.buffer_VertexBuffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_VertexBuffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_VertexBuffer_flags, file.GetBytes(), file.GetSize(), context->GetTechniqueNameW());
        }

        // Making sure the matrix is the exact same as in the test.
        // You can save as hex in the gigi viewer
        uint32_t viewProjMtx[] = {
            0xC01A4345, 0x00000000, 0x3E0BB24D, 0x40D6375E,
            0x3C919674, 0x40193168, 0x3EA0C4AE, 0xC017F199,
            0x36BC032B, 0xB75A94C2, 0x38CF9DDA, 0x3DC955A4,
            0xBD657BFE, 0x3E0565E1, 0xBF7D69B3, 0x418827EE
        };
        memcpy(&context->m_input.variable_ViewProjMtx, viewProjMtx, sizeof(viewProjMtx));
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.texture_Depth_Buffer, context->m_output.c_texture_Depth_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\MeshShaders\\MeshAmplification\\0.bin");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Color_Buffer, context->m_output.c_texture_Color_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\MeshShaders\\MeshAmplification\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, MeshAmplificationLines::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        // Load, create and set VertexBuffer
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Raster\\simpleRasterVB.bin");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Raster\\simpleRasterVB.bin");
                return;
            }

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_VertexBuffer_format = DXGI_FORMAT_UNKNOWN;
            context->m_input.buffer_VertexBuffer_stride = sizeof(simpleRaster::Struct_VertexFormat);
            context->m_input.buffer_VertexBuffer_count = file.GetSize() / sizeof(simpleRaster::Struct_VertexFormat);
            context->m_input.buffer_VertexBuffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_VertexBuffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_VertexBuffer_flags, file.GetBytes(), file.GetSize(), context->GetTechniqueNameW());
        }

        // Making sure the matrix is the exact same as in the test.
        // You can save as hex in the gigi viewer
        uint32_t viewProjMtx[] = {
            0xBF83D523, 0x00000000, 0xC00BBE5D, 0x3FF6389F,
            0x3E86F342, 0x4019613B, 0xBDFE9F01, 0xC0864C7F,
            0xB8BC4E1F, 0x374A8ABE, 0x3831A51F, 0x3DCC99F6,
            0x3F65D77A, 0xBDF73809, 0xBED8D483, 0x3F88E888
        };
        memcpy(&context->m_input.variable_ViewProjMtx, viewProjMtx, sizeof(viewProjMtx));
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.texture_Depth_Buffer, context->m_output.c_texture_Depth_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\MeshShaders\\MeshAmplificationLines\\0.bin");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Color_Buffer, context->m_output.c_texture_Color_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\MeshShaders\\MeshAmplificationLines\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, NoVertex_NoIndex_NoInstance::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        // Making sure the matrix is the exact same as in the test.
        // You can save as hex in the gigi viewer
        uint32_t viewProjMtx[] = {
            0x401A8279, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x401A8279, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0xB8D1BC76, 0x3DCAC5B4,
            0x00000000, 0x00000000, 0x3F800000, 0x41200000
        };
        memcpy(&context->m_input.variable_ViewProjMtx, viewProjMtx, sizeof(viewProjMtx));
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.texture_Depth_Buffer, context->m_output.c_texture_Depth_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\NoVertex_NoIndex_NoInstance\\0.bin");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Color_Buffer, context->m_output.c_texture_Color_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\NoVertex_NoIndex_NoInstance\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, simpleRaster_Points::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        // Load, create and set VertexBuffer
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Raster\\simpleRasterVB.bin");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Raster\\simpleRasterVB.bin");
                return;
            }

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_VertexBuffer_format = DXGI_FORMAT_UNKNOWN;
            context->m_input.buffer_VertexBuffer_stride = sizeof(simpleRaster::Struct_VertexFormat);
            context->m_input.buffer_VertexBuffer_count = file.GetSize() / sizeof(simpleRaster::Struct_VertexFormat);
            context->m_input.buffer_VertexBuffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_VertexBuffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_VertexBuffer_flags, file.GetBytes(), file.GetSize(), context->GetTechniqueNameW());

            // Set up the vertex input layout for the vertex buffer
            context->m_input.buffer_VertexBuffer_vertexInputLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
            context->m_input.buffer_VertexBuffer_vertexInputLayout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
        }

        // Making sure the matrix is the exact same as in the test.
        // You can save as hex in the gigi viewer
        uint32_t viewProjMtx[] = {
            0xBFFD3CFB, 0x00000000, 0xBFB118D6, 0xBF7C1BFF,
            0xBDA6AEC6, 0x401A3DFA, 0x3DEE58AD, 0xBFE3F7B9,
            0xB86FFB0C, 0xB6C5670B, 0x38AB943C, 0x3DCC21FA,
            0x3F12753A, 0x3D70F21E, 0xBF516D16, 0x4056E7BA
        };
        memcpy(&context->m_input.variable_ViewProjMtx, viewProjMtx, sizeof(viewProjMtx));
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.texture_Depth_Buffer, context->m_output.c_texture_Depth_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\simpleRaster_Points\\0.bin");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Color_Buffer, context->m_output.c_texture_Color_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\simpleRaster_Points\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, simpleRaster_Lines::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        // Load, create and set VertexBuffer
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Raster\\simpleRasterVB.bin");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Raster\\simpleRasterVB.bin");
                return;
            }

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_VertexBuffer_format = DXGI_FORMAT_UNKNOWN;
            context->m_input.buffer_VertexBuffer_stride = sizeof(simpleRaster::Struct_VertexFormat);
            context->m_input.buffer_VertexBuffer_count = file.GetSize() / sizeof(simpleRaster::Struct_VertexFormat);
            context->m_input.buffer_VertexBuffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_VertexBuffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_VertexBuffer_flags, file.GetBytes(), file.GetSize(), context->GetTechniqueNameW());

            // Set up the vertex input layout for the vertex buffer
            context->m_input.buffer_VertexBuffer_vertexInputLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
            context->m_input.buffer_VertexBuffer_vertexInputLayout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
        }

        // Making sure the matrix is the exact same as in the test.
        // You can save as hex in the gigi viewer
        uint32_t viewProjMtx[] = {
            0xBFFD3CFB, 0x00000000, 0xBFB118D6, 0xBF7C1BFF,
            0xBDA6AEC6, 0x401A3DFA, 0x3DEE58AD, 0xBFE3F7B9,
            0xB86FFB0C, 0xB6C5670B, 0x38AB943C, 0x3DCC21FA,
            0x3F12753A, 0x3D70F21E, 0xBF516D16, 0x4056E7BA
        };
        memcpy(&context->m_input.variable_ViewProjMtx, viewProjMtx, sizeof(viewProjMtx));
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.texture_Depth_Buffer, context->m_output.c_texture_Depth_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\simpleRaster_Lines\\0.bin");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Color_Buffer, context->m_output.c_texture_Color_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\simpleRaster_Lines\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, simpleRaster::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        // Load, create and set VertexBuffer
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Raster\\simpleRasterVB.bin");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Raster\\simpleRasterVB.bin");
                return;
            }

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_VertexBuffer_format = DXGI_FORMAT_UNKNOWN;
            context->m_input.buffer_VertexBuffer_stride = sizeof(simpleRaster::Struct_VertexFormat);
            context->m_input.buffer_VertexBuffer_count = file.GetSize() / sizeof(simpleRaster::Struct_VertexFormat);
            context->m_input.buffer_VertexBuffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_VertexBuffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_VertexBuffer_flags, file.GetBytes(), file.GetSize(), context->GetTechniqueNameW());
        }

        // Making sure the matrix is the exact same as in the test.
        // You can save as hex in the gigi viewer
        uint32_t viewProjMtx[] = {
            0xBFFD3CFB, 0x00000000, 0xBFB118D6, 0xBF7C1BFF,
            0xBDA6AEC6, 0x401A3DFA, 0x3DEE58AD, 0xBFE3F7B9,
            0xB86FFB0C, 0xB6C5670B, 0x38AB943C, 0x3DCC21FA,
            0x3F12753A, 0x3D70F21E, 0xBF516D16, 0x4056E7BA
        };
        memcpy(&context->m_input.variable_ViewProjMtx, viewProjMtx, sizeof(viewProjMtx));
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.texture_Depth_Buffer, context->m_output.c_texture_Depth_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\simpleRaster\\0.bin");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Color_Buffer, context->m_output.c_texture_Color_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\simpleRaster\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, simpleRasterInSubgraph::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        // Load, create and set VertexBuffer
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Raster\\simpleRasterVB.bin");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Raster\\simpleRasterVB.bin");
                return;
            }

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_VB_format = DXGI_FORMAT_UNKNOWN;
            context->m_input.buffer_VB_stride = sizeof(simpleRaster::Struct_VertexFormat);
            context->m_input.buffer_VB_count = file.GetSize() / sizeof(simpleRaster::Struct_VertexFormat);
            context->m_input.buffer_VB_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_VB = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_VB_flags, file.GetBytes(), file.GetSize(), context->GetTechniqueNameW());
        }

        // Making sure the matrix is the exact same as in the test.
        // You can save as hex in the gigi viewer
        uint32_t viewProjMtx[] = {
            0xBFFD3CFB, 0x00000000, 0xBFB118D6, 0xBF7C1BFF,
            0xBDA6AEC6, 0x401A3DFA, 0x3DEE58AD, 0xBFE3F7B9,
            0xB86FFB0C, 0xB6C5670B, 0x38AB943C, 0x3DCC21FA,
            0x3F12753A, 0x3D70F21E, 0xBF516D16, 0x4056E7BA
        };
        memcpy(&context->m_input.variable_DoSimpleRaster_ViewProjMtx, viewProjMtx, sizeof(viewProjMtx));
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.texture_DoSimpleRaster_Depth_Buffer, context->m_output.c_texture_DoSimpleRaster_Depth_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\simpleRaster\\0.bin");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_DoSimpleRaster_Color_Buffer, context->m_output.c_texture_DoSimpleRaster_Color_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\simpleRaster\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, simpleRaster2::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        // Load, create and set VertexBuffer
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Raster\\simpleRasterVB.bin");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Raster\\simpleRasterVB.bin");
                return;
            }

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_VertexBuffer_format = DXGI_FORMAT_UNKNOWN;
            context->m_input.buffer_VertexBuffer_stride = sizeof(simpleRaster2::Struct_VertexFormat);
            context->m_input.buffer_VertexBuffer_count = file.GetSize() / sizeof(simpleRaster2::Struct_VertexFormat);
            context->m_input.buffer_VertexBuffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_VertexBuffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_VertexBuffer_flags, file.GetBytes(), file.GetSize(), context->GetTechniqueNameW());

            // Set up the vertex input layout for the vertex buffer
            context->m_input.buffer_VertexBuffer_vertexInputLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
            context->m_input.buffer_VertexBuffer_vertexInputLayout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
        }

        // Making sure the matrix is the exact same as in the test.
        // You can save as hex in the gigi viewer
        uint32_t viewProjMtx[] = {
            0xBFFD3CFB, 0x00000000, 0xBFB118D6, 0xBF7C1BFF,
            0xBDA6AEC6, 0x401A3DFA, 0x3DEE58AD, 0xBFE3F7B9,
            0x3F1278FA, 0x3D70F849, 0xBF517273, 0x405086AB,
            0x3F12753A, 0x3D70F21E, 0xBF516D16, 0x4056E7BA
        };
        memcpy(&context->m_input.variable_ViewProjMtx, viewProjMtx, sizeof(viewProjMtx));
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.texture_Depth_Buffer, context->m_output.c_texture_Depth_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\simpleRaster2\\0.bin");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Color_Buffer, context->m_output.c_texture_Color_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\simpleRaster2\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Stencil::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        // Load, create and set Vertex Buffer
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Raster\\StencilVB.bin");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Raster\\StencilVB.bin");
                return;
            }

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_Vertex_Buffer_format = DXGI_FORMAT_UNKNOWN;
            context->m_input.buffer_Vertex_Buffer_stride = sizeof(Stencil::Struct_VertexBuffer);
            context->m_input.buffer_Vertex_Buffer_count = file.GetSize() / sizeof(Stencil::Struct_VertexBuffer);
            context->m_input.buffer_Vertex_Buffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_Vertex_Buffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_Vertex_Buffer_flags, file.GetBytes(), file.GetSize(), context->GetTechniqueNameW());

            // Set up the vertex input layout for the vertex buffer
            context->m_input.buffer_Vertex_Buffer_vertexInputLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
            context->m_input.buffer_Vertex_Buffer_vertexInputLayout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
            context->m_input.buffer_Vertex_Buffer_vertexInputLayout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
        }

        // Making sure the matrix is the exact same as in the test.
        // You can save as hex in the gigi viewer
        uint32_t viewProjMtx[] = {
            0x401A8279, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x401A8279, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0xB8D1BC76, 0x3DCBCBDF,
            0x00000000, 0x00000000, 0x3F800000, 0x40A00000
        };
        memcpy(&context->m_input.variable_ViewProjMtx, viewProjMtx, sizeof(viewProjMtx));
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Color, context->m_output.c_texture_Color_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\Stencil\\0.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, YesVertexStruct_NoIndex_NoInstance::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        // Load, create and set Vertex Buffer
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Raster\\StencilVB.bin");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Raster\\StencilVB.bin");
                return;
            }

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_Vertex_Buffer_format = DXGI_FORMAT_UNKNOWN;
            context->m_input.buffer_Vertex_Buffer_stride = sizeof(YesVertexStruct_NoIndex_NoInstance::Struct_VertexFormat);
            context->m_input.buffer_Vertex_Buffer_count = file.GetSize() / sizeof(YesVertexStruct_NoIndex_NoInstance::Struct_VertexFormat);
            context->m_input.buffer_Vertex_Buffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_Vertex_Buffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_Vertex_Buffer_flags, file.GetBytes(), file.GetSize(), context->GetTechniqueNameW());

            // Set up the vertex input layout for the vertex buffer
            context->m_input.buffer_Vertex_Buffer_vertexInputLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
            context->m_input.buffer_Vertex_Buffer_vertexInputLayout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
            context->m_input.buffer_Vertex_Buffer_vertexInputLayout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
        }

        // Making sure the matrix is the exact same as in the test.
        // You can save as hex in the gigi viewer
        uint32_t viewProjMtx[] = {
            0x401A8279, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x401A8279, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0xB8D1BC76, 0x3DCAC5B4,
            0x00000000, 0x00000000, 0x3F800000, 0x41200000
        };
        memcpy(&context->m_input.variable_ViewProjMtx, viewProjMtx, sizeof(viewProjMtx));

        context->m_input.variable_viewMode = YesVertexStruct_NoIndex_NoInstance::ViewMode::Normal;
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.texture_Depth_Buffer, context->m_output.c_texture_Depth_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\YesVertexStruct_NoIndex_NoInstance\\0.bin");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Color_Buffer, context->m_output.c_texture_Color_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\YesVertexStruct_NoIndex_NoInstance\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, YesVertexStruct_NoIndex_YesInstanceStruct::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        // Load, create and set Vertex Buffer
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Raster\\StencilVB.bin");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Raster\\StencilVB.bin");
                return;
            }

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_Vertex_Buffer_format = DXGI_FORMAT_UNKNOWN;
            context->m_input.buffer_Vertex_Buffer_stride = sizeof(YesVertexStruct_NoIndex_YesInstanceStruct::Struct_VertexFormat);
            context->m_input.buffer_Vertex_Buffer_count = file.GetSize() / sizeof(YesVertexStruct_NoIndex_YesInstanceStruct::Struct_VertexFormat);
            context->m_input.buffer_Vertex_Buffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_Vertex_Buffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_Vertex_Buffer_flags, file.GetBytes(), file.GetSize(), context->GetTechniqueNameW());

            // Set up the vertex input layout for the vertex buffer
            context->m_input.buffer_Vertex_Buffer_vertexInputLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
            context->m_input.buffer_Vertex_Buffer_vertexInputLayout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
            context->m_input.buffer_Vertex_Buffer_vertexInputLayout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
        }

        // Load, create and set Index Buffer
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Raster\\YesVertexStruct_NoIndex_YesInstanceStruct_InstanceBuffer.bin");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Raster\\YesVertexStruct_NoIndex_YesInstanceStruct_InstanceBuffer.bin");
                return;
            }

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_Instance_Buffer_format = DXGI_FORMAT_UNKNOWN;
            context->m_input.buffer_Instance_Buffer_stride = sizeof(YesVertexStruct_NoIndex_YesInstanceStruct::Struct_InstanceBufferFormat);
            context->m_input.buffer_Instance_Buffer_count = file.GetSize() / sizeof(YesVertexStruct_NoIndex_YesInstanceStruct::Struct_InstanceBufferFormat);
            context->m_input.buffer_Instance_Buffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_Instance_Buffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_Vertex_Buffer_flags, file.GetBytes(), file.GetSize(), context->GetTechniqueNameW());

            // Set up the vertex input layout for the vertex buffer
            context->m_input.buffer_Instance_Buffer_vertexInputLayout.push_back({ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 });
            context->m_input.buffer_Instance_Buffer_vertexInputLayout.push_back({ "TEXCOORD", 2, DXGI_FORMAT_R32_FLOAT, 1, 12, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 });
        }

        // Making sure the matrix is the exact same as in the test.
        // You can save as hex in the gigi viewer
        uint32_t viewProjMtx[] = {
            0x401A8279, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x401A8279, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0xB8D1BC76, 0x3DCAC5B4,
            0x00000000, 0x00000000, 0x3F800000, 0x41200000
        };
        memcpy(&context->m_input.variable_ViewProjMtx, viewProjMtx, sizeof(viewProjMtx));

        context->m_input.variable_viewMode = YesVertexStruct_NoIndex_YesInstanceStruct::ViewMode::Normal;
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.texture_Depth_Buffer, context->m_output.c_texture_Depth_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\YesVertexStruct_NoIndex_YesInstanceStruct\\0.bin");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Color_Buffer, context->m_output.c_texture_Color_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\YesVertexStruct_NoIndex_YesInstanceStruct\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, YesVertexStruct_NoIndex_YesInstanceType::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        // Load, create and set Vertex Buffer
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Raster\\StencilVB.bin");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Raster\\StencilVB.bin");
                return;
            }

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_Vertex_Buffer_format = DXGI_FORMAT_UNKNOWN;
            context->m_input.buffer_Vertex_Buffer_stride = sizeof(YesVertexStruct_NoIndex_YesInstanceType::Struct_VertexFormat);
            context->m_input.buffer_Vertex_Buffer_count = file.GetSize() / sizeof(YesVertexStruct_NoIndex_YesInstanceType::Struct_VertexFormat);
            context->m_input.buffer_Vertex_Buffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_Vertex_Buffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_Vertex_Buffer_flags, file.GetBytes(), file.GetSize(), context->GetTechniqueNameW());

            // Set up the vertex input layout for the vertex buffer
            context->m_input.buffer_Vertex_Buffer_vertexInputLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
            context->m_input.buffer_Vertex_Buffer_vertexInputLayout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
            context->m_input.buffer_Vertex_Buffer_vertexInputLayout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
        }

        // Load, create and set Index Buffer
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Raster\\YesVertexStruct_NoIndex_YesInstanceType_InstanceBuffer.bin");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Raster\\YesVertexStruct_NoIndex_YesInstanceType_InstanceBuffer.bin");
                return;
            }

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_Instance_Buffer_format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            context->m_input.buffer_Instance_Buffer_stride = 0;
            context->m_input.buffer_Instance_Buffer_count = 3;// file.GetSize() / (sizeof(float) * 4);
            context->m_input.buffer_Instance_Buffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_Instance_Buffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_Instance_Buffer_flags, file.GetBytes(), file.GetSize(), context->GetTechniqueNameW());

            // Set up the vertex input layout for the vertex buffer
            context->m_input.buffer_Instance_Buffer_vertexInputLayout.push_back({ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 });
        }

        // Making sure the matrix is the exact same as in the test.
        // You can save as hex in the gigi viewer
        uint32_t viewProjMtx[] = {
            0x401A8279, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x401A8279, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0xB8D1BC76, 0x3DCAC5B4,
            0x00000000, 0x00000000, 0x3F800000, 0x41200000
        };
        memcpy(&context->m_input.variable_ViewProjMtx, viewProjMtx, sizeof(viewProjMtx));

        context->m_input.variable_viewMode = YesVertexStruct_NoIndex_YesInstanceType::ViewMode::Normal;
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.texture_Depth_Buffer, context->m_output.c_texture_Depth_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\YesVertexStruct_NoIndex_YesInstanceType\\0.bin");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Color_Buffer, context->m_output.c_texture_Color_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\YesVertexStruct_NoIndex_YesInstanceType\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, YesVertexStruct_YesIndex_NoInstance::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        // Load, create and set Vertex Buffer
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Raster\\YesVertexStruct_YesIndex_NoInstance_VertexBuffer.csv");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Raster\\YesVertexStruct_YesIndex_NoInstance_VertexBuffer.csv");
                return;
            }

            std::vector<float> inputTypedBufferData;

            bool result = DX12Utils::ParseCSV::ForEachValue(file.GetBytes(), true,
                [&inputTypedBufferData](int tokenIndex, const char* token)
                {
                    // skip empty tokens, caused by trailing commas
                    if (token[0] == 0)
                        return true;

                    float f;
                    sscanf(token, "%f", &f);
                    inputTypedBufferData.push_back(f);
                    return true;
                }
            );

            if (!result)
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Raster\\YesVertexStruct_YesIndex_NoInstance_VertexBuffer.csv");

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_Vertex_Buffer_format = DXGI_FORMAT_UNKNOWN;
            context->m_input.buffer_Vertex_Buffer_stride = sizeof(YesVertexStruct_YesIndex_NoInstance::Struct_VertexFormat);
            context->m_input.buffer_Vertex_Buffer_count = inputTypedBufferData.size() * sizeof(inputTypedBufferData[0]) / sizeof(YesVertexStruct_YesIndex_NoInstance::Struct_VertexFormat);
            context->m_input.buffer_Vertex_Buffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_Vertex_Buffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_Vertex_Buffer_flags, inputTypedBufferData, context->GetTechniqueNameW());

            // Set up the vertex input layout for the vertex buffer
            context->m_input.buffer_Vertex_Buffer_vertexInputLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
            context->m_input.buffer_Vertex_Buffer_vertexInputLayout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
        }

        // Load, create and set Index Buffer
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Raster\\YesVertexStruct_YesIndex_NoInstance_IndexBuffer.csv");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Raster\\YesVertexStruct_YesIndex_NoInstance_IndexBuffer.csv");
                return;
            }

            std::vector<unsigned int> inputTypedBufferData;

            bool result = DX12Utils::ParseCSV::ForEachValue(file.GetBytes(), true,
                [&inputTypedBufferData](int tokenIndex, const char* token)
                {
                    // skip empty tokens, caused by trailing commas
                    if (token[0] == 0)
                        return true;

                    unsigned int u;
                    sscanf(token, "%u", &u);
                    inputTypedBufferData.push_back(u);
                    return true;
                }
            );

            if (!result)
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Raster\\YesVertexStruct_YesIndex_NoInstance_IndexBuffer.csv");

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_Index_Buffer_format = DXGI_FORMAT_R32_UINT;
            context->m_input.buffer_Index_Buffer_stride = 0;
            context->m_input.buffer_Index_Buffer_count = inputTypedBufferData.size();
            context->m_input.buffer_Index_Buffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_Index_Buffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_Vertex_Buffer_flags, inputTypedBufferData, context->GetTechniqueNameW());
        }

        // Making sure the matrix is the exact same as in the test.
        // You can save as hex in the gigi viewer
        uint32_t viewProjMtx[] = {
            0x401A8279, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x401A8279, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0xB8D1BC76, 0x3DCAC5B4,
            0x00000000, 0x00000000, 0x3F800000, 0x41200000
        };
        memcpy(&context->m_input.variable_ViewProjMtx, viewProjMtx, sizeof(viewProjMtx));

        context->m_input.variable_viewMode = YesVertexStruct_YesIndex_NoInstance::ViewMode::UV;
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.texture_Depth_Buffer, context->m_output.c_texture_Depth_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\YesVertexStruct_YesIndex_NoInstance\\0.bin");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Color_Buffer, context->m_output.c_texture_Color_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\YesVertexStruct_YesIndex_NoInstance\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, YesVertexType_NoIndex_NoInstance::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        // Load, create and set Vertex Buffer
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Raster\\YesVertexType_NoIndex_NoInstance_VertexBuffer.csv");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Raster\\YesVertexType_NoIndex_NoInstance_VertexBuffer.csv");
                return;
            }

            std::vector<float> inputTypedBufferData;

            bool result = DX12Utils::ParseCSV::ForEachValue(file.GetBytes(), true,
                [&inputTypedBufferData](int tokenIndex, const char* token)
                {
                    // skip empty tokens, caused by trailing commas
                    if (token[0] == 0)
                        return true;

                    float f;
                    sscanf(token, "%f", &f);
                    inputTypedBufferData.push_back(f);
                    return true;
                }
            );

            if (!result)
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Raster\\YesVertexType_NoIndex_NoInstance_VertexBuffer.csv");

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_Vertex_Buffer_format = DXGI_FORMAT_R32G32B32_FLOAT;
            context->m_input.buffer_Vertex_Buffer_stride = 0;
            context->m_input.buffer_Vertex_Buffer_count = inputTypedBufferData.size() / 3;
            context->m_input.buffer_Vertex_Buffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_Vertex_Buffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_Vertex_Buffer_flags, inputTypedBufferData, context->GetTechniqueNameW());

            // Set up the vertex input layout for the vertex buffer
            context->m_input.buffer_Vertex_Buffer_vertexInputLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
        }

        // Making sure the matrix is the exact same as in the test.
        // You can save as hex in the gigi viewer
        uint32_t viewProjMtx[] = {
            0x401A8279, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x401A8279, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0xB8D1BC76, 0x3DCAC5B4,
            0x00000000, 0x00000000, 0x3F800000, 0x41200000
        };
        memcpy(&context->m_input.variable_ViewProjMtx, viewProjMtx, sizeof(viewProjMtx));
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackBinary(device, commandList, context->m_output.texture_Depth_Buffer, context->m_output.c_texture_Depth_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\YesVertexType_NoIndex_NoInstance\\0.bin");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Color_Buffer, context->m_output.c_texture_Color_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Raster\\YesVertexType_NoIndex_NoInstance\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, AnyHit::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\RayTrace\\AnyHitVB.bin");
        if (!file.Valid())
        {
            testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\RayTrace\\AnyHitVB.bin");
            return;
        }

        // Create a buffer and set the buffer data on the technique context
        context->m_input.buffer_SceneVB_format = DXGI_FORMAT_UNKNOWN;
        context->m_input.buffer_SceneVB_stride = sizeof(AnyHit::Struct_VertexBuffer);
        context->m_input.buffer_SceneVB_count = file.GetSize() / sizeof(AnyHit::Struct_VertexBuffer);
        context->m_input.buffer_SceneVB_state = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        context->m_input.buffer_SceneVB = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_SceneVB_flags, file.GetBytes(), file.GetSize(), context->GetTechniqueNameW(), context->m_input.buffer_SceneVB_state);

        // Create the tlas buffer
        context->m_input.buffer_Scene_format = DXGI_FORMAT_R32G32B32_FLOAT;
        context->m_input.buffer_Scene_stride = 0;
        context->m_input.buffer_Scene_count = context->m_input.buffer_SceneVB_count;
        context->m_input.buffer_Scene_state = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        context->CreateManagedTLAS(
            device, commandList,
            context->m_input.buffer_SceneVB, context->m_input.buffer_SceneVB_count, false,
            D3D12_RAYTRACING_GEOMETRY_FLAG_NONE,
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
            DXGI_FORMAT_R32G32B32_FLOAT, offsetof(AnyHit::Struct_VertexBuffer, position), sizeof(AnyHit::Struct_VertexBuffer),
            context->m_input.buffer_Scene_blas, context->m_input.buffer_Scene_blasSize,
            context->m_input.buffer_Scene, context->m_input.buffer_Scene_tlasSize,
            context->LogFn
        );

        // Making sure the values are the exact same as in the test.
        // You can save as hex in the gigi viewer

        uint32_t cameraPos[] = { 0xBD41BE50, 0x3EA2F44F, 0xC06DB074 };
        memcpy(&context->m_input.variable_CameraPos, cameraPos, sizeof(cameraPos));

        uint32_t invViewProjMtx[] = { 0x3ED2C305, 0x3BF14D57, 0xBEF2279C, 0xBDE0EDC9, 0x80000000, 0x3ED15999, 0x404BAC28, 0x3E24058C, 0x3D3CAEF3, 0xBD86C4CE, 0xC2148A7A, 0x3F7A307E, 0x80000000, 0x80000000, 0x411FFBE7, 0x3A83126E };
        memcpy(&context->m_input.variable_InvViewProjMtx, invViewProjMtx, sizeof(invViewProjMtx));

        context->m_input.variable_depthNearPlane = 0.0f;
    }

    if (testContext.IsFirstPostExecute(event))
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Output, context->m_output.c_texture_Output_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\RayTrace\\AnyHit\\0.png");
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, AnyHitSimple::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\RayTrace\\AnyHitVB.bin");
        if (!file.Valid())
        {
            testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\RayTrace\\AnyHitVB.bin");
            return;
        }

        // Create a buffer and set the buffer data on the technique context
        DXGI_FORMAT buffer_SceneVB_format = DXGI_FORMAT_UNKNOWN;
        unsigned int buffer_SceneVB_stride = sizeof(AnyHit::Struct_VertexBuffer);
        unsigned int buffer_SceneVB_count = file.GetSize() / sizeof(AnyHit::Struct_VertexBuffer);
        D3D12_RESOURCE_STATES buffer_SceneVB_state = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        ID3D12Resource* buffer_SceneVB = context->CreateManagedBuffer(device, commandList, D3D12_RESOURCE_FLAG_NONE, file.GetBytes(), file.GetSize(), context->GetTechniqueNameW(), buffer_SceneVB_state);

        // Create the tlas buffer
        context->m_input.buffer_Scene_format = DXGI_FORMAT_R32G32B32_FLOAT;
        context->m_input.buffer_Scene_stride = 0;
        context->m_input.buffer_Scene_count = buffer_SceneVB_count;
        context->m_input.buffer_Scene_state = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        context->CreateManagedTLAS(
            device, commandList,
            buffer_SceneVB, buffer_SceneVB_count, false,
            D3D12_RAYTRACING_GEOMETRY_FLAG_NONE,
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
            DXGI_FORMAT_R32G32B32_FLOAT, offsetof(AnyHit::Struct_VertexBuffer, position), sizeof(AnyHit::Struct_VertexBuffer),
            context->m_input.buffer_Scene_blas, context->m_input.buffer_Scene_blasSize,
            context->m_input.buffer_Scene, context->m_input.buffer_Scene_tlasSize,
            context->LogFn
        );

        // Making sure the values are the exact same as in the test.
        // You can save as hex in the gigi viewer

        uint32_t cameraPos[] = { 0xBD41BE50, 0x3EA2F44F, 0xC06DB074 };
        memcpy(&context->m_input.variable_CameraPos, cameraPos, sizeof(cameraPos));

        uint32_t invViewProjMtx[] = { 0x3ED2C305, 0x3BF14D57, 0xBEF2279C, 0xBDE0EDC9, 0x80000000, 0x3ED15999, 0x404BAC28, 0x3E24058C, 0x3D3CAEF3, 0xBD86C4CE, 0xC2148A7A, 0x3F7A307E, 0x80000000, 0x80000000, 0x411FFBE7, 0x3A83126E };
        memcpy(&context->m_input.variable_InvViewProjMtx, invViewProjMtx, sizeof(invViewProjMtx));

        context->m_input.variable_depthNearPlane = 0.0f;
    }

    if (testContext.IsFirstPostExecute(event))
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Output, context->m_output.c_texture_Output_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\RayTrace\\AnyHitSimple\\0.png");
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, IntersectionShader::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\RayTrace\\IntersectionShader.csv");
        if (!file.Valid())
        {
            testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\RayTrace\\IntersectionShader.csv");
            return;
        }

        std::vector<float> inputTypedBufferData;

        bool result = DX12Utils::ParseCSV::ForEachValue(file.GetBytes(), true,
            [&inputTypedBufferData](int tokenIndex, const char* token)
            {
                // skip empty tokens, caused by trailing commas
                if (token[0] == 0)
                    return true;

                float f;
                sscanf(token, "%f", &f);
                inputTypedBufferData.push_back(f);
                return true;
            }
        );

        if (!result)
            testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\RayTrace\\IntersectionShader.csv");

        // Create a buffer and set the buffer data on the technique context
        context->m_input.buffer_AABBsSRV_format = DXGI_FORMAT_R32_FLOAT;
        context->m_input.buffer_AABBsSRV_stride = 0;
        context->m_input.buffer_AABBsSRV_count = inputTypedBufferData.size();
        context->m_input.buffer_AABBsSRV_state = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        context->m_input.buffer_AABBsSRV = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_AABBsSRV_flags, inputTypedBufferData, context->GetTechniqueNameW(), context->m_input.buffer_AABBsSRV_state);

        // Create the tlas buffer
        context->m_input.buffer_AABBs_format = DXGI_FORMAT_R32_FLOAT;
        context->m_input.buffer_AABBs_stride = 0;
        context->m_input.buffer_AABBs_count = inputTypedBufferData.size();
        context->m_input.buffer_AABBs_state = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        context->CreateManagedTLAS(
            device, commandList,
            context->m_input.buffer_AABBsSRV, context->m_input.buffer_AABBsSRV_count, true,
            D3D12_RAYTRACING_GEOMETRY_FLAG_NONE,
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
            DXGI_FORMAT_R32_FLOAT, 0, 12,
            context->m_input.buffer_AABBs_blas, context->m_input.buffer_AABBs_blasSize,
            context->m_input.buffer_AABBs, context->m_input.buffer_AABBs_tlasSize,
            context->LogFn
        );

        // Making sure the values are the exact same as in the test.
        // You can save as hex in the gigi viewer

        uint32_t cameraPos[] = { 0x3F000000, 0x3F000000, 0xC0400000 };
        memcpy(&context->m_input.variable_CameraPos, cameraPos, sizeof(cameraPos));

        uint32_t invViewProjMtx[] = { 0x3ED413CD, 0x80000000, 0x409FFBE7, 0x3A03126F, 0x80000000, 0x3ED413CD, 0x409FFBE7, 0x3A03126F, 0x80000000, 0x80000000, 0xC1EFF9DB, 0x3F7F3B65, 0x80000000, 0x80000000, 0x411FFBE7, 0x3A83126F };
        memcpy(&context->m_input.variable_InvViewProjMtx, invViewProjMtx, sizeof(invViewProjMtx));
    }

    if (testContext.IsFirstPostExecute(event))
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Output, context->m_output.c_texture_Output_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\RayTrace\\IntersectionShader\\0.png");
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, TwoRayGens::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\RayTrace\\SimpleRT_VertexBufferSimple.csv");
        if (!file.Valid())
        {
            testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\RayTrace\\SimpleRT_VertexBufferSimple.csv");
            return;
        }

        std::vector<float> inputTypedBufferData;

        bool result = DX12Utils::ParseCSV::ForEachValue(file.GetBytes(), true,
            [&inputTypedBufferData](int tokenIndex, const char* token)
            {
                // skip empty tokens, caused by trailing commas
                if (token[0] == 0)
                    return true;

                float f;
                sscanf(token, "%f", &f);
                inputTypedBufferData.push_back(f);
                return true;
            }
        );

        if (!result)
            testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\RayTrace\\SimpleRT_VertexBufferSimple.csv");

        // Create a vertex buffer. the technique doesn't want it
        ID3D12Resource* vertexBuffer = context->CreateManagedBuffer(device, commandList, D3D12_RESOURCE_FLAG_NONE, inputTypedBufferData, context->GetTechniqueNameW(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        // Create the tlas buffer
        context->m_input.buffer_Scene_format = DXGI_FORMAT_R32_FLOAT;
        context->m_input.buffer_Scene_stride = 0;
        context->m_input.buffer_Scene_count = inputTypedBufferData.size();
        context->m_input.buffer_Scene_state = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        context->CreateManagedTLAS(
            device, commandList,
            vertexBuffer, context->m_input.buffer_Scene_count / 3, false,
            D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE,
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
            DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
            context->m_input.buffer_Scene_blas, context->m_input.buffer_Scene_blasSize,
            context->m_input.buffer_Scene, context->m_input.buffer_Scene_tlasSize,
            context->LogFn
        );

        // Making sure the values are the exact same as in the test.
        // You can save as hex in the gigi viewer

        uint32_t cameraPos[] = { 0x00000000, 0x00000000, 0xC1200000 };
        memcpy(&context->m_input.variable_cameraPos, cameraPos, sizeof(cameraPos));

        uint32_t clipToWorld[] = { 0x3ED413CD, 0x80000000, 0x80000000, 0x00000000, 0x80000000, 0x3ED413CD, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0xC2C7FAE1, 0x3F7D70A3, 0x80000000, 0x80000000, 0x411FFBE7, 0x3A83126F };
        memcpy(&context->m_input.variable_clipToWorld, clipToWorld, sizeof(clipToWorld));

        context->m_input.variable_depthNearPlane = 0.0f;

        // Load the BlueChannel texture
        context->m_input.texture_BlueChannel_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        context->m_input.texture_BlueChannel_state = D3D12_RESOURCE_STATE_COPY_DEST;
        context->m_input.texture_BlueChannel = context->CreateManagedTextureFromFile(
            device,
            commandList,
            context->m_input.texture_BlueChannel_flags,
            context->m_input.texture_BlueChannel_format,
            DX12Utils::ResourceType::Texture2D,
            "..\\..\\..\\Techniques\\UnitTests\\cabinsmall.png",
            true,
            context->m_input.texture_BlueChannel_size,
            context->GetTechniqueNameW()
        );
    }

    if (testContext.IsFirstPostExecute(event))
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Texture, context->m_output.c_texture_Texture_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\RayTrace\\TwoRayGens\\0.png");
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, TwoRayGensSubgraph::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\RayTrace\\SimpleRT_VertexBufferSimple.csv");
        if (!file.Valid())
        {
            testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\RayTrace\\SimpleRT_VertexBufferSimple.csv");
            return;
        }

        std::vector<float> inputTypedBufferData;

        bool result = DX12Utils::ParseCSV::ForEachValue(file.GetBytes(), true,
            [&inputTypedBufferData](int tokenIndex, const char* token)
            {
                // skip empty tokens, caused by trailing commas
                if (token[0] == 0)
                    return true;

                float f;
                sscanf(token, "%f", &f);
                inputTypedBufferData.push_back(f);
                return true;
            }
        );

        if (!result)
            testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\RayTrace\\SimpleRT_VertexBufferSimple.csv");

        // Create a vertex buffer. the technique doesn't want it
        ID3D12Resource* vertexBuffer = context->CreateManagedBuffer(device, commandList, D3D12_RESOURCE_FLAG_NONE, inputTypedBufferData, context->GetTechniqueNameW(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        // Create the tlas buffer
        context->m_input.buffer_Scene_format = DXGI_FORMAT_R32_FLOAT;
        context->m_input.buffer_Scene_stride = 0;
        context->m_input.buffer_Scene_count = inputTypedBufferData.size();
        context->m_input.buffer_Scene_state = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        context->CreateManagedTLAS(
            device, commandList,
            vertexBuffer, context->m_input.buffer_Scene_count / 3, false,
            D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE,
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
            DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
            context->m_input.buffer_Scene_blas, context->m_input.buffer_Scene_blasSize,
            context->m_input.buffer_Scene, context->m_input.buffer_Scene_tlasSize,
            context->LogFn
        );

        // Making sure the values are the exact same as in the test.
        // You can save as hex in the gigi viewer

        uint32_t cameraPos[] = { 0x00000000, 0x00000000, 0xC1200000 };
        memcpy(&context->m_input.variable_cameraPos, cameraPos, sizeof(cameraPos));

        uint32_t clipToWorld[] = { 0x3ED413CD, 0x80000000, 0x80000000, 0x00000000, 0x80000000, 0x3ED413CD, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0xC2C7FAE1, 0x3F7D70A3, 0x80000000, 0x80000000, 0x411FFBE7, 0x3A83126F };
        memcpy(&context->m_input.variable_clipToWorld, clipToWorld, sizeof(clipToWorld));

        context->m_input.variable_depthNearPlane = 0.0f;

        // Load the BlueChannel texture
        context->m_input.texture_BlueChannel_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        context->m_input.texture_BlueChannel_state = D3D12_RESOURCE_STATE_COPY_DEST;
        context->m_input.texture_BlueChannel = context->CreateManagedTextureFromFile(
            device,
            commandList,
            context->m_input.texture_BlueChannel_flags,
            context->m_input.texture_BlueChannel_format,
            DX12Utils::ResourceType::Texture2D,
            "..\\..\\..\\Techniques\\UnitTests\\cabinsmall.png",
            true,
            context->m_input.texture_BlueChannel_size,
            context->GetTechniqueNameW()
        );
    }

    if (testContext.IsFirstPostExecute(event))
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Texture, context->m_output.c_texture_Texture_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\RayTrace\\TwoRayGensSubgraph\\0.png");
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, simpleRT::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\RayTrace\\SimpleRT_VertexBuffer.csv");
        if (!file.Valid())
        {
            testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\RayTrace\\SimpleRT_VertexBuffer.csv");
            return;
        }

        std::vector<float> inputTypedBufferData;

        bool result = DX12Utils::ParseCSV::ForEachValue(file.GetBytes(), true,
            [&inputTypedBufferData](int tokenIndex, const char* token)
            {
                // skip empty tokens, caused by trailing commas
                if (token[0] == 0)
                    return true;

                float f;
                sscanf(token, "%f", &f);
                inputTypedBufferData.push_back(f);
                return true;
            }
        );

        if (!result)
            testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\RayTrace\\SimpleRT_VertexBuffer.csv");

        // Create a managed buffer to hold our vertex buffer. The technique doesn't want access to it
        ID3D12Resource* vertexBuffer = context->CreateManagedBuffer(device, commandList, D3D12_RESOURCE_FLAG_NONE, inputTypedBufferData, context->GetTechniqueNameW(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        // Create the tlas buffer
        context->m_input.buffer_Scene_format = DXGI_FORMAT_R32_FLOAT;
        context->m_input.buffer_Scene_stride = sizeof(simpleRT::Struct_VertexBuffer);
        context->m_input.buffer_Scene_count = inputTypedBufferData.size() * sizeof(float) / context->m_input.buffer_Scene_stride;
        context->m_input.buffer_Scene_state = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        context->CreateManagedTLAS(
            device, commandList,
            vertexBuffer, context->m_input.buffer_Scene_count, false,
            D3D12_RAYTRACING_GEOMETRY_FLAG_NONE,
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
            DXGI_FORMAT_R32G32B32_FLOAT, offsetof(simpleRT::Struct_VertexBuffer, Position), sizeof(simpleRT::Struct_VertexBuffer),
            context->m_input.buffer_Scene_blas, context->m_input.buffer_Scene_blasSize,
            context->m_input.buffer_Scene, context->m_input.buffer_Scene_tlasSize,
            context->LogFn
        );


        // Making sure the values are the exact same as in the test.
        // You can save as hex in the gigi viewer

        uint32_t clipToWorld[] = { 0x3F3C8362, 0x80000000, 0x80000000, 0x00000000, 0x80000000, 0x3ED413CF, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0xC2C7FAE2, 0x3F7D70A5, 0x80000000, 0x80000000, 0x411FFBE8, 0x3A83126F };
        memcpy(&context->m_input.variable_clipToWorld, clipToWorld, sizeof(clipToWorld));

        context->m_input.variable_cameraPos[0] = 0.0f;
        context->m_input.variable_cameraPos[1] = 0.0f;
        context->m_input.variable_cameraPos[2] = -10.0f;

        context->m_input.variable_depthNearPlane = 0.1f;

        context->m_input.variable_hitColor[0] = 0.0f;
        context->m_input.variable_hitColor[1] = 1.0f;
        context->m_input.variable_hitColor[2] = 0.0f;

        context->m_input.variable_missColor[0] = 1.0f;
        context->m_input.variable_missColor[1] = 0.0f;
        context->m_input.variable_missColor[2] = 0.0f;

        context->m_input.variable_enabled = true;
    }

    if (testContext.IsFirstPostExecute(event))
    {
        // Note: not verifying the constant buffer. if that is wrong, the rendering will change
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Texture, context->m_output.c_texture_Texture_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\RayTrace\\simpleRT\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, simpleRT_inline::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\RayTrace\\simpleRT_inlineVB.bin");
        if (!file.Valid())
        {
            testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\RayTrace\\simpleRT_inlineVB.bin");
            return;
        }

        // Create a buffer and set the buffer data on the technique context
        context->m_input.buffer_Scene_VB_format = DXGI_FORMAT_UNKNOWN;
        context->m_input.buffer_Scene_VB_stride = sizeof(simpleRT_inline::Struct_VertexBuffer);
        context->m_input.buffer_Scene_VB_count = file.GetSize() / sizeof(simpleRT_inline::Struct_VertexBuffer);
        context->m_input.buffer_Scene_VB_state = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        context->m_input.buffer_Scene_VB = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_Scene_VB_flags, file.GetBytes(), file.GetSize(), context->GetTechniqueNameW(), context->m_input.buffer_Scene_VB_state);

        // Create the tlas buffer
        context->m_input.buffer_Scene_format = DXGI_FORMAT_R32_FLOAT;
        context->m_input.buffer_Scene_stride = sizeof(simpleRT_inline::Struct_VertexBuffer);
        context->m_input.buffer_Scene_count = context->m_input.buffer_Scene_VB_count;
        context->m_input.buffer_Scene_state = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        context->CreateManagedTLAS(
            device, commandList,
            context->m_input.buffer_Scene_VB, context->m_input.buffer_Scene_count, false,
            D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE,
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
            DXGI_FORMAT_R32G32B32_FLOAT, offsetof(simpleRT_inline::Struct_VertexBuffer, Position), sizeof(simpleRT_inline::Struct_VertexBuffer),
            context->m_input.buffer_Scene_blas, context->m_input.buffer_Scene_blasSize,
            context->m_input.buffer_Scene, context->m_input.buffer_Scene_tlasSize,
            context->LogFn
        );


        // Making sure the values are the exact same as in the test.
        // You can save as hex in the gigi viewer

        uint32_t cameraPos[] = { 0xC00CCCCD, 0x3F09FBE7, 0x402322D1 };
        memcpy(&context->m_input.variable_cameraPos, cameraPos, sizeof(cameraPos));

        uint32_t clipToWorld[] = { 0xBEADC77F, 0xBC6560B9, 0xC1AFFB7F, 0x3F11EBC7, 0xB124B189, 0x3ED3B556, 0x40AC7676, 0x3D73BB46, 0xBE732041, 0x3CA3F3D5, 0x41CBE64D, 0xBF50C0AA, 0x2D8476D4, 0xAC1295F9, 0x411FFBE8, 0x3A83126F };
        memcpy(&context->m_input.variable_clipToWorld, clipToWorld, sizeof(clipToWorld));

        context->m_input.variable_depthNearPlane = 0.0f;

        context->m_input.variable_hitColor[0] = 0.0f;
        context->m_input.variable_hitColor[1] = 1.0f;
        context->m_input.variable_hitColor[2] = 0.0f;

        context->m_input.variable_missColor[0] = 0.2f;
        context->m_input.variable_missColor[1] = 0.2f;
        context->m_input.variable_missColor[2] = 0.2f;

        context->m_input.variable_enabled = true;
    }

    if (testContext.IsFirstPostExecute(event))
    {
        // Note: not verifying the constant buffer. if that is wrong, the rendering will change
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Texture, context->m_output.c_texture_Texture_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\RayTrace\\simpleRT_inline\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, SubInSub::Context* context, UnitTestEvent event)
{
    static ID3D12Resource* texture_ImportedTexture_InitialState = nullptr;

    if (testContext.IsFirstPreExecute(event))
    {
        context->m_input.texture_Input_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        context->m_input.texture_Input_state = D3D12_RESOURCE_STATE_COPY_DEST;
        context->m_input.texture_Input = context->CreateManagedTextureFromFile(
            device,
            commandList,
            context->m_input.texture_Input_flags,
            context->m_input.texture_Input_format,
            DX12Utils::ResourceType::Texture2D,
            "..\\..\\..\\Techniques\\UnitTests\\cabinsmall.png",
            true,
            context->m_input.texture_Input_size,
            context->GetTechniqueNameW()
        );

        texture_ImportedTexture_InitialState = context->CreateManagedTextureFromFile(
            device,
            commandList,
            context->m_input.texture_Input_flags,
            context->m_input.texture_Input_format,
            DX12Utils::ResourceType::Texture2D,
            "..\\..\\..\\Techniques\\UnitTests\\cabinsmall.png",
            true,
            context->m_input.texture_Input_size,
            L"texture_ImportedTexture_InitialState",
            D3D12_RESOURCE_STATE_COPY_SOURCE
        );
    }

    // Reset imported texture to it's starting state every frame
    if (event == UnitTestEvent::PreExecute)
    {
        commandList->CopyResource(context->m_input.texture_Input, texture_ImportedTexture_InitialState);
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Inner1_Inner2_Output, context->m_output.c_texture_Inner1_Inner2_Output_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\SubGraph\\SubInSub\\0.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, SubGraphLoops::Context* context, UnitTestEvent event)
{
    static ID3D12Resource* texture_ImportedTexture_InitialState = nullptr;

    if (testContext.IsFirstPreExecute(event))
    {
        context->m_input.texture_Input_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        context->m_input.texture_Input_state = D3D12_RESOURCE_STATE_COPY_DEST;
        context->m_input.texture_Input = context->CreateManagedTextureFromFile(
            device,
            commandList,
            context->m_input.texture_Input_flags,
            context->m_input.texture_Input_format,
            DX12Utils::ResourceType::Texture2D,
            "..\\..\\..\\Techniques\\UnitTests\\cabinsmall.png",
            true,
            context->m_input.texture_Input_size,
            context->GetTechniqueNameW()
        );

        texture_ImportedTexture_InitialState = context->CreateManagedTextureFromFile(
            device,
            commandList,
            context->m_input.texture_Input_flags,
            context->m_input.texture_Input_format,
            DX12Utils::ResourceType::Texture2D,
            "..\\..\\..\\Techniques\\UnitTests\\cabinsmall.png",
            true,
            context->m_input.texture_Input_size,
            L"texture_ImportedTexture_InitialState",
            D3D12_RESOURCE_STATE_COPY_SOURCE
        );
    }

    // Reset imported texture to it's starting state every frame
    if (event == UnitTestEvent::PreExecute)
    {
        commandList->CopyResource(context->m_input.texture_Input, texture_ImportedTexture_InitialState);
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_FilterSub_Iteration_4_Output, context->m_output.c_texture_FilterSub_Iteration_4_Output_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\SubGraph\\SubGraphLoops\\0.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, SubGraphTest::Context* context, UnitTestEvent event)
{
    static ID3D12Resource* texture_ImportedTexture_InitialState = nullptr;

    if (testContext.IsFirstPreExecute(event))
    {
        context->m_input.texture_Test_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        context->m_input.texture_Test_state = D3D12_RESOURCE_STATE_COPY_DEST;
        context->m_input.texture_Test = context->CreateManagedTextureFromFile(
            device,
            commandList,
            context->m_input.texture_Test_flags,
            context->m_input.texture_Test_format,
            DX12Utils::ResourceType::Texture2D,
            "..\\..\\..\\Techniques\\UnitTests\\cabinsmall.png",
            true,
            context->m_input.texture_Test_size,
            context->GetTechniqueNameW()
        );

        texture_ImportedTexture_InitialState = context->CreateManagedTextureFromFile(
            device,
            commandList,
            context->m_input.texture_Test_flags,
            context->m_input.texture_Test_format,
            DX12Utils::ResourceType::Texture2D,
            "..\\..\\..\\Techniques\\UnitTests\\cabinsmall.png",
            true,
            context->m_input.texture_Test_size,
            L"texture_ImportedTexture_InitialState",
            D3D12_RESOURCE_STATE_COPY_SOURCE
        );
    }

    // Reset imported texture to it's starting state every frame
    if (event == UnitTestEvent::PreExecute)
    {
        commandList->CopyResource(context->m_input.texture_Test, texture_ImportedTexture_InitialState);
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Inner_Exported_Tex, context->m_output.c_texture_Inner_Exported_Tex_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\SubGraph\\SubGraphTest\\0.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Texture2DArrayRW_CS::Context* context, UnitTestEvent event)
{
    static ID3D12Resource* texture_ImportedTexture_InitialState = nullptr;

    if (testContext.IsFirstPreExecute(event))
    {
        // Load and create ImportedTexture
        {
            context->m_input.texture_ImportedTexture_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            context->m_input.texture_ImportedTexture_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedTexture = context->CreateManagedTextureFromFile(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                DX12Utils::ResourceType::Texture2DArray,
                "..\\..\\..\\Techniques\\UnitTests\\Textures\\Image%i.png",
                true,
                context->m_input.texture_ImportedTexture_size,
                context->GetTechniqueNameW()
            );
        }

        // Store off the initial state of the imported texture as a managed resource so we can reset it every frame
        {
            texture_ImportedTexture_InitialState = context->CreateManagedTextureFromFile(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                DX12Utils::ResourceType::Texture2DArray,
                "..\\..\\..\\Techniques\\UnitTests\\Textures\\Image%i.png",
                true,
                context->m_input.texture_ImportedTexture_size,
                L"texture_ImportedTexture_InitialState",
                D3D12_RESOURCE_STATE_COPY_SOURCE
            );
        }

        // Load and create ImportedColor
        {
            static const unsigned char clearValue[4] = { 128, 128, 128, 255 };

            context->m_input.texture_ImportedColor_format = DXGI_FORMAT_R8G8B8A8_UNORM;
            context->m_input.texture_ImportedColor_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedColor_size[0] = 64;
            context->m_input.texture_ImportedColor_size[1] = 64;
            context->m_input.texture_ImportedColor_size[2] = 3;
            context->m_input.texture_ImportedColor_numMips = 1;
            context->m_input.texture_ImportedColor = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_ImportedColor_flags,
                context->m_input.texture_ImportedColor_format,
                context->m_input.texture_ImportedColor_size,
                context->m_input.texture_ImportedColor_numMips,
                DX12Utils::ResourceType::Texture2DArray,
                (void*)clearValue, sizeof(clearValue),
                context->GetTechniqueNameW()
            );
        }
    }

    // Reset imported texture to it's starting state every frame
    if (event == UnitTestEvent::PreExecute)
    {
        commandList->CopyResource(context->m_input.texture_ImportedTexture, texture_ImportedTexture_InitialState);
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DArrayRW_CS\\0_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 1, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DArrayRW_CS\\1_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 2, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DArrayRW_CS\\2_0.png");

        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DArrayRW_CS\\0_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 1, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DArrayRW_CS\\1_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 2, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DArrayRW_CS\\2_1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Texture2DArrayRW_PS::Context* context, UnitTestEvent event)
{
    static ID3D12Resource* texture_ImportedTexture_InitialState = nullptr;

    if (testContext.IsFirstPreExecute(event))
    {
        // Load and create ImportedTexture
        {
            context->m_input.texture_ImportedTexture_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            context->m_input.texture_ImportedTexture_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedTexture = context->CreateManagedTextureFromFile(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                DX12Utils::ResourceType::Texture2DArray,
                "..\\..\\..\\Techniques\\UnitTests\\Textures\\Image%i.png",
                true,
                context->m_input.texture_ImportedTexture_size,
                context->GetTechniqueNameW()
            );
        }

        // Store off the initial state of the imported texture as a managed resource so we can reset it every frame
        {
            texture_ImportedTexture_InitialState = context->CreateManagedTextureFromFile(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                DX12Utils::ResourceType::Texture2DArray,
                "..\\..\\..\\Techniques\\UnitTests\\Textures\\Image%i.png",
                true,
                context->m_input.texture_ImportedTexture_size,
                L"texture_ImportedTexture_InitialState",
                D3D12_RESOURCE_STATE_COPY_SOURCE
            );
        }

        // Load and create ImportedColor
        {
            static const unsigned char clearValue[4] = { 128, 128, 128, 255 };

            context->m_input.texture_ImportedColor_format = DXGI_FORMAT_R8G8B8A8_UNORM;
            context->m_input.texture_ImportedColor_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedColor_size[0] = 64;
            context->m_input.texture_ImportedColor_size[1] = 64;
            context->m_input.texture_ImportedColor_size[2] = 3;
            context->m_input.texture_ImportedColor_numMips = 1;
            context->m_input.texture_ImportedColor = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_ImportedColor_flags,
                context->m_input.texture_ImportedColor_format,
                context->m_input.texture_ImportedColor_size,
                context->m_input.texture_ImportedColor_numMips,
                DX12Utils::ResourceType::Texture2DArray,
                (void*)clearValue, sizeof(clearValue),
                context->GetTechniqueNameW()
            );
        }

        // Load and create Color
        {
            static const unsigned char clearValue[4] = { 255, 255, 255, 255 };

            context->m_input.texture_Color_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            context->m_input.texture_Color_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_Color_size[0] = 64;
            context->m_input.texture_Color_size[1] = 64;
            context->m_input.texture_Color_size[2] = 3;
            context->m_input.texture_Color_numMips = 1;
            context->m_input.texture_Color = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_Color_flags,
                context->m_input.texture_Color_format,
                context->m_input.texture_Color_size,
                context->m_input.texture_Color_numMips,
                DX12Utils::ResourceType::Texture2DArray,
                (void*)clearValue, sizeof(clearValue),
                context->GetTechniqueNameW()
            );
        }
    }

    // Reset imported texture to it's starting state every frame
    if (event == UnitTestEvent::PreExecute)
    {
        commandList->CopyResource(context->m_input.texture_ImportedTexture, texture_ImportedTexture_InitialState);
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DArrayRW_PS\\0_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 1, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DArrayRW_PS\\1_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 2, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DArrayRW_PS\\2_0.png");

        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DArrayRW_PS\\0_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 1, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DArrayRW_PS\\1_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 2, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DArrayRW_PS\\2_1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Texture2DArrayRW_RGS::Context* context, UnitTestEvent event)
{
    static ID3D12Resource* texture_ImportedTexture_InitialState = nullptr;

    if (testContext.IsFirstPreExecute(event))
    {
        // Load and create ImportedColor
        {
            static const unsigned char clearValue[4] = { 255, 64, 128, 255 };

            context->m_input.texture_ImportedColor_format = DXGI_FORMAT_R8G8B8A8_UNORM;
            context->m_input.texture_ImportedColor_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedColor_size[0] = 64;
            context->m_input.texture_ImportedColor_size[1] = 64;
            context->m_input.texture_ImportedColor_size[2] = 3;
            context->m_input.texture_ImportedColor_numMips = 1;
            context->m_input.texture_ImportedColor = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_ImportedColor_flags,
                context->m_input.texture_ImportedColor_format,
                context->m_input.texture_ImportedColor_size,
                context->m_input.texture_ImportedColor_numMips,
                DX12Utils::ResourceType::Texture2DArray,
                (void*)clearValue, sizeof(clearValue),
                context->GetTechniqueNameW()
            );
        }

        // Load and create ImportedTexture
        {
            context->m_input.texture_ImportedTexture_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            context->m_input.texture_ImportedTexture_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedTexture = context->CreateManagedTextureFromFile(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                DX12Utils::ResourceType::Texture2DArray,
                "..\\..\\..\\Techniques\\UnitTests\\Textures\\Image%i.png",
                true,
                context->m_input.texture_ImportedTexture_size,
                context->GetTechniqueNameW()
            );
        }

        // Store off the initial state of the imported texture as a managed resource so we can reset it every frame
        {
            texture_ImportedTexture_InitialState = context->CreateManagedTextureFromFile(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                DX12Utils::ResourceType::Texture2DArray,
                "..\\..\\..\\Techniques\\UnitTests\\Textures\\Image%i.png",
                true,
                context->m_input.texture_ImportedTexture_size,
                L"texture_ImportedTexture_InitialState",
                D3D12_RESOURCE_STATE_COPY_SOURCE
            );
        }
    }

    // Reset imported texture to it's starting state every frame
    if (event == UnitTestEvent::PreExecute)
    {
        commandList->CopyResource(context->m_input.texture_ImportedTexture, texture_ImportedTexture_InitialState);
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DArrayRW_RGS\\0_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 1, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DArrayRW_RGS\\1_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 2, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DArrayRW_RGS\\2_0.png");

        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DArrayRW_RGS\\0_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 1, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DArrayRW_RGS\\1_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 2, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DArrayRW_RGS\\2_1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Texture2DRW_CS::Context* context, UnitTestEvent event)
{
    static ID3D12Resource* texture_ImportedTexture_InitialState = nullptr;

    if (testContext.IsFirstPreExecute(event))
    {
        // Load and create ImportedColor
        {
            unsigned char clearValue[4] = { 255, 64, 128, 255 };

            context->m_input.texture_ImportedColor_format = DXGI_FORMAT_R8G8B8A8_UNORM;
            context->m_input.texture_ImportedColor_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedColor_size[0] = 512;
            context->m_input.texture_ImportedColor_size[1] = 512;
            context->m_input.texture_ImportedColor_size[2] = 1;
            context->m_input.texture_ImportedColor_numMips = 1;
            context->m_input.texture_ImportedColor = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_ImportedColor_flags,
                context->m_input.texture_ImportedColor_format,
                context->m_input.texture_ImportedColor_size,
                context->m_input.texture_ImportedColor_numMips,
                DX12Utils::ResourceType::Texture2D,
                (void*)clearValue, sizeof(clearValue),
                context->GetTechniqueNameW()
            );
        }

        // Load and create ImportedTexture
        {
            static const unsigned char clearValue[4] = { 255, 255, 255, 255 };

            context->m_input.texture_ImportedTexture_format = DXGI_FORMAT_R8G8B8A8_UNORM;
            context->m_input.texture_ImportedTexture_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedTexture_size[0] = 512;
            context->m_input.texture_ImportedTexture_size[1] = 512;
            context->m_input.texture_ImportedTexture_size[2] = 1;
            context->m_input.texture_ImportedTexture_numMips = 1;
            context->m_input.texture_ImportedTexture = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                context->m_input.texture_ImportedTexture_size,
                context->m_input.texture_ImportedTexture_numMips,
                DX12Utils::ResourceType::Texture2D,
                (void*)clearValue, sizeof(clearValue),
                context->GetTechniqueNameW()
            );
        }

        // Store off the initial state of the imported texture as a managed resource so we can reset it every frame
        {
            static const unsigned char clearValue[4] = { 255, 255, 255, 255 };

            texture_ImportedTexture_InitialState = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                context->m_input.texture_ImportedTexture_size,
                context->m_input.texture_ImportedTexture_numMips,
                DX12Utils::ResourceType::Texture2D,
                (void*)clearValue, sizeof(clearValue),
                L"texture_ImportedTexture_InitialState",
                D3D12_RESOURCE_STATE_COPY_SOURCE
            );
        }
    }

    // Reset imported texture to it's starting state every frame
    if (event == UnitTestEvent::PreExecute)
    {
        commandList->CopyResource(context->m_input.texture_ImportedTexture, texture_ImportedTexture_InitialState);
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DRW_CS\\0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DRW_CS\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Texture2DRW_PS::Context* context, UnitTestEvent event)
{
    static ID3D12Resource* texture_ImportedTexture_InitialState = nullptr;

    if (testContext.IsFirstPreExecute(event))
    {
        // Load and create Color
        {
            unsigned char clearValue[4] = { 255, 64, 128, 255 };

            context->m_input.texture_Color_format = DXGI_FORMAT_R8G8B8A8_UNORM;
            context->m_input.texture_Color_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_Color_size[0] = 512;
            context->m_input.texture_Color_size[1] = 512;
            context->m_input.texture_Color_size[2] = 1;
            context->m_input.texture_Color_numMips = 1;
            context->m_input.texture_Color = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_Color_flags,
                context->m_input.texture_Color_format,
                context->m_input.texture_Color_size,
                context->m_input.texture_Color_numMips,
                DX12Utils::ResourceType::Texture2D,
                (void*)clearValue, sizeof(clearValue),
                context->GetTechniqueNameW()
            );
        }

        // Load and create ImportedColor
        {
            unsigned char clearValue[4] = { 255, 64, 128, 255 };
            for (int i = 0; i < 3; ++i)
            {
                double d = double(clearValue[i]) / 255.0;
                d = DX12Utils::LinearTosRGB(d);
                clearValue[i] = min(max(d * 256.0, 0.0), 255.0);
            }

            context->m_input.texture_ImportedColor_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            context->m_input.texture_ImportedColor_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedColor_size[0] = 512;
            context->m_input.texture_ImportedColor_size[1] = 512;
            context->m_input.texture_ImportedColor_size[2] = 1;
            context->m_input.texture_ImportedColor_numMips = 1;
            context->m_input.texture_ImportedColor = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_ImportedColor_flags,
                context->m_input.texture_ImportedColor_format,
                context->m_input.texture_ImportedColor_size,
                context->m_input.texture_ImportedColor_numMips,
                DX12Utils::ResourceType::Texture2D,
                (void*)clearValue, sizeof(clearValue),
                context->GetTechniqueNameW()
            );
        }

        // Load and create ImportedTexture
        {
            static const unsigned char clearValue[4] = { 255, 255, 255, 255 };

            context->m_input.texture_ImportedTexture_format = DXGI_FORMAT_R8G8B8A8_UNORM;
            context->m_input.texture_ImportedTexture_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedTexture_size[0] = 512;
            context->m_input.texture_ImportedTexture_size[1] = 512;
            context->m_input.texture_ImportedTexture_size[2] = 1;
            context->m_input.texture_ImportedTexture_numMips = 1;
            context->m_input.texture_ImportedTexture = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                context->m_input.texture_ImportedTexture_size,
                context->m_input.texture_ImportedTexture_numMips,
                DX12Utils::ResourceType::Texture2D,
                (void*)clearValue, sizeof(clearValue),
                context->GetTechniqueNameW()
            );
        }

        // Store off the initial state of the imported texture as a managed resource so we can reset it every frame
        {
            static const unsigned char clearValue[4] = { 255, 255, 255, 255 };

            texture_ImportedTexture_InitialState = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                context->m_input.texture_ImportedTexture_size,
                context->m_input.texture_ImportedTexture_numMips,
                DX12Utils::ResourceType::Texture2D,
                (void*)clearValue, sizeof(clearValue),
                L"texture_ImportedTexture_InitialState",
                D3D12_RESOURCE_STATE_COPY_SOURCE
            );
        }
    }

    // Reset imported texture to it's starting state every frame
    if (event == UnitTestEvent::PreExecute)
    {
        commandList->CopyResource(context->m_input.texture_ImportedTexture, texture_ImportedTexture_InitialState);
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DRW_PS\\0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DRW_PS\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Texture2DRW_RGS::Context* context, UnitTestEvent event)
{
    static ID3D12Resource* texture_ImportedTexture_InitialState = nullptr;

    if (testContext.IsFirstPreExecute(event))
    {
        // Load and create ImportedColor
        {
            unsigned char clearValue[4] = { 255, 64, 128, 255 };

            context->m_input.texture_ImportedColor_format = DXGI_FORMAT_R8G8B8A8_UNORM;
            context->m_input.texture_ImportedColor_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedColor_size[0] = 512;
            context->m_input.texture_ImportedColor_size[1] = 512;
            context->m_input.texture_ImportedColor_size[2] = 1;
            context->m_input.texture_ImportedColor_numMips = 1;
            context->m_input.texture_ImportedColor = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_ImportedColor_flags,
                context->m_input.texture_ImportedColor_format,
                context->m_input.texture_ImportedColor_size,
                context->m_input.texture_ImportedColor_numMips,
                DX12Utils::ResourceType::Texture2D,
                (void*)clearValue, sizeof(clearValue),
                context->GetTechniqueNameW()
            );
        }

        // Load and create ImportedTexture
        {
            static const unsigned char clearValue[4] = { 255, 255, 255, 255 };

            context->m_input.texture_ImportedTexture_format = DXGI_FORMAT_R8G8B8A8_UNORM;
            context->m_input.texture_ImportedTexture_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedTexture_size[0] = 512;
            context->m_input.texture_ImportedTexture_size[1] = 512;
            context->m_input.texture_ImportedTexture_size[2] = 1;
            context->m_input.texture_ImportedTexture_numMips = 1;
            context->m_input.texture_ImportedTexture = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                context->m_input.texture_ImportedTexture_size,
                context->m_input.texture_ImportedTexture_numMips,
                DX12Utils::ResourceType::Texture2D,
                (void*)clearValue, sizeof(clearValue),
                context->GetTechniqueNameW()
            );
        }

        // Store off the initial state of the imported texture as a managed resource so we can reset it every frame
        {
            static const unsigned char clearValue[4] = { 255, 255, 255, 255 };

            texture_ImportedTexture_InitialState = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                context->m_input.texture_ImportedTexture_size,
                context->m_input.texture_ImportedTexture_numMips,
                DX12Utils::ResourceType::Texture2D,
                (void*)clearValue, sizeof(clearValue),
                L"texture_ImportedTexture_InitialState",
                D3D12_RESOURCE_STATE_COPY_SOURCE
            );
        }
    }

    // Reset imported texture to it's starting state every frame
    if (event == UnitTestEvent::PreExecute)
    {
        commandList->CopyResource(context->m_input.texture_ImportedTexture, texture_ImportedTexture_InitialState);
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DRW_RGS\\0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture2DRW_RGS\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Texture3DRW_CS::Context* context, UnitTestEvent event)
{
    static ID3D12Resource* texture_ImportedTexture_InitialState = nullptr;

    if (testContext.IsFirstPreExecute(event))
    {
        // Load and create ImportedTexture
        {
            context->m_input.texture_ImportedTexture_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            context->m_input.texture_ImportedTexture_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedTexture = context->CreateManagedTextureFromFile(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                DX12Utils::ResourceType::Texture3D,
                "..\\..\\..\\Techniques\\UnitTests\\Textures\\Image%i.png",
                true,
                context->m_input.texture_ImportedTexture_size,
                context->GetTechniqueNameW()
            );
        }

        // Store off the initial state of the imported texture as a managed resource so we can reset it every frame
        {
            texture_ImportedTexture_InitialState = context->CreateManagedTextureFromFile(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                DX12Utils::ResourceType::Texture3D,
                "..\\..\\..\\Techniques\\UnitTests\\Textures\\Image%i.png",
                true,
                context->m_input.texture_ImportedTexture_size,
                L"texture_ImportedTexture_InitialState",
                D3D12_RESOURCE_STATE_COPY_SOURCE
            );
        }

        // Load and create ImportedColor
        {
            unsigned char clearValue[4] = { 128, 128, 128, 255 };

            for (int i = 0; i < 3; ++i)
            {
                double d = double(clearValue[i]) / 255.0;
                d = DX12Utils::LinearTosRGB(d);
                clearValue[i] = min(max(d * 256.0, 0.0), 255.0);
            }

            context->m_input.texture_ImportedColor_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            context->m_input.texture_ImportedColor_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedColor_size[0] = 64;
            context->m_input.texture_ImportedColor_size[1] = 64;
            context->m_input.texture_ImportedColor_size[2] = 3;
            context->m_input.texture_ImportedColor_numMips = 1;
            context->m_input.texture_ImportedColor = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_ImportedColor_flags,
                context->m_input.texture_ImportedColor_format,
                context->m_input.texture_ImportedColor_size,
                context->m_input.texture_ImportedColor_numMips,
                DX12Utils::ResourceType::Texture3D,
                (void*)clearValue, sizeof(clearValue),
                context->GetTechniqueNameW()
            );
        }
    }

    // Reset imported texture to it's starting state every frame
    if (event == UnitTestEvent::PreExecute)
    {
        commandList->CopyResource(context->m_input.texture_ImportedTexture, texture_ImportedTexture_InitialState);
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture3DRW_CS\\0_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 1, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture3DRW_CS\\0_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 2, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture3DRW_CS\\0_2.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture3DRW_CS\\1_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 1, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture3DRW_CS\\1_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 2, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture3DRW_CS\\1_2.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Texture3DRW_PS::Context* context, UnitTestEvent event)
{
    static ID3D12Resource* texture_ImportedTexture_InitialState = nullptr;

    if (testContext.IsFirstPreExecute(event))
    {
        // Load and create Color
        {
            unsigned char clearValue[4] = { 255, 255, 255, 255 };

            for (int i = 0; i < 3; ++i)
            {
                double d = double(clearValue[i]) / 255.0;
                d = DX12Utils::LinearTosRGB(d);
                clearValue[i] = min(max(d * 256.0, 0.0), 255.0);
            }

            context->m_input.texture_Color_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            context->m_input.texture_Color_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_Color_size[0] = 64;
            context->m_input.texture_Color_size[1] = 64;
            context->m_input.texture_Color_size[2] = 1;
            context->m_input.texture_Color_numMips = 1;
            context->m_input.texture_Color = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_Color_flags,
                context->m_input.texture_Color_format,
                context->m_input.texture_Color_size,
                context->m_input.texture_Color_numMips,
                DX12Utils::ResourceType::Texture2D,
                (void*)clearValue, sizeof(clearValue),
                context->GetTechniqueNameW()
            );
        }

        // Load and create ImportedColor
        {
            unsigned char clearValue[4] = { 128, 128, 128, 255 };

            for (int i = 0; i < 3; ++i)
            {
                double d = double(clearValue[i]) / 255.0;
                d = DX12Utils::LinearTosRGB(d);
                clearValue[i] = min(max(d * 256.0, 0.0), 255.0);
            }

            context->m_input.texture_ImportedColor_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            context->m_input.texture_ImportedColor_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedColor_size[0] = 64;
            context->m_input.texture_ImportedColor_size[1] = 64;
            context->m_input.texture_ImportedColor_size[2] = 3;
            context->m_input.texture_ImportedColor_numMips = 1;
            context->m_input.texture_ImportedColor = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_ImportedColor_flags,
                context->m_input.texture_ImportedColor_format,
                context->m_input.texture_ImportedColor_size,
                context->m_input.texture_ImportedColor_numMips,
                DX12Utils::ResourceType::Texture3D,
                (void*)clearValue, sizeof(clearValue),
                context->GetTechniqueNameW()
            );
        }

        // Load and create ImportedTexture
        {
            context->m_input.texture_ImportedTexture_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            context->m_input.texture_ImportedTexture_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedTexture = context->CreateManagedTextureFromFile(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                DX12Utils::ResourceType::Texture3D,
                "..\\..\\..\\Techniques\\UnitTests\\Textures\\Image%i.png",
                true,
                context->m_input.texture_ImportedTexture_size,
                context->GetTechniqueNameW()
            );
        }

        // Store off the initial state of the imported texture as a managed resource so we can reset it every frame
        {
            texture_ImportedTexture_InitialState = context->CreateManagedTextureFromFile(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                DX12Utils::ResourceType::Texture3D,
                "..\\..\\..\\Techniques\\UnitTests\\Textures\\Image%i.png",
                true,
                context->m_input.texture_ImportedTexture_size,
                L"texture_ImportedTexture_InitialState",
                D3D12_RESOURCE_STATE_COPY_SOURCE
            );
        }
    }

    // Reset imported texture to it's starting state every frame
    if (event == UnitTestEvent::PreExecute)
    {
        commandList->CopyResource(context->m_input.texture_ImportedTexture, texture_ImportedTexture_InitialState);
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture3DRW_PS\\0_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 1, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture3DRW_PS\\0_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 2, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture3DRW_PS\\0_2.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture3DRW_PS\\1_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 1, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture3DRW_PS\\1_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 2, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture3DRW_PS\\1_2.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Texture3DRW_RGS::Context* context, UnitTestEvent event)
{
    static ID3D12Resource* texture_ImportedTexture_InitialState = nullptr;

    if (testContext.IsFirstPreExecute(event))
    {
        // Load and create ImportedTexture
        {
            context->m_input.texture_ImportedTexture_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            context->m_input.texture_ImportedTexture_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedTexture = context->CreateManagedTextureFromFile(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                DX12Utils::ResourceType::Texture3D,
                "..\\..\\..\\Techniques\\UnitTests\\Textures\\Image%i.png",
                true,
                context->m_input.texture_ImportedTexture_size,
                context->GetTechniqueNameW()
            );
        }

        // Store off the initial state of the imported texture as a managed resource so we can reset it every frame
        {
            texture_ImportedTexture_InitialState = context->CreateManagedTextureFromFile(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                DX12Utils::ResourceType::Texture3D,
                "..\\..\\..\\Techniques\\UnitTests\\Textures\\Image%i.png",
                true,
                context->m_input.texture_ImportedTexture_size,
                L"texture_ImportedTexture_InitialState",
                D3D12_RESOURCE_STATE_COPY_SOURCE
            );
        }

        // Load and create ImportedColor
        {
            unsigned char clearValue[4] = { 128, 128, 128, 255 };

            for (int i = 0; i < 3; ++i)
            {
                double d = double(clearValue[i]) / 255.0;
                d = DX12Utils::LinearTosRGB(d);
                clearValue[i] = min(max(d * 256.0, 0.0), 255.0);
            }

            context->m_input.texture_ImportedColor_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            context->m_input.texture_ImportedColor_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedColor_size[0] = 64;
            context->m_input.texture_ImportedColor_size[1] = 64;
            context->m_input.texture_ImportedColor_size[2] = 3;
            context->m_input.texture_ImportedColor_numMips = 1;
            context->m_input.texture_ImportedColor = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_ImportedColor_flags,
                context->m_input.texture_ImportedColor_format,
                context->m_input.texture_ImportedColor_size,
                context->m_input.texture_ImportedColor_numMips,
                DX12Utils::ResourceType::Texture3D,
                (void*)clearValue, sizeof(clearValue),
                context->GetTechniqueNameW()
            );
        }
    }

    // Reset imported texture to it's starting state every frame
    if (event == UnitTestEvent::PreExecute)
    {
        commandList->CopyResource(context->m_input.texture_ImportedTexture, texture_ImportedTexture_InitialState);
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture3DRW_RGS\\0_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 1, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture3DRW_RGS\\0_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 2, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture3DRW_RGS\\0_2.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture3DRW_RGS\\1_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 1, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture3DRW_RGS\\1_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 2, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Texture3DRW_RGS\\1_2.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, TextureCubeRW_CS::Context* context, UnitTestEvent event)
{
    static ID3D12Resource* texture_ImportedTexture_InitialState = nullptr;

    if (testContext.IsFirstPreExecute(event))
    {
        // Load and create ImportedColor
        {
            unsigned char clearValue[4] = { 64, 128, 196, 255 };

            for (int i = 0; i < 3; ++i)
            {
                double d = double(clearValue[i]) / 255.0;
                d = DX12Utils::LinearTosRGB(d);
                clearValue[i] = min(max(d * 256.0, 0.0), 255.0);
            }

            context->m_input.texture_ImportedColor_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            context->m_input.texture_ImportedColor_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedColor_size[0] = 64;
            context->m_input.texture_ImportedColor_size[1] = 64;
            context->m_input.texture_ImportedColor_size[2] = 6;
            context->m_input.texture_ImportedColor_numMips = 1;
            context->m_input.texture_ImportedColor = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_ImportedColor_flags,
                context->m_input.texture_ImportedColor_format,
                context->m_input.texture_ImportedColor_size,
                context->m_input.texture_ImportedColor_numMips,
                DX12Utils::ResourceType::TextureCube,
                (void*)clearValue, sizeof(clearValue),
                context->GetTechniqueNameW()
            );
        }

        // Load and create ImportedTexture
        {
            context->m_input.texture_ImportedTexture_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            context->m_input.texture_ImportedTexture_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedTexture = context->CreateManagedTextureFromFile(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                DX12Utils::ResourceType::TextureCube,
                "..\\..\\..\\Techniques\\UnitTests\\Textures\\Cube%s.png",
                true,
                context->m_input.texture_ImportedTexture_size,
                context->GetTechniqueNameW()
            );
        }

        // Store off the initial state of the imported texture as a managed resource so we can reset it every frame
        {
            texture_ImportedTexture_InitialState = context->CreateManagedTextureFromFile(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                DX12Utils::ResourceType::TextureCube,
                "..\\..\\..\\Techniques\\UnitTests\\Textures\\Cube%s.png",
                true,
                context->m_input.texture_ImportedTexture_size,
                L"texture_ImportedTexture_InitialState",
                D3D12_RESOURCE_STATE_COPY_SOURCE
            );
        }
    }

    // Reset imported texture to it's starting state every frame
    if (event == UnitTestEvent::PreExecute)
    {
        commandList->CopyResource(context->m_input.texture_ImportedTexture, texture_ImportedTexture_InitialState);
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_CS\\0_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 1, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_CS\\1_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 2, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_CS\\2_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 3, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_CS\\3_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 4, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_CS\\4_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 5, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_CS\\5_0.png");

        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_CS\\0_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 1, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_CS\\1_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 2, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_CS\\2_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 3, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_CS\\3_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 4, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_CS\\4_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 5, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_CS\\5_1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, TextureCubeRW_PS::Context* context, UnitTestEvent event)
{
    static ID3D12Resource* texture_ImportedTexture_InitialState = nullptr;

    if (testContext.IsFirstPreExecute(event))
    {
        // Load and create Color
        {
            unsigned char clearValue[4] = { 255, 255, 255, 255 };

            context->m_input.texture_Color_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            context->m_input.texture_Color_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_Color_size[0] = 64;
            context->m_input.texture_Color_size[1] = 64;
            context->m_input.texture_Color_size[2] = 1;
            context->m_input.texture_Color_numMips = 1;
            context->m_input.texture_Color = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_Color_flags,
                context->m_input.texture_Color_format,
                context->m_input.texture_Color_size,
                context->m_input.texture_Color_numMips,
                DX12Utils::ResourceType::Texture2D,
                (void*)clearValue, sizeof(clearValue),
                context->GetTechniqueNameW()
            );
        }

        // Load and create ImportedColor
        {
            unsigned char clearValue[4] = { 64, 128, 196, 255 };

            for (int i = 0; i < 3; ++i)
            {
                double d = double(clearValue[i]) / 255.0;
                d = DX12Utils::LinearTosRGB(d);
                clearValue[i] = min(max(d * 256.0, 0.0), 255.0);
            }

            context->m_input.texture_ImportedColor_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            context->m_input.texture_ImportedColor_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedColor_size[0] = 64;
            context->m_input.texture_ImportedColor_size[1] = 64;
            context->m_input.texture_ImportedColor_size[2] = 6;
            context->m_input.texture_ImportedColor_numMips = 1;
            context->m_input.texture_ImportedColor = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_ImportedColor_flags,
                context->m_input.texture_ImportedColor_format,
                context->m_input.texture_ImportedColor_size,
                context->m_input.texture_ImportedColor_numMips,
                DX12Utils::ResourceType::TextureCube,
                (void*)clearValue, sizeof(clearValue),
                context->GetTechniqueNameW()
            );
        }

        // Load and create ImportedTexture
        {
            context->m_input.texture_ImportedTexture_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            context->m_input.texture_ImportedTexture_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedTexture = context->CreateManagedTextureFromFile(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                DX12Utils::ResourceType::TextureCube,
                "..\\..\\..\\Techniques\\UnitTests\\Textures\\Cube%s.png",
                true,
                context->m_input.texture_ImportedTexture_size,
                context->GetTechniqueNameW()
            );
        }

        // Store off the initial state of the imported texture as a managed resource so we can reset it every frame
        {
            texture_ImportedTexture_InitialState = context->CreateManagedTextureFromFile(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                DX12Utils::ResourceType::TextureCube,
                "..\\..\\..\\Techniques\\UnitTests\\Textures\\Cube%s.png",
                true,
                context->m_input.texture_ImportedTexture_size,
                L"texture_ImportedTexture_InitialState",
                D3D12_RESOURCE_STATE_COPY_SOURCE
            );
        }
    }

    // Reset imported texture to it's starting state every frame
    if (event == UnitTestEvent::PreExecute)
    {
        commandList->CopyResource(context->m_input.texture_ImportedTexture, texture_ImportedTexture_InitialState);
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_PS\\0_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 1, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_PS\\1_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 2, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_PS\\2_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 3, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_PS\\3_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 4, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_PS\\4_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 5, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_PS\\5_0.png");

        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_PS\\0_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 1, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_PS\\1_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 2, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_PS\\2_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 3, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_PS\\3_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 4, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_PS\\4_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 5, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_PS\\5_1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, TextureCubeRW_RGS::Context* context, UnitTestEvent event)
{
    static ID3D12Resource* texture_ImportedTexture_InitialState = nullptr;

    if (testContext.IsFirstPreExecute(event))
    {
        // Load and create ImportedColor
        {
            unsigned char clearValue[4] = { 64, 128, 196, 255 };

            for (int i = 0; i < 3; ++i)
            {
                double d = double(clearValue[i]) / 255.0;
                d = DX12Utils::LinearTosRGB(d);
                clearValue[i] = min(max(d * 256.0, 0.0), 255.0);
            }

            context->m_input.texture_ImportedColor_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            context->m_input.texture_ImportedColor_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedColor_size[0] = 64;
            context->m_input.texture_ImportedColor_size[1] = 64;
            context->m_input.texture_ImportedColor_size[2] = 6;
            context->m_input.texture_ImportedColor_numMips = 1;
            context->m_input.texture_ImportedColor = context->CreateManagedTextureAndClear(
                device,
                commandList,
                context->m_input.texture_ImportedColor_flags,
                context->m_input.texture_ImportedColor_format,
                context->m_input.texture_ImportedColor_size,
                context->m_input.texture_ImportedColor_numMips,
                DX12Utils::ResourceType::TextureCube,
                (void*)clearValue, sizeof(clearValue),
                context->GetTechniqueNameW()
            );
        }

        // Load and create ImportedTexture
        {
            context->m_input.texture_ImportedTexture_format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            context->m_input.texture_ImportedTexture_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_ImportedTexture = context->CreateManagedTextureFromFile(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                DX12Utils::ResourceType::TextureCube,
                "..\\..\\..\\Techniques\\UnitTests\\Textures\\Cube%s.png",
                true,
                context->m_input.texture_ImportedTexture_size,
                context->GetTechniqueNameW()
            );
        }

        // Store off the initial state of the imported texture as a managed resource so we can reset it every frame
        {
            texture_ImportedTexture_InitialState = context->CreateManagedTextureFromFile(
                device,
                commandList,
                context->m_input.texture_ImportedTexture_flags,
                context->m_input.texture_ImportedTexture_format,
                DX12Utils::ResourceType::TextureCube,
                "..\\..\\..\\Techniques\\UnitTests\\Textures\\Cube%s.png",
                true,
                context->m_input.texture_ImportedTexture_size,
                L"texture_ImportedTexture_InitialState",
                D3D12_RESOURCE_STATE_COPY_SOURCE
            );
        }
    }

    // Reset imported texture to it's starting state every frame
    if (event == UnitTestEvent::PreExecute)
    {
        commandList->CopyResource(context->m_input.texture_ImportedTexture, texture_ImportedTexture_InitialState);
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_RGS\\0_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 1, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_RGS\\1_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 2, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_RGS\\2_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 3, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_RGS\\3_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 4, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_RGS\\4_0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_input.texture_ImportedTexture, context->m_input.texture_ImportedTexture_state, 5, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_RGS\\5_0.png");

        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_RGS\\0_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 1, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_RGS\\1_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 2, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_RGS\\2_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 3, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_RGS\\3_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 4, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_RGS\\4_1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_NodeTexture, context->m_output.c_texture_NodeTexture_endingState, 5, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureCubeRW_RGS\\5_1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Mips_CS_2D::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Output, context->m_output.c_texture_Output_endingState, 0, 3, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_CS_2D\\0.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Mips_CS_2DArray::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Output, context->m_output.c_texture_Output_endingState, 0, 3, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_CS_2DArray\\0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Output, context->m_output.c_texture_Output_endingState, 1, 3, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_CS_2DArray\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Mips_CS_Cube::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Output, context->m_output.c_texture_Output_endingState, 0, 3, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_CS_Cube\\0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Output, context->m_output.c_texture_Output_endingState, 1, 3, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_CS_Cube\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Mips_CS_3D::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Output, context->m_output.c_texture_Output_endingState, 0, 3, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_CS_3D\\0.0.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Mips_DrawCall::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        // Load, create and set Vertex Buffer - Bunny
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Textures\\Mips_DrawCall_Bunny.bin");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Textures\\Mips_DrawCall_Bunny.bin");
                return;
            }

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_VertexBuffer_format = DXGI_FORMAT_UNKNOWN;
            context->m_input.buffer_VertexBuffer_stride = sizeof(Mips_DrawCall::Struct_VertexFormat);
            context->m_input.buffer_VertexBuffer_count = file.GetSize() / sizeof(Mips_DrawCall::Struct_VertexFormat);
            context->m_input.buffer_VertexBuffer_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_VertexBuffer = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_VertexBuffer_flags, file.GetBytes(), file.GetSize(), context->GetTechniqueNameW());

            // Set up the vertex input layout for the vertex buffer
            context->m_input.buffer_VertexBuffer_vertexInputLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
            context->m_input.buffer_VertexBuffer_vertexInputLayout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
            context->m_input.buffer_VertexBuffer_vertexInputLayout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
        }

        // Load, create and set Vertex Buffer - Sphere
        {
            DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Textures\\Mips_DrawCall_Sphere.bin");
            if (!file.Valid())
            {
                testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Textures\\Mips_DrawCall_Sphere.bin");
                return;
            }

            // Create a buffer and set the buffer data on the technique context
            context->m_input.buffer_SphereVB_format = DXGI_FORMAT_UNKNOWN;
            context->m_input.buffer_SphereVB_stride = sizeof(Mips_DrawCall::Struct_VertexFormat);
            context->m_input.buffer_SphereVB_count = file.GetSize() / sizeof(Mips_DrawCall::Struct_VertexFormat);
            context->m_input.buffer_SphereVB_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.buffer_SphereVB = context->CreateManagedBuffer(device, commandList, context->m_input.c_buffer_SphereVB_flags, file.GetBytes(), file.GetSize(), context->GetTechniqueNameW());

            // Set up the vertex input layout for the vertex buffer
            context->m_input.buffer_SphereVB_vertexInputLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
            context->m_input.buffer_SphereVB_vertexInputLayout.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
            context->m_input.buffer_SphereVB_vertexInputLayout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

            // Making sure the matrix is the exact same as in the test.
            // You can save as hex in the gigi viewer
            uint32_t viewProjMtx[] = {
                0x3F7CDFB5, 0x00000000, 0x400CFC0F, 0xBE27456E,
                0xBF4D915C, 0x400FE0EC, 0x3EB85B26, 0xBF255AC1,
                0x38B2358B, 0x3818E824, 0xB81FD206, 0x3DCC5EFD,
                0xBF5984CA, 0xBEBAA298, 0x3EC312E2, 0x400C6EEC
            };

            memcpy(&context->m_input.variable_ViewProjMtx, viewProjMtx, sizeof(viewProjMtx));
        }
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Final_Color_Buffer, context->m_output.c_texture_Final_Color_Buffer_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_DrawCall\\0.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Mips_ShaderToken_2D::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_Output, context->m_output.c_texture_Output_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_ShaderToken_2D\\0.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Mips_ShaderToken_2DArray::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_internal.texture__loadedTexture_0, context->m_internal.c_texture__loadedTexture_0_endingState, 0, 3, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_ShaderToken_2DArray\\0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_internal.texture__loadedTexture_0, context->m_internal.c_texture__loadedTexture_0_endingState, 1, 3, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_ShaderToken_2DArray\\1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_internal.texture__loadedTexture_0, context->m_internal.c_texture__loadedTexture_0_endingState, 2, 3, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_ShaderToken_2DArray\\2.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Mips_ShaderToken_Cube::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_internal.texture__loadedTexture_0, context->m_internal.c_texture__loadedTexture_0_endingState, 0, 3, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_ShaderToken_Cube\\0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_internal.texture__loadedTexture_0, context->m_internal.c_texture__loadedTexture_0_endingState, 2, 3, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_ShaderToken_Cube\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Mips_ShaderToken_3D::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_internal.texture__loadedTexture_0, context->m_internal.c_texture__loadedTexture_0_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_ShaderToken_3D\\0.0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_internal.texture__loadedTexture_0, context->m_internal.c_texture__loadedTexture_0_endingState, 1, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_ShaderToken_3D\\0.1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_internal.texture__loadedTexture_0, context->m_internal.c_texture__loadedTexture_0_endingState, 2, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_ShaderToken_3D\\0.2.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_internal.texture__loadedTexture_0, context->m_internal.c_texture__loadedTexture_0_endingState, 3, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_ShaderToken_3D\\0.3.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_internal.texture__loadedTexture_0, context->m_internal.c_texture__loadedTexture_0_endingState, 4, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_ShaderToken_3D\\0.4.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_internal.texture__loadedTexture_0, context->m_internal.c_texture__loadedTexture_0_endingState, 5, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_ShaderToken_3D\\0.5.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_internal.texture__loadedTexture_0, context->m_internal.c_texture__loadedTexture_0_endingState, 0, 1, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_ShaderToken_3D\\1.0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_internal.texture__loadedTexture_0, context->m_internal.c_texture__loadedTexture_0_endingState, 1, 1, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_ShaderToken_3D\\1.1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_internal.texture__loadedTexture_0, context->m_internal.c_texture__loadedTexture_0_endingState, 2, 1, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_ShaderToken_3D\\1.2.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_internal.texture__loadedTexture_0, context->m_internal.c_texture__loadedTexture_0_endingState, 0, 2, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_ShaderToken_3D\\2.0.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Mips_RGS_2D::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPreExecute(event))
    {
        DX12Utils::FileCache::File& file = DX12Utils::FileCache::Get("..\\..\\..\\Techniques\\UnitTests\\Textures\\Mips_RGS_2D.bin");
        if (!file.Valid())
        {
            testContext.Fail("Could not load ..\\..\\..\\Techniques\\UnitTests\\Textures\\Mips_RGS_2D.bin");
            return;
        }

        // Create a vertex buffer to hold the vertex data
        ID3D12Resource* vertexData = context->CreateManagedBuffer(device, commandList, D3D12_RESOURCE_FLAG_NONE, file.GetBytes(), file.GetSize(), context->GetTechniqueNameW(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

        // Create the tlas buffer
        context->m_input.buffer_VertexBuffer_format = DXGI_FORMAT_R32G32B32_FLOAT;
        context->m_input.buffer_VertexBuffer_stride = 0;
        context->m_input.buffer_VertexBuffer_count = file.GetSize() / (sizeof(float) * 3);
        context->m_input.buffer_VertexBuffer_state = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        context->CreateManagedTLAS(
            device, commandList,
            vertexData, context->m_input.buffer_VertexBuffer_count, false,
            D3D12_RAYTRACING_GEOMETRY_FLAG_NONE,
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
            DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 3,
            context->m_input.buffer_VertexBuffer_blas, context->m_input.buffer_VertexBuffer_blasSize,
            context->m_input.buffer_VertexBuffer, context->m_input.buffer_VertexBuffer_tlasSize,
            context->LogFn
        );
    }

    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_MipTex, context->m_output.c_texture_MipTex_endingState, 0, 0, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_RGS_2D\\0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_MipTex, context->m_output.c_texture_MipTex_endingState, 0, 1, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_RGS_2D\\1.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_MipTex, context->m_output.c_texture_MipTex_endingState, 0, 2, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_RGS_2D\\2.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, Mips_VSPS_2D::Context* context, UnitTestEvent event)
{
    if (testContext.IsFirstPostExecute(event))
    {
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_MipTex, context->m_output.c_texture_MipTex_endingState, 0, 1, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_VSPS_2D\\0.png");
        testContext.VerifyReadbackPNG(device, commandList, context->m_output.texture_MipTex, context->m_output.c_texture_MipTex_endingState, 0, 2, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\Mips_VSPS_2D\\1.png");
    }
}

void UnitTestImpl(UnitTestContext& testContext, ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DX12Utils::ReadbackHelper& readbackHelper, TextureFormats::Context* context, UnitTestEvent event)
{
    // TextureFormat enum (TextureFormats.h)
    // put through TextureFormatToDXGI_FORMAT
    // Not including "Any", "D32_Float_S8", or "D24_Unorm_S8"
    // Needs to match the noise order and count tested for in TextureFormats.py
    struct Format
    {
        DXGI_FORMAT format;
        const char* name;
    };
    static const Format s_formats[] =
    {
        { DXGI_FORMAT_R8_UNORM, "R8_Unorm"},
        { DXGI_FORMAT_R8G8_UNORM, "RG8_Unorm"},
        { DXGI_FORMAT_R8G8B8A8_UNORM, "RGBA8_Unorm"},
        { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, "RGBA8_Unorm_sRGB"},
        { DXGI_FORMAT_R16_FLOAT, "R16_Float"},
        { DXGI_FORMAT_R16G16_FLOAT, "RG16_Float"},
        { DXGI_FORMAT_R16G16B16A16_FLOAT, "RGBA16_Float"},
        { DXGI_FORMAT_R32_FLOAT, "R32_Float"},
        { DXGI_FORMAT_R32G32_FLOAT, "RG32_Float"},
        { DXGI_FORMAT_R32G32B32A32_FLOAT, "RGBA32_Float"},
        { DXGI_FORMAT_R32_UINT, "R32_Uint"},
        { DXGI_FORMAT_R32G32B32A32_UINT, "RGBA32_Uint"},
        { DXGI_FORMAT_R11G11B10_FLOAT, "R11G11B10_Float"},
        { DXGI_FORMAT_D32_FLOAT, "D32_Float"},
        { DXGI_FORMAT_D16_UNORM, "D16_Unorm"},
    };
    static std::string fileNames[_countof(s_formats)];

    // Each frame we render to a different format texture and do a readback request for it
    static int s_frameIndex = 0;

    if (s_frameIndex >= _countof(s_formats))
        return;

    if (event == UnitTestEvent::PreExecute)
    {
        // Load and create ImportedColor
        {
            context->m_input.texture_Texture_format = s_formats[s_frameIndex].format;
            context->m_input.texture_Texture_state = D3D12_RESOURCE_STATE_COPY_DEST;
            context->m_input.texture_Texture_size[0] = 16;
            context->m_input.texture_Texture_size[1] = 16;
            context->m_input.texture_Texture_size[2] = 1;
            context->m_input.texture_Texture_numMips = 1;
            context->m_input.texture_Texture = context->CreateManagedTexture(
                device,
                commandList,
                context->m_input.texture_Texture_flags,
                context->m_input.texture_Texture_format,
                context->m_input.texture_Texture_size,
                context->m_input.texture_Texture_numMips,
                DX12Utils::ResourceType::Texture2D,
                nullptr,
                context->GetTechniqueNameW()
            );
        }
    }

    if (event == UnitTestEvent::PostExecute)
    {
        char buffer[1024];
        sprintf_s(buffer, "..\\..\\..\\Techniques\\UnitTests\\_GoldImages\\Textures\\TextureFormats\\%s.bin", s_formats[s_frameIndex].name);
        fileNames[s_frameIndex] = buffer;
        testContext.VerifyReadbackBinary(device, commandList, context->m_input.texture_Texture, context->m_input.texture_Texture_state, 0, 0, fileNames[s_frameIndex].c_str());
        s_frameIndex++;
    }
}
