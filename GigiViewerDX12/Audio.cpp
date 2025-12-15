///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "Audio.h"

#include "Nodes/nodes.h"
#include "Interpreter/GigiInterpreterPreviewWindowDX12.h"
#include "Shared/HashAll.h"
#include "DX12Utils/Profiler.h"

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <cmath>

#include "Schemas/Types.h"

#include "DX12Utils/ReadbackObject.h"

#include <functiondiscoverykeys_devpkey.h>  // For PKEY_Device_FriendlyName

#pragma comment(lib, "ole32.lib")

#define REFTIMES_PER_MSEC  10000
#define SAFE_RELEASE(PTR) if ((PTR) != NULL) { (PTR)->Release(); (PTR) = NULL; }

namespace Audio
{
    struct QueuedReadback
    {
        unsigned int sampleCount = 0;
    };

    struct Output
    {
        std::string deviceName;

        int framesInFlight = 0;

        IMMDevice* pDevice = NULL;
        IAudioClient* pAudioClient = NULL;
        UINT32 bufferFrameCount = 0;
        IAudioRenderClient* pRenderClient = NULL;

        unsigned int nextSampleCount = 0;

        std::vector<QueuedReadback> readbacks;

        std::vector<float> cpuReadbackBuffer;

        ReadbackObject readbackObject;

        unsigned int frameIndex = 0;
    };

    struct Input
    {
        std::string deviceName;

        IMMDevice* pDevice = NULL;
        IAudioClient* pAudioClient = NULL;
        UINT32 bufferFrameCount = 0;
        IAudioCaptureClient* pCaptureClient = NULL;

        std::vector<float> samples;
    };

    struct State
    {
        size_t lastSettingsHash = 0;

        IMMDeviceEnumerator* pEnumerator = NULL;
        WAVEFORMATEXTENSIBLE wfx = {};

        Output output;
        Input input;
    };

    static State s_state;

    bool Init(int framesInFlight)
    {
        s_state.output.framesInFlight = framesInFlight;
        s_state.output.readbacks.resize(s_state.output.framesInFlight);

        // Initialize COM
        HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        return !FAILED(hr);
    }

    void ReleaseAudioInput()
    {
        SAFE_RELEASE(s_state.input.pDevice);
        SAFE_RELEASE(s_state.input.pAudioClient);
        SAFE_RELEASE(s_state.input.pCaptureClient);
    }

    void ReleaseAudioOutput(GigiInterpreterPreviewWindowDX12& interpreter)
    {
        SAFE_RELEASE(s_state.output.pDevice);
        SAFE_RELEASE(s_state.output.pAudioClient);
        SAFE_RELEASE(s_state.output.pRenderClient);

        s_state.output.readbacks.clear();
        s_state.output.readbacks.resize(s_state.output.framesInFlight);

        s_state.output.nextSampleCount = 0;

        s_state.output.readbackObject.Release(interpreter.getDelayedReleaseTracker());
    }

    void ReleaseAudio(GigiInterpreterPreviewWindowDX12& interpreter)
    {
        SAFE_RELEASE(s_state.pEnumerator);
        ReleaseAudioInput();
        ReleaseAudioOutput(interpreter);
    }

    void Shutdown(GigiInterpreterPreviewWindowDX12& interpreter)
    {
        ReleaseAudio(interpreter);
        CoUninitialize();
    }

    bool InitAudioOutput(const GGUserFile_Audio& audioSettings, GigiInterpreterPreviewWindowDX12& interpreter)
    {
        // Get default audio endpoint
        HRESULT hr = s_state.pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &s_state.output.pDevice);
        if (FAILED(hr))
            return false;

        // Get the device name
        {
            IPropertyStore* pProps = NULL;
            hr = s_state.output.pDevice->OpenPropertyStore(STGM_READ, &pProps);
            if (SUCCEEDED(hr))
            {
                PROPVARIANT varName;
                PropVariantInit(&varName);

                hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
                if (SUCCEEDED(hr))
                    s_state.output.deviceName = FromWideString(varName.pwszVal);

                pProps->Release();
            }
        }

