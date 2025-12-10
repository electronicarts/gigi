///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "WebCam.h"

#include "Nodes/nodes.h"
#include "Interpreter/GigiInterpreterPreviewWindowDX12.h"
#include "Shared/HashAll.h"
#include "DX12Utils/Profiler.h"

#include "Schemas/Types.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <Mferror.h>
#include <Shlwapi.h>

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "shlwapi.lib")

namespace webcam
{
	// SourceReaderCallback enables asynchronous calls to ReadSample
	// which would otherwise block.
	// OnReadSample is called on another thread established by Windows
	// with the result of calling ReadSample.
	// Client calls GetSample to retrieve whatever sample if any was last
	// delivered.
	class SourceReaderCallback : public IMFSourceReaderCallback
	{
	public:
		SourceReaderCallback()
			: m_nRefCount{ 1 }
			, m_bEndOfStream{ TRUE }
			, m_hrStatus{ S_OK }
			, m_LastSample{ nullptr }
		{
			InitializeCriticalSection(&m_CriticalSection);
		}

		// IUnknown methods
		STDMETHODIMP QueryInterface(REFIID iid, void** ppv)
		{
			static const QITAB qit[] =
			{
				QITABENT(SourceReaderCallback, IMFSourceReaderCallback),
				{ 0 },
			};
			return QISearch(this, qit, iid, ppv);
		}
		STDMETHODIMP_(ULONG) AddRef()
		{
			return InterlockedIncrement(&m_nRefCount);
		}
		STDMETHODIMP_(ULONG) Release()
		{
			ULONG uCount = InterlockedDecrement(&m_nRefCount);
			if (uCount == 0)
			{
				delete this;
			}
			return uCount;
		}

		// GetSample may return NULL, this is fine.
		IMFSample* GetSample(IMFSourceReader* sourceReader)
		{
			EnterCriticalSection(&m_CriticalSection);

			// Do we have a sample to return?
			IMFSample* returnedSample = m_LastSample;
			if (m_LastSample)
				m_LastSample = nullptr;

			if (m_nQueuedSamples == 0)
			{
				HRESULT hr = sourceReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, NULL, NULL, NULL, NULL);
				if (FAILED(hr))
				{
					abort();
				}
				m_nQueuedSamples = 1;
			}

			LeaveCriticalSection(&m_CriticalSection);

			// Caller's responsibility to Release() the returned sample.
			return returnedSample;
		}

		STDMETHODIMP OnReadSample(
			HRESULT hrStatus,
			DWORD /* dwStreamIndex */,
			DWORD dwStreamFlags,
			LONGLONG llTimestamp,
			IMFSample* pSample      // Can be NULL
		)
		{
			EnterCriticalSection(&m_CriticalSection);

			if (SUCCEEDED(hrStatus))
			{
				// Since OnReadSample occurs 1:1 with calls to ReadSample, we need to indicate that
				// a "sample" has been provided even though it might be null, so that we issue more
				// ReadSample calls, hence setting m_nQueuedSamples to zero outside the pSample test.
				m_nQueuedSamples = 0;
				if (pSample)
				{
					pSample->AddRef();
					if (m_LastSample)
						m_LastSample->Release();
					m_LastSample = pSample;
				}
			}
			else
			{
				// Streaming error.
				NotifyError(hrStatus);
			}

			if (MF_SOURCE_READERF_ENDOFSTREAM & dwStreamFlags)
			{
				m_bEndOfStream = TRUE;
			}
			m_hrStatus = hrStatus;

			LeaveCriticalSection(&m_CriticalSection);
			return S_OK;
		}

		STDMETHODIMP OnEvent(DWORD, IMFMediaEvent*)
		{
			return S_OK;
		}

		STDMETHODIMP OnFlush(DWORD)
		{
			return S_OK;
		}
	private:
		// Destructor is private. Caller should call Release.
		virtual ~SourceReaderCallback()
		{
			DeleteCriticalSection(&m_CriticalSection);
		}

		void NotifyError(HRESULT hr)
		{
			// TODO: pass on the error condition
		}

		long m_nRefCount;
		CRITICAL_SECTION m_CriticalSection;
		BOOL m_bEndOfStream;
		HRESULT m_hrStatus;
		IMFSample* m_LastSample;
		long m_nQueuedSamples;
	};

	static IMFSourceReader* mfSourceReader = nullptr;
	static UINT32 sourceWidth = 0;
	static UINT32 sourceHeight = 0;
	static SourceReaderCallback* sourceReaderCallback = nullptr;
	static UINT32 sampleCount = 0;
	static std::string deviceName;

	IMFAttributes* mfAttributes = nullptr;
	IMFActivate** mfActivate = nullptr;
	UINT mfActivateCount = 0;
	IMFMediaSource* mfMediaSource = nullptr;
	IMFAttributes* mfAttributes2 = nullptr;
	IMFMediaType* mfNativeMediaType = nullptr;
	IMFMediaType* mfSetMediaType = nullptr;

	void Release()
	{
		if (mfAttributes)
		{
			mfAttributes->Release();
			mfAttributes = nullptr;
		}

		if (mfActivate)
		{
			for (UINT i = 0; i < mfActivateCount; ++i)
				mfActivate[i]->Release();

			CoTaskMemFree(mfActivate); // free array
			mfActivate = nullptr;
		}

		if (mfMediaSource)
		{
			mfMediaSource->Release();
			mfMediaSource = nullptr;
		}

		if (mfAttributes2)
		{
			mfAttributes2->Release();
			mfAttributes2 = nullptr;
		}

		if (sourceReaderCallback)
		{
			sourceReaderCallback->Release();
			sourceReaderCallback = nullptr;
		}

		if (mfSourceReader)
		{
			mfSourceReader->Release();
			mfSourceReader = nullptr;
		}

		if (mfNativeMediaType)
		{
			mfNativeMediaType->Release();
			mfNativeMediaType = nullptr;
		}

		if (mfSetMediaType)
		{
			mfSetMediaType->Release();
			mfSetMediaType = nullptr;
		}
	}

	bool Init()
	{
		if (mfSourceReader != nullptr)
			return true;

		HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
		if (FAILED(hr))
			return false;

		hr = MFStartup(MF_VERSION);
		if (FAILED(hr))
			return false;

		// Create attribute store to query for vidcap (webcam)
		hr = MFCreateAttributes(&mfAttributes, 1);
		if (FAILED(hr))
			return false;

		hr = mfAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
		if (FAILED(hr))
			return false;

		// enumerate and activate the first one.
		hr = MFEnumDeviceSources(mfAttributes, &mfActivate, &mfActivateCount);
		if (FAILED(hr) || mfActivateCount == 0)
			return false;

		hr = mfActivate[0]->ActivateObject(IID_PPV_ARGS(&mfMediaSource));
		if (FAILED(hr))
			return false;

		// Get the device name
		{
			WCHAR* deviceNameW = nullptr;
			UINT32 nameLength = 0;
			hr = mfActivate[0]->GetAllocatedString(
				MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
				&deviceNameW,
				&nameLength
			);
			if (SUCCEEDED(hr))
			{
				deviceName = FromWideString(deviceNameW);
				CoTaskMemFree(deviceNameW); // Free when done
			}
		}

		// Create attribute store to set conversion to RGB32, and use async callback.
		hr = MFCreateAttributes(&mfAttributes2, 1);
		if (FAILED(hr))
			return false;

		hr = mfAttributes2->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
		if (FAILED(hr))
			return false;

		sourceReaderCallback = new SourceReaderCallback();
		hr = mfAttributes2->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, sourceReaderCallback);
		if (FAILED(hr))
			return false;

		hr = MFCreateSourceReaderFromMediaSource(mfMediaSource, mfAttributes2, &mfSourceReader);
		if (FAILED(hr))
			return false;

		hr = mfSourceReader->GetNativeMediaType(0, 0, &mfNativeMediaType);
		if (FAILED(hr))
			return false;

		//GUID guidMajorType;
		//mfNativeMediaType->GetGUID(MF_MT_MAJOR_TYPE, &guidMajorType);
		hr = MFGetAttributeSize(mfNativeMediaType, MF_MT_FRAME_SIZE, &sourceWidth, &sourceHeight);
		if (FAILED(hr))
			return false;

		// Force XRGB because I'm lazy
		hr = MFCreateMediaType(&mfSetMediaType);
		if (FAILED(hr))
			return false;

		hr = mfSetMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
		if (FAILED(hr))
			return false;
		hr = mfSetMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
		if (FAILED(hr))
			return false;
		hr = mfSourceReader->SetCurrentMediaType(0, NULL, mfSetMediaType);
		if (FAILED(hr))
			return false;

		return true;
	}

	std::string GetDeviceName()
	{
		return deviceName;
	}

	bool GetWidthHeight(int* width, int* height)
	{
		if (!mfSourceReader)
			return false;

		*width = sourceWidth;
		*height = sourceHeight;

		return true;
	}

	bool ReadSample(std::vector<unsigned char>* dest)
	{
		if (!mfSourceReader)
			return false;

		IMFSample* mfSample = sourceReaderCallback->GetSample(mfSourceReader);
		if (mfSample != nullptr)
		{
			IMFMediaBuffer* mfMediaBuffer = nullptr;
			mfSample->ConvertToContiguousBuffer(&mfMediaBuffer);
			BYTE* bytes = nullptr;
			DWORD curLength, maxLength;
			mfMediaBuffer->Lock(&bytes, &maxLength, &curLength);
			dest->resize(sourceWidth * sourceHeight * 4);
			// BGRA with zero in alpha -> RGBA
			DWORD* src = (DWORD*)bytes;
			DWORD* dst = (DWORD*)(dest->data());
			for (int i = 0, e = sourceWidth * sourceHeight; i < e; ++i)
			{
				DWORD s = src[i];
				dst[i] = 0xFF000000 | (s >> 16) | (s & 0x00ff00) | ((s & 0xFF) << 16);
			}
			mfMediaBuffer->Unlock();
			mfMediaBuffer->Release();
			mfSample->Release();
			sampleCount++;
			return true;
		}
		else if (sampleCount == 0)
		{
			sampleCount++;
			// Samples are returned asynchronously so immediately after Init we expect ReadSample to
			// not return a result. Return the dimensions and black for the first sample, otherwise
			// the caller marks the file as failed to load.
			dest->resize(sourceWidth * sourceHeight * 4);
			DWORD* dst = (DWORD*)(dest->data());
			for (int i = 0, e = sourceWidth * sourceHeight; i < e; ++i)
			{
				dst[i] = 0xFF000000;
			}
			return true;
		}

		return false;
	}
}