        // Activate audio client
        hr = s_state.output.pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&s_state.output.pAudioClient);
        if (FAILED(hr))
            return false;

        // Initialize audio client
        hr = s_state.output.pAudioClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED,
            AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
            REFTIMES_PER_MSEC * audioSettings.outputBufferLengthMs,
            0,
            (WAVEFORMATEX*)&s_state.wfx,
            NULL);

        if (FAILED(hr))
            return false;

        // Get buffer size
        hr = s_state.output.pAudioClient->GetBufferSize(&s_state.output.bufferFrameCount);
        if (FAILED(hr))
            return false;

        // Get render client
        hr = s_state.output.pAudioClient->GetService(__uuidof(IAudioRenderClient), (void**)&s_state.output.pRenderClient);
        if (FAILED(hr))
            return false;

        // Generate initial buffer
        BYTE* pData = NULL;
        hr = s_state.output.pRenderClient->GetBuffer(s_state.output.bufferFrameCount, &pData);
        if (FAILED(hr))
            return false;

        // Fill initial buffer with silence
        memset(pData, 0, s_state.output.bufferFrameCount * s_state.wfx.Format.nBlockAlign);
        hr = s_state.output.pRenderClient->ReleaseBuffer(s_state.output.bufferFrameCount, 0);
        if (FAILED(hr))
            return false;

        // Start playing
        hr = s_state.output.pAudioClient->Start();
        if (FAILED(hr))
            return false;

        return true;
    }

    bool InitAudioInput(const GGUserFile_Audio& audioSettings, GigiInterpreterPreviewWindowDX12& interpreter)
    {
        // Get default audio endpoint
        HRESULT hr = s_state.pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &s_state.input.pDevice);
        if (FAILED(hr))
            return false;

        // Get the device name
        {
            IPropertyStore* pProps = NULL;
            hr = s_state.input.pDevice->OpenPropertyStore(STGM_READ, &pProps);
            if (SUCCEEDED(hr))
            {
                PROPVARIANT varName;
                PropVariantInit(&varName);

                hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
                if (SUCCEEDED(hr))
                    s_state.input.deviceName = FromWideString(varName.pwszVal);

                pProps->Release();
            }
        }

        // Activate audio client
        hr = s_state.input.pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&s_state.input.pAudioClient);
        if (FAILED(hr))
            return false;

        // Initialize audio client
        hr = s_state.input.pAudioClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED,
            AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
            REFTIMES_PER_MSEC * audioSettings.inputBufferLengthMs,
            0,
            (WAVEFORMATEX*)&s_state.wfx,
            NULL);

        if (FAILED(hr))
            return false;

        // Get buffer size
        hr = s_state.input.pAudioClient->GetBufferSize(&s_state.input.bufferFrameCount);
        if (FAILED(hr))
            return false;

        // Get capture client
        hr = s_state.input.pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&s_state.input.pCaptureClient);
        if (FAILED(hr))
            return false;

        // Start capturing
        hr = s_state.input.pAudioClient->Start();
        if (FAILED(hr))
            return false;

        return true;
    }

    void ReinitAsNeeded(const GGUserFile_Audio& audioSettings, GigiInterpreterPreviewWindowDX12& interpreter)
    {
        // If same settings as last time, do nothing
        size_t hash = HashAll(audioSettings.sampleRate, audioSettings.stereo, audioSettings.outputBufferLengthMs, audioSettings.inputBufferLengthMs);
        if (s_state.lastSettingsHash == hash)
            return;
        s_state.lastSettingsHash = hash;

        // Release the audio
        ReleaseAudio(interpreter);

        // Init the audio
        // Create device enumerator
        HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
                              __uuidof(IMMDeviceEnumerator), (void**)&s_state.pEnumerator);
        if (FAILED(hr))
            return;

        // Set the desired format
        s_state.wfx.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        s_state.wfx.Format.nChannels = audioSettings.stereo ? 2 : 1;
        s_state.wfx.Format.nSamplesPerSec = audioSettings.sampleRate;
        s_state.wfx.Format.wBitsPerSample = 32;
        s_state.wfx.Format.nBlockAlign = (s_state.wfx.Format.nChannels * s_state.wfx.Format.wBitsPerSample) / 8;
        s_state.wfx.Format.nAvgBytesPerSec = s_state.wfx.Format.nSamplesPerSec * s_state.wfx.Format.nBlockAlign;
        s_state.wfx.Format.cbSize = 22;
        s_state.wfx.Samples.wValidBitsPerSample = 32;
        s_state.wfx.dwChannelMask = audioSettings.stereo ? (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT) : SPEAKER_FRONT_CENTER;
        s_state.wfx.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;

        if (!InitAudioOutput(audioSettings, interpreter))
            ReleaseAudioOutput(interpreter);

        if (!InitAudioInput(audioSettings, interpreter))
            ReleaseAudioInput();
    }

    bool DoReadback(const GGUserFile_Audio& audioSettings, ID3D12Resource* readbackResource, bool forceSilence)
    {
        // if nothing to read back, nothing to do
        QueuedReadback& readback = s_state.output.readbacks[s_state.output.frameIndex % s_state.output.framesInFlight];
        if (!readbackResource || readback.sampleCount == 0)
            return true;

        // numBytesRead can be bigger than the resource if the settings are changed
        D3D12_RESOURCE_DESC readbackResourceDesc = readbackResource->GetDesc();
        size_t numBytesRead = readback.sampleCount * sizeof(float) * (audioSettings.stereo ? 2 : 1);
        numBytesRead = min(numBytesRead, readbackResourceDesc.Width);

        // Keep a buffer to hold the data on the CPU
        s_state.output.cpuReadbackBuffer.resize(numBytesRead / sizeof(float));

        // Map and copy to our CPU buffer
        {
            D3D12_RANGE readRange;
            readRange.Begin = 0;
            readRange.End = numBytesRead;

            D3D12_RANGE writeRange;
            writeRange.Begin = 1;
            writeRange.End = 0;

            unsigned char* data = nullptr;
            HRESULT hr = readbackResource->Map(0, &readRange, reinterpret_cast<void**>(&data));
            if (FAILED(hr))
                return false;

            memcpy(s_state.output.cpuReadbackBuffer.data(), data, numBytesRead);

            readbackResource->Unmap(0, &writeRange);
        }

        // Apply volume
        {
            float volume = audioSettings.outputVolume * (forceSilence ? 0.0f : 1.0f);
            if (volume != 1.0f)
            {
                for (size_t i = 0; i < readback.sampleCount * (audioSettings.stereo ? 2 : 1); ++i)
                    s_state.output.cpuReadbackBuffer[i] *= volume;
            }
        }

        // Write it to the audio buffer
        {
            BYTE* pData = NULL;
            HRESULT hr = s_state.output.pRenderClient->GetBuffer(readback.sampleCount, &pData);
            if (FAILED(hr))
                return false;

            memcpy(pData, s_state.output.cpuReadbackBuffer.data(), numBytesRead);

            hr = s_state.output.pRenderClient->ReleaseBuffer(readback.sampleCount, 0);
            if (FAILED(hr))
                return false;
        }

        // We handled these bytes
        readback.sampleCount = 0;

        return true;
    }

    void PostRender(const GGUserFile_Audio& audioSettings, GigiInterpreterPreviewWindowDX12& interpreter, ID3D12GraphicsCommandList* commandList, int maxFramesInFlight, bool forceSilence)
    {
        ReinitAsNeeded(audioSettings, interpreter);

        if (!s_state.output.pAudioClient)
            return;

        // Do readback
        {
            bool existsAsBuffer = false;
            RuntimeTypes::RenderGraphNode_Resource_Buffer& runtimeDataBuffer = interpreter.GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(audioSettings.outputBuffer.name.c_str(), existsAsBuffer);
            if (existsAsBuffer)
            {
                ID3D12Resource* readbackResource = s_state.output.readbackObject.OnNewFrame(interpreter.GetDevice(), commandList, runtimeDataBuffer.m_resource, maxFramesInFlight, interpreter.getDelayedReleaseTracker(), interpreter.GetTransitionsNonConst());
                if (readbackResource)
                    DoReadback(audioSettings, readbackResource, forceSilence);
            }
        }

        // Set the size and format of the audio output buffer.
        {
            for (auto& pair : interpreter.m_importedResources)
            {
                if (pair.second.nodeIndex == -1 || pair.second.resourceIndex == -1)
                    continue;

                if (audioSettings.outputBuffer.name == pair.first)
                {
                    int desiredCount = audioSettings.outputBufferLengthMs * audioSettings.sampleRate * (audioSettings.stereo ? 2 : 1) / 1000;
                    if (pair.second.buffer.type != DataFieldType::Float || pair.second.buffer.count != desiredCount)
                    {
                        pair.second.buffer.type = DataFieldType::Float;
                        pair.second.buffer.count = desiredCount;
                        pair.second.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                    }
                    break;
                }
            }
        }

        // Queue up the next readback
        {
            QueuedReadback& readback = s_state.output.readbacks[s_state.output.frameIndex % s_state.output.framesInFlight];
            readback.sampleCount = s_state.output.nextSampleCount;
        }

        // Advance what frame we are on
        s_state.output.frameIndex++;
    }

    bool PreRender(const GGUserFile_Audio& audioSettings, GigiInterpreterPreviewWindowDX12& interpreter, ID3D12GraphicsCommandList* commandList)
    {
        // Init the audio output variables
        int varStereoIndex = interpreter.GetRuntimeVariableIndex(audioSettings.Var_Stereo.c_str());
        int varSampleRateIndex = interpreter.GetRuntimeVariableIndex(audioSettings.Var_SampleRate.c_str());
        int varOutSampleWindowCountIndex = interpreter.GetRuntimeVariableIndex(audioSettings.Var_AudioOutSampleWindowCount.c_str());
        int varInSampleWindowCountIndex = interpreter.GetRuntimeVariableIndex(audioSettings.Var_AudioInSampleWindowCount.c_str());
        {
            if (varStereoIndex != -1)
                *((bool*)interpreter.GetRuntimeVariable(varStereoIndex).storage.value) = audioSettings.stereo;

            if (varSampleRateIndex != -1)
                *((unsigned int*)interpreter.GetRuntimeVariable(varSampleRateIndex).storage.value) = audioSettings.sampleRate;

            if (varOutSampleWindowCountIndex != -1)
                *((unsigned int*)interpreter.GetRuntimeVariable(varOutSampleWindowCountIndex).storage.value) = 0;

            if (varInSampleWindowCountIndex != -1)
                *((unsigned int*)interpreter.GetRuntimeVariable(varInSampleWindowCountIndex).storage.value) = 0;
        }

        ReinitAsNeeded(audioSettings, interpreter);

        // Audio Ouput
        if (s_state.output.pAudioClient)
        {
            // Calculate how many samples are available
            UINT32 numFramesPadding;
            HRESULT hr = s_state.output.pAudioClient->GetCurrentPadding(&numFramesPadding);
            if (FAILED(hr))
                return false;
            UINT32 numFramesAvailable = s_state.output.bufferFrameCount - numFramesPadding;

            // Also subtract out queued readbacks
            for (QueuedReadback& readback : s_state.output.readbacks)
                numFramesAvailable -= readback.sampleCount;

            // Limit to our buffer size. Can come up when settings change.
            UINT32 bufferFramesTotal = audioSettings.outputBufferLengthMs * audioSettings.sampleRate / 1000;
            numFramesAvailable = min(numFramesAvailable, bufferFramesTotal);

            // Set the number of frames available variable
            if (varOutSampleWindowCountIndex != -1)
                *((unsigned int*)interpreter.GetRuntimeVariable(varOutSampleWindowCountIndex).storage.value) = numFramesAvailable;

            s_state.output.nextSampleCount = numFramesAvailable;
        }

        // Audio Input
        if (s_state.input.pAudioClient)
        {
            // Set the size and format of the audio input buffer.
            for (auto& pair : interpreter.m_importedResources)
            {
                if (pair.second.nodeIndex == -1 || pair.second.resourceIndex == -1)
                    continue;

                if (audioSettings.inputBuffer.name == pair.first)
                {
                    int desiredCount = audioSettings.inputBufferLengthMs * audioSettings.sampleRate * (audioSettings.stereo ? 2 : 1) / 1000;
                    if (pair.second.buffer.type != DataFieldType::Float || pair.second.buffer.count != desiredCount || pair.second.resetEveryFrame)
                    {
                        pair.second.resetEveryFrame = false;
                        pair.second.buffer.type = DataFieldType::Float;
                        pair.second.buffer.count = desiredCount;
                        pair.second.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                    }
                    break;
                }
            }

            // Get the resource
            ID3D12Resource* destResource = nullptr;
            {
                bool existsAsBuffer = false;
                RuntimeTypes::RenderGraphNode_Resource_Buffer& runtimeDataBuffer = interpreter.GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(audioSettings.inputBuffer.name.c_str(), existsAsBuffer);
                if (existsAsBuffer)
                    destResource = runtimeDataBuffer.m_resource;
            }

            if (destResource)
            {
                s_state.input.samples.resize(0);

                UINT32 nextPacketSize = 0;
                HRESULT hr = s_state.input.pCaptureClient->GetNextPacketSize(&nextPacketSize);
                if (FAILED(hr))
                    return false;

                UINT32 bufferFrames = (audioSettings.inputBufferLengthMs * audioSettings.sampleRate) / 1000;

                while (bufferFrames > 0 && nextPacketSize > 0)
                {
                    BYTE* pData = NULL;
                    UINT32 numFramesAvailable = 0;
                    DWORD flags = 0;

                    hr = s_state.input.pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
                    if (FAILED(hr))
                        return false;

                    numFramesAvailable = min(numFramesAvailable, bufferFrames);

                    size_t start = s_state.input.samples.size();
                    size_t numFloats = numFramesAvailable * (audioSettings.stereo ? 2 : 1);

                    s_state.input.samples.resize(start + numFloats);

                    memcpy(&s_state.input.samples[start], pData, numFloats * sizeof(float));

                    hr = s_state.input.pCaptureClient->ReleaseBuffer(numFramesAvailable);
                    if (FAILED(hr))
                        return false;

                    hr = s_state.input.pCaptureClient->GetNextPacketSize(&nextPacketSize);
                    if (FAILED(hr))
                        return false;
                }

                // Apply volume
                {
                    float volume = audioSettings.inputVolume;
                    if (volume != 1.0f)
                    {
                        for (float& f : s_state.input.samples)
                            f *= volume;
                    }
                }

                if (s_state.input.samples.size() > 0)
                {
                    BasicPixScopeProfiler _p(commandList, "Copying audio input into buffer");

                    // Transition destination resource to copy dest
                    TransitionTracker& transitionTracker = interpreter.GetTransitionsNonConst();
                    transitionTracker.Transition(TRANSITION_DEBUG_INFO(destResource, D3D12_RESOURCE_STATE_COPY_DEST));
                    transitionTracker.Flush(commandList);

                    UploadBufferTracker uploadBufferTracker = interpreter.getUploadBufferTracker();
                    UploadBufferTracker::Buffer* uploadBuffer = uploadBufferTracker.GetBuffer(interpreter.GetDevice(), s_state.input.samples.size() * sizeof(float), false);
                    if (!uploadBuffer)
                        return false;

                    // Write the data to the upload buffer
                    {
                        // map the memory
                        D3D12_RANGE readRange;
                        readRange.Begin = 1;
                        readRange.End = 0;
                        float* destSamples = nullptr;
                        HRESULT hr = uploadBuffer->buffer->Map(0, &readRange, reinterpret_cast<void**>(&destSamples));
                        if (FAILED(hr))
                            return false;

                        memcpy(destSamples, s_state.input.samples.data(), s_state.input.samples.size() * sizeof(float));

                        uploadBuffer->buffer->Unmap(0, nullptr);
                    }

                    // Limit the copy to the actual size of the buffer. This can happen when the buffer changes size
                    size_t copyBytes = s_state.input.samples.size() * sizeof(float);
                    copyBytes = min(copyBytes, destResource->GetDesc().Width);

                    commandList->CopyBufferRegion(destResource, 0, uploadBuffer->buffer, 0, copyBytes);
                }

                // Set the number of frames available variable
                if (varInSampleWindowCountIndex != -1)
                    *((unsigned int*)interpreter.GetRuntimeVariable(varInSampleWindowCountIndex).storage.value) = (unsigned int)s_state.input.samples.size() / (audioSettings.stereo ? 2 : 1);
            }
        }

        return true;
    }

    std::string GetInputDeviceName()
    {
        return s_state.input.deviceName;
    }

    std::string GetOutputDeviceName()
    {
        return s_state.output.deviceName;
    }
};