namespace WebCam
{

struct State
{
    int width = 0;
    int height = 0;
    std::vector<unsigned char> pixels;
};

static State s_state;

void Init()
{
	webcam::Init();
}

void Shutdown()
{
	webcam::Release();
}

std::string GetDeviceName()
{
	return webcam::GetDeviceName();
}

bool PreRender(const GGUserFile_WebCam& settings, GigiInterpreterPreviewWindowDX12& interpreter, ID3D12GraphicsCommandList* commandList)
{
	if (!webcam::GetWidthHeight(&s_state.width, &s_state.height))
		return false;

    // Set the size and format of the output texture
    {
        for (auto& pair : interpreter.m_importedResources)
        {
            if (pair.second.nodeIndex == -1 || pair.second.resourceIndex == -1)
                continue;

            if (settings.outputTexture.name == pair.first)
            {
                TextureFormat desiredFormat = TextureFormat::RGBA8_Unorm;

                if (pair.second.texture.format != desiredFormat || pair.second.texture.size[0] != s_state.width || pair.second.texture.size[1] != s_state.height || pair.second.texture.size[2] != 1 || pair.second.resetEveryFrame)
                {
                    pair.second.texture.format = desiredFormat;
					pair.second.texture.size[0] = s_state.width;
					pair.second.texture.size[1] = s_state.height;
					pair.second.texture.size[2] = 1;
					pair.second.resetEveryFrame = false;
                    pair.second.state = GigiInterpreterPreviewWindowDX12::ImportedResourceState::dirty;
                }
                break;
            }
        }
    }

	// Get the resource if we can
	ID3D12Resource* destResource = nullptr;
	{
		bool existsAsTexture = false;
		RuntimeTypes::RenderGraphNode_Resource_Texture& runtimeDataTexture = interpreter.GetRuntimeNodeData_RenderGraphNode_Resource_Texture(settings.outputTexture.name.c_str(), existsAsTexture);
		if (existsAsTexture)
			destResource = runtimeDataTexture.m_resource;
		if (!destResource)
			return true;
	}

	// Read the webcam data if we can. If not, use what we already have from the last frame
	s_state.pixels.resize(s_state.width * s_state.height * 4, 255);
	webcam::ReadSample(&s_state.pixels);

	// Upload the pixels!
	{
		BasicPixScopeProfiler _p(commandList, "Copying webcam input into texture");

		// Transition destination resource to copy dest
		TransitionTracker& transitionTracker = interpreter.GetTransitionsNonConst();
		transitionTracker.Transition(TRANSITION_DEBUG_INFO(destResource, D3D12_RESOURCE_STATE_COPY_DEST));
		transitionTracker.Flush(commandList);

		// get info about layout of texture memory
		D3D12_RESOURCE_DESC desc = destResource->GetDesc();
		std::vector<unsigned char> layoutsMem((sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)));
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutsMem.data();
		unsigned int numRows = 0;
		size_t unalignedPitch = 0;
		interpreter.GetDevice()->GetCopyableFootprints(&desc, 0, 1, 0, layout, &numRows, &unalignedPitch, nullptr);
		size_t alignedPitch = layout->Footprint.RowPitch;
		size_t bufferSize = numRows * alignedPitch * layout->Footprint.Depth;

		// Create an upload buffer
		UploadBufferTracker::Buffer* uploadBuffer = interpreter.getUploadBufferTracker().GetBuffer(interpreter.GetDevice(), bufferSize, false);
		if (!uploadBuffer)
			return false;

		// Write the pixels to the upload buffer
		{
			// map the memory
			D3D12_RANGE readRange;
			readRange.Begin = 1;
			readRange.End = 0;
			unsigned char* destPixels = nullptr;
			HRESULT hr = uploadBuffer->buffer->Map(0, &readRange, reinterpret_cast<void**>(&destPixels));
			if (FAILED(hr))
				return false;

			// fill the upload buffer with pixel data.
			const unsigned char* src = s_state.pixels.data();
			unsigned char* dest = destPixels;

			for (unsigned int i = 0; i < numRows; ++i)
			{
				memcpy(dest, src, unalignedPitch);
				src += unalignedPitch;
				dest += alignedPitch;
			}

			// unmap the memory
			uploadBuffer->buffer->Unmap(0, nullptr);
		}

		// Copy the upload buffer into the resource
		{
			D3D12_TEXTURE_COPY_LOCATION src = {};
			src.pResource = uploadBuffer->buffer;
			src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			src.PlacedFootprint = *layout;

			D3D12_TEXTURE_COPY_LOCATION dest = {};
			dest.pResource = destResource;
			dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			dest.SubresourceIndex = 0;

			commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
		}
	}

	return true;
}

};
