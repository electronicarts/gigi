///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "GigiInterpreterPreviewWindowDX12.h"
#include "DX12Utils/CreateResources.h"
#include "DX12Utils/Utils.h"
#include <d3dx12/d3dx12.h>
#include "NodesShared.h"

GigiInterpreterPreviewWindowDX12* GigiInterpreterPreviewWindowDX12::s_interpreter = nullptr;

void RuntimeTypes::RenderGraphNode_Base::Release(GigiInterpreterPreviewWindowDX12& interpreter)
{
	for (RuntimeTypes::ViewableResource& viewableResource : m_viewableResources)
	{
		interpreter.m_delayedRelease.Add(viewableResource.m_resource);
		interpreter.m_transitions.Untrack(viewableResource.m_resource);
		viewableResource.m_resource = nullptr;

		interpreter.m_delayedRelease.Add(viewableResource.m_resourceReadback);
		interpreter.m_transitions.Untrack(viewableResource.m_resourceReadback);
		viewableResource.m_resourceReadback = nullptr;
	}
}

void RuntimeTypes::RenderGraphNode_Base::OnCompileOK(GigiInterpreterPreviewWindowDX12& interpreter)
{
	Release(interpreter);
	m_viewableResources.clear();
}

void RuntimeTypes::RenderGraphNode_Base::HandleViewableResource(GigiInterpreterPreviewWindowDX12& interpreter, ViewableResource::Type type, const char* displayName, ID3D12Resource* resource, DXGI_FORMAT resourceFormat, int resourceFormatCount, int structIndex, const int _resourceSize[3], int numMips, int stride, int count, bool hideFromUI, bool isResultOfWrite, unsigned int bufferViewBegin, unsigned int bufferViewCount)
{
	// View depth stencil format textures as two resources
	if (resourceFormat == DXGI_FORMAT_D24_UNORM_S8_UINT || resourceFormat == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
	{
		DXGI_FORMAT format1 = (resourceFormat == DXGI_FORMAT_D24_UNORM_S8_UINT) ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		DXGI_FORMAT format2 = (resourceFormat == DXGI_FORMAT_D24_UNORM_S8_UINT) ? DXGI_FORMAT_X24_TYPELESS_G8_UINT : DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;

		std::string longerDisplayName = std::string(displayName) + " (Depth)";
		HandleViewableResource(interpreter, type, longerDisplayName.c_str(), resource, format1, resourceFormatCount, structIndex, _resourceSize, numMips, stride, count, hideFromUI, isResultOfWrite, bufferViewBegin, bufferViewCount);

		longerDisplayName = std::string(displayName) + " (Stencil)";
		HandleViewableResource(interpreter, type, longerDisplayName.c_str(), resource, format2, resourceFormatCount, structIndex, _resourceSize, numMips, stride, count, hideFromUI, isResultOfWrite, bufferViewBegin, bufferViewCount);

		return;
	}

	// get the viewable texture data
	if (m_viewableResourceIndex >= m_viewableResources.size())
		m_viewableResources.resize(m_viewableResourceIndex + 1);
	ViewableResource& res = m_viewableResources[m_viewableResourceIndex];
	m_viewableResourceIndex++;

	res.m_type = type;
	res.m_displayName = displayName;
	res.m_hideFromUI = hideFromUI;
	res.m_isResultOfWrite = isResultOfWrite;
    res.m_bufferViewBegin = bufferViewBegin;
    res.m_bufferViewCount = bufferViewCount;

	// downsize the resource based on mip index
	int resourceSize[3] = { _resourceSize[0], _resourceSize[1], _resourceSize[2] };
	for (int i = 0; i < res.m_mipIndex; ++i)
	{
		resourceSize[0] = max(resourceSize[0] / 2, 1);
		resourceSize[1] = max(resourceSize[1] / 2, 1);

		// 3d textures also get shrank on z axis.
		if (type == ViewableResource::Type::Texture3D)
			resourceSize[2] = max(resourceSize[2] / 2, 1);
	}

	// If it doesn't want to be viewed, we don't care about it, nothing to do
	if (res.m_wantsToBeViewed)
	{
		// Handle copying the resource around, as needed
		if (resource)
		{
			// translate the creation format for stencil bits
			DXGI_FORMAT viewableFormat = resourceFormat;
			if (viewableFormat == DXGI_FORMAT_X32_TYPELESS_G8X24_UINT || viewableFormat == DXGI_FORMAT_X24_TYPELESS_G8_UINT)
				viewableFormat = DXGI_FORMAT_R8_UINT;

			// (re) create the resource if needed
			if (!res.m_resource
				|| res.m_format != viewableFormat || res.m_formatCount != resourceFormatCount || res.m_structIndex != structIndex
				|| res.m_size[0] != resourceSize[0] || res.m_size[1] != resourceSize[1] || res.m_size[2] != resourceSize[2]
				|| res.m_numMips != numMips
				|| res.m_stride != stride || res.m_count != count)
			{
				// free the resources if they already existed
				interpreter.m_delayedRelease.Add(res.m_resource);
				interpreter.m_transitions.Untrack(res.m_resource);
				interpreter.m_delayedRelease.Add(res.m_resourceReadback);
				interpreter.m_transitions.Untrack(res.m_resourceReadback);

				// set the data
				res.m_format = viewableFormat;
				res.m_formatCount = resourceFormatCount;
				res.m_structIndex = structIndex;
				res.m_size[0] = resourceSize[0];
				res.m_size[1] = resourceSize[1];
				res.m_size[2] = resourceSize[2];
				res.m_numMips = numMips;
				res.m_stride = stride;
				res.m_count = count;

				res.m_origResourceDesc = resource->GetDesc();

				// create the new resource
				switch (res.m_type)
				{
					case RuntimeTypes::ViewableResource::Type::Texture2D:
					case RuntimeTypes::ViewableResource::Type::Texture2DArray:
					case RuntimeTypes::ViewableResource::Type::Texture3D:
					case RuntimeTypes::ViewableResource::Type::TextureCube:
                    case RuntimeTypes::ViewableResource::Type::Texture2DMS:
					{
						// Make the view slice
						{
							unsigned int size[3] = { (unsigned int)res.m_size[0], (unsigned int)res.m_size[1], (unsigned int)res.m_size[2] };
							if (res.m_type != RuntimeTypes::ViewableResource::Type::Texture3D)
								size[2] = 1;

							unsigned int sampleCount = 1;
							res.m_resource = CreateTexture(interpreter.m_device, size, 1, res.m_format, sampleCount, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, ResourceType::Texture2D, (std::string(displayName) + " Copy").c_str());
							interpreter.m_transitions.Track(TRANSITION_DEBUG_INFO(res.m_resource, D3D12_RESOURCE_STATE_COPY_DEST));
						}

						// Make a readback buffer that can hold the entire source resource
						{
							D3D12_RESOURCE_DESC resourceDesc = resource->GetDesc();

							unsigned int numSubResources = (res.m_type == RuntimeTypes::ViewableResource::Type::Texture3D)
								? resourceDesc.MipLevels
								: resourceDesc.MipLevels * resourceDesc.DepthOrArraySize;

							size_t totalBytes = 0;
							interpreter.m_device->GetCopyableFootprints(&resourceDesc, 0, numSubResources, 0, nullptr, nullptr, nullptr, &totalBytes);

							res.m_resourceReadback = CreateBuffer(interpreter.m_device, (unsigned int)totalBytes, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_READBACK, (std::string(displayName) + " Readback").c_str());
							interpreter.m_transitions.Track(TRANSITION_DEBUG_INFO(res.m_resourceReadback, D3D12_RESOURCE_STATE_COPY_DEST));
						}
						break;
					}
					case RuntimeTypes::ViewableResource::Type::ConstantBuffer:
					{
						res.m_resource = CreateBuffer(interpreter.m_device, res.m_size[0], D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (std::string(displayName) + " Copy").c_str());
						interpreter.m_transitions.Track(TRANSITION_DEBUG_INFO(res.m_resource, D3D12_RESOURCE_STATE_COMMON));

						res.m_resourceReadback = CreateBuffer(interpreter.m_device, res.m_size[0], D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_READBACK, (std::string(displayName) + " Readback").c_str());
						interpreter.m_transitions.Track(TRANSITION_DEBUG_INFO(res.m_resourceReadback, D3D12_RESOURCE_STATE_COPY_DEST));

						break;
					}
					case RuntimeTypes::ViewableResource::Type::Buffer:
					{
						res.m_resource = CreateBuffer(interpreter.m_device, res.m_size[0], D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT, (std::string(displayName) + " Copy").c_str());
						interpreter.m_transitions.Track(TRANSITION_DEBUG_INFO(res.m_resource, D3D12_RESOURCE_STATE_COMMON));

						res.m_resourceReadback = CreateBuffer(interpreter.m_device, res.m_size[0], D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_READBACK, (std::string(displayName) + " Readback").c_str());
						interpreter.m_transitions.Track(TRANSITION_DEBUG_INFO(res.m_resourceReadback, D3D12_RESOURCE_STATE_COPY_DEST));

						break;
					}
					default:
					{
						return;
					}
				}
			}

			// Do copy for viewing
			{
                char profilerLabel[1024];
                sprintf_s(profilerLabel, "Copy For Viewing: %s", displayName);
                BasicPixScopeProfiler _p(interpreter.m_commandList, profilerLabel);

				// Transition
				interpreter.m_transitions.Transition(TRANSITION_DEBUG_INFO(resource, D3D12_RESOURCE_STATE_COPY_SOURCE));
				interpreter.m_transitions.Transition(TRANSITION_DEBUG_INFO(res.m_resource, D3D12_RESOURCE_STATE_COPY_DEST));
				interpreter.m_transitions.Flush(interpreter.m_commandList);

				// make sure these are in range
				res.m_arrayIndex = min(res.m_arrayIndex, res.m_size[2] - 1);
				res.m_mipIndex = min(res.m_mipIndex, res.m_numMips - 1);

				// multi dimensional textures only get a 2d slice
				if (res.m_type == RuntimeTypes::ViewableResource::Type::Texture3D)
				{
					DXGI_FORMAT_Info resourceFormatInfo = Get_DXGI_FORMAT_Info(resourceFormat);

					D3D12_BOX srcBox;
					srcBox.left = 0;
					srcBox.right = res.m_size[0];
					srcBox.top = 0;
					srcBox.bottom = res.m_size[1];
					srcBox.front = res.m_arrayIndex;
					srcBox.back = res.m_arrayIndex + 1;

					if (resourceFormatInfo.isCompressed)
					{
						srcBox.left = ALIGN(4, srcBox.left);
						srcBox.right = ALIGN(4, srcBox.right);
						srcBox.top = ALIGN(4, srcBox.top);
						srcBox.bottom = ALIGN(4, srcBox.bottom);
					}

					D3D12_TEXTURE_COPY_LOCATION src = {};
					src.pResource = resource;
					src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
					src.SubresourceIndex = res.m_mipIndex;

					D3D12_TEXTURE_COPY_LOCATION dest = {};
					dest.pResource = res.m_resource;
					dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
					dest.SubresourceIndex = 0;

					interpreter.m_commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, &srcBox);
				}
				else if (res.m_type == RuntimeTypes::ViewableResource::Type::Texture2DArray || res.m_type == RuntimeTypes::ViewableResource::Type::TextureCube || res.m_type == RuntimeTypes::ViewableResource::Type::Texture2D || res.m_type == RuntimeTypes::ViewableResource::Type::Texture2DMS)
				{
					DXGI_FORMAT_Info resourceFormatInfo = Get_DXGI_FORMAT_Info(resourceFormat);

					D3D12_TEXTURE_COPY_LOCATION src = {};
					src.pResource = resource;
					src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
					src.SubresourceIndex = D3D12CalcSubresource(res.m_mipIndex, res.m_arrayIndex, resourceFormatInfo.planeIndex, res.m_numMips, res.m_size[2]);

					D3D12_TEXTURE_COPY_LOCATION dest = {};
					dest.pResource = res.m_resource;
					dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
					dest.SubresourceIndex = 0;

					// MSAA preview is not implemented yet.
					// D3D12 ERROR : ID3D12CommandList::CopyTextureRegion : The destination resource multisampling properties must equal the source resource.The destination resource has 1 samples and 0 quality.The source resource has 8 samples and 0 quality.[RESOURCE_MANIPULATION ERROR #849: COPYTEXTUREREGION_INVALIDDSTRESOURCE]
					if(res.m_type != RuntimeTypes::ViewableResource::Type::Texture2DMS)
					    interpreter.m_commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
				}
				else
				{
					// For buffer / constant buffer
					interpreter.m_commandList->CopyResource(res.m_resource, resource);
				}

				// put the destination into pixel shader resource so it can be used by imgui
				interpreter.m_transitions.Transition(TRANSITION_DEBUG_INFO(res.m_resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
			}

			// do copy for readback
			if (res.m_wantsToBeReadBack)
			{
                char profilerLabel[1024];
                sprintf_s(profilerLabel, "Copy For Readback: %s", displayName);
                BasicPixScopeProfiler _p(interpreter.m_commandList, profilerLabel);

				interpreter.m_transitions.Transition(TRANSITION_DEBUG_INFO(resource, D3D12_RESOURCE_STATE_COPY_SOURCE));
				interpreter.m_transitions.Transition(TRANSITION_DEBUG_INFO(res.m_resourceReadback, D3D12_RESOURCE_STATE_COPY_DEST));
				interpreter.m_transitions.Flush(interpreter.m_commandList);

				if (res.m_type == RuntimeTypes::ViewableResource::Type::Texture2D ||
					res.m_type == RuntimeTypes::ViewableResource::Type::Texture2DArray ||
					res.m_type == RuntimeTypes::ViewableResource::Type::Texture3D ||
                    res.m_type == RuntimeTypes::ViewableResource::Type::TextureCube ||
                    res.m_type == RuntimeTypes::ViewableResource::Type::Texture2DMS)
				{
					// Calculate the number of sub resources
					D3D12_RESOURCE_DESC resourceDesc = resource->GetDesc();

					unsigned int numSubResources = (res.m_type == RuntimeTypes::ViewableResource::Type::Texture3D)
						? resourceDesc.MipLevels
						: resourceDesc.MipLevels * resourceDesc.DepthOrArraySize;

					std::vector<unsigned char> layoutsMem((sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * numSubResources);
					D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutsMem.data();
					interpreter.m_device->GetCopyableFootprints(&resourceDesc, 0, numSubResources, 0, layouts, nullptr, nullptr, nullptr);

					for (unsigned int subResourceIndex = 0; subResourceIndex < numSubResources; ++subResourceIndex)
					{
						D3D12_TEXTURE_COPY_LOCATION src = {};
						src.pResource = resource;
						src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
						src.SubresourceIndex = subResourceIndex;

						D3D12_TEXTURE_COPY_LOCATION dest = {};
						dest.pResource = res.m_resourceReadback;
						dest.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
						dest.PlacedFootprint = layouts[subResourceIndex];

						// MSAA preview is not implemented yet.
						// D3D12 ERROR : ID3D12CommandList::CopyTextureRegion : The destination resource multisampling properties must equal the source resource.The destination resource has 1 samples and 0 quality.The source resource has 8 samples and 0 quality.[RESOURCE_MANIPULATION ERROR #849: COPYTEXTUREREGION_INVALIDDSTRESOURCE]
						if( res.m_type != RuntimeTypes::ViewableResource::Type::Texture2DMS)
							interpreter.m_commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
					}
				}
				else
				{
					interpreter.m_commandList->CopyResource(res.m_resourceReadback, resource);
				}
			}
		}

		// Mark this as no longer wanting to be viewed or read back. It's up to the consumer to turn this flag on again for next frame.
		res.m_wantsToBeViewed = false;
		res.m_wantsToBeReadBack = false;
	}

	// If we have a GPU write to do, let's do that
	if (interpreter.m_GPUResourceWrites.count(displayName) != 0)
	{
		if (!resource)
		{
			interpreter.m_logFn(LogLevel::Warn, "GPU Resource write to \"%s\" failed because the resource doesn't exist", displayName);
			return;
		}

		const GigiInterpreterPreviewWindowDX12::GPUResourceWrite& write = interpreter.m_GPUResourceWrites[displayName];

		// If this is a texture, we need to set the aligned and unaligned pitch correctly
		int uploadBufferUnalignedPitch = 0;
		int uploadBufferAlignedPitch = 0;
		int uploadBufferRowCount = 0;
		if (res.m_type == RuntimeTypes::ViewableResource::Type::Texture2D ||
			res.m_type == RuntimeTypes::ViewableResource::Type::Texture2DArray ||
			res.m_type == RuntimeTypes::ViewableResource::Type::Texture3D ||
			res.m_type == RuntimeTypes::ViewableResource::Type::TextureCube ||
            res.m_type == RuntimeTypes::ViewableResource::Type::Texture2DMS)
		{
			DXGI_FORMAT_Info resourceFormatInfo = Get_DXGI_FORMAT_Info(resourceFormat);
			uploadBufferUnalignedPitch = resourceSize[0] * resourceFormatInfo.bytesPerPixel;
			uploadBufferAlignedPitch = ALIGN(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT, uploadBufferUnalignedPitch);

			if (res.m_type == RuntimeTypes::ViewableResource::Type::Texture3D)
				uploadBufferRowCount = resourceSize[1] * resourceSize[2];
			else
				uploadBufferRowCount = resourceSize[1];
		}
		// else, set it up to do a single memcpy
		else
		{
			uploadBufferUnalignedPitch = (int)write.data.size();
			uploadBufferAlignedPitch = uploadBufferUnalignedPitch;
			uploadBufferRowCount = 1;
		}
		size_t uploadBufferSize = uploadBufferAlignedPitch * uploadBufferRowCount;

		// make an upload buffer
		UploadBufferTracker::Buffer* uploadBuffer = interpreter.m_uploadBufferTracker.GetBuffer(interpreter.m_device, uploadBufferSize, false);

		// write into it!
		{
			void* mappedDest = nullptr;
			HRESULT hr = uploadBuffer->buffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedDest));
			if (SUCCEEDED(hr))
			{
				// Fill it with zeros to make padding for alignment deterministic
				memset(mappedDest, 0, uploadBufferSize);

				// copy each aligned stride
				for (int rowIndex = 0; rowIndex < uploadBufferRowCount; ++rowIndex)
				{
					const char* src = &write.data[rowIndex * uploadBufferUnalignedPitch];
					char* dest = &((char*)mappedDest)[rowIndex * uploadBufferAlignedPitch];
					memcpy(dest, src, uploadBufferUnalignedPitch);
				}
				uploadBuffer->buffer->Unmap(0, nullptr);
			}
		}

		// make sure the resource is ready to be coppied into
		interpreter.m_transitions.Transition(TRANSITION_DEBUG_INFO(resource, D3D12_RESOURCE_STATE_COPY_DEST));
		interpreter.m_transitions.Flush(interpreter.m_commandList);

		// copy to the destination
		if (res.m_type == RuntimeTypes::ViewableResource::Type::Texture3D)
		{
			D3D12_RESOURCE_DESC resourceDesc = resource->GetDesc();
			resourceDesc.DepthOrArraySize = resourceSize[2];
			std::vector<unsigned char> layoutMem(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64));
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutMem.data();
			interpreter.m_device->GetCopyableFootprints(&resourceDesc, 0, 1, 0, layout, nullptr, nullptr, nullptr);

			D3D12_TEXTURE_COPY_LOCATION src = {};
			src.pResource = resource;
			src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			src.SubresourceIndex = 0;

			D3D12_TEXTURE_COPY_LOCATION dest = {};
			dest.pResource = uploadBuffer->buffer;
			dest.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			dest.PlacedFootprint = *layout;

			interpreter.m_commandList->CopyTextureRegion(&src, 0, 0, 0, &dest, nullptr);
		}
		else if (res.m_type == RuntimeTypes::ViewableResource::Type::Texture2D ||
			res.m_type == RuntimeTypes::ViewableResource::Type::Texture2DArray ||
			res.m_type == RuntimeTypes::ViewableResource::Type::TextureCube ||
			res.m_type == RuntimeTypes::ViewableResource::Type::Texture2DMS)
		{
			D3D12_RESOURCE_DESC resourceDesc = resource->GetDesc();
			std::vector<unsigned char> layoutMem(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64));
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutMem.data();
			interpreter.m_device->GetCopyableFootprints(&resourceDesc, 0, 1, 0, layout, nullptr, nullptr, nullptr);

			D3D12_TEXTURE_COPY_LOCATION dest = {};
			dest.pResource = resource;
			dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			dest.SubresourceIndex = write.subresourceIndex;

			D3D12_TEXTURE_COPY_LOCATION src = {};
			src.pResource = uploadBuffer->buffer;
			src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			src.PlacedFootprint = *layout;

			interpreter.m_commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
		}
		else
		{
			interpreter.m_commandList->CopyResource(resource, uploadBuffer->buffer);
		}
	}
}

std::vector<RuntimeTypes::ViewableResource*> GigiInterpreterPreviewWindowDX12::MarkShaderAssertsForReadback()
{
	std::vector<RuntimeTypes::ViewableResource*> assertsBuffers;
	const size_t nodesCount = m_renderGraph.nodes.size();
	for (size_t i = 0; i < nodesCount; ++i)
	{
		std::vector<RuntimeTypes::ViewableResource>* viewableResources = nullptr;
		RuntimeNodeDataLambda(
			m_renderGraph.nodes[i],
			[&](auto node, auto* runtimeData)
			{
				std::string renderGraphText;

				if (runtimeData)
					viewableResources = &(runtimeData->m_viewableResources);
			}
		);

		if (!viewableResources)
			continue;

		for (RuntimeTypes::ViewableResource& res : *viewableResources)
		{
			std::string_view displayName = res.m_displayName;
			const bool isAssertBuf = displayName.find("__GigiAssertUAV") != displayName.npos;
			const bool isAfter = displayName.ends_with("(UAV - After)");
			if (isAssertBuf && isAfter)
			{
				res.m_wantsToBeViewed = true;
				res.m_wantsToBeReadBack = true;

				assertsBuffers.push_back(&res);
			}
		}
	}

	return assertsBuffers;
}

void GigiInterpreterPreviewWindowDX12::CollectShaderAsserts(const std::vector<RuntimeTypes::ViewableResource*>& assertsBuffers)
{
	collectedAsserts.clear();

	for (const RuntimeTypes::ViewableResource* res : assertsBuffers)
	{
		if (res->m_resourceReadback)
		{
			std::vector<unsigned char> bytes(res->m_size[0]);
			unsigned char* data = nullptr;
			res->m_resourceReadback->Map(0, nullptr, reinterpret_cast<void**>(&data));
			if (!data)
				continue;

			memcpy(bytes.data(), data, res->m_size[0]);
			res->m_resourceReadback->Unmap(0, nullptr);

			const uint32_t* assertBufData = reinterpret_cast<uint32_t*>(bytes.data());
			const uint32_t isFired = assertBufData[0];
			const uint32_t fmtId = assertBufData[1];

			const std::vector<std::string>& fmtStrings = m_renderGraph.assertsFormatStrings;
			if (fmtId >= fmtStrings.size())
			{
				GigiAssert(false, "OOB in asserts format strings array");
				return;
			}

			const char* fmt = fmtStrings[fmtId].c_str();
			std::unordered_set<std::string>& firedAsserts = m_renderGraph.firedAssertsIdentifiers;
			const bool isNewAssert = firedAsserts.find(res->m_displayName) == firedAsserts.end();

			if (isFired && isNewAssert)
			{
				firedAsserts.insert(res->m_displayName);

				std::string context = res->m_displayName.substr(0, res->m_displayName.find(':'));

				const float* v = reinterpret_cast<float*>(bytes.data() + 2 * sizeof(uint32_t));

				const int fmtSize = std::snprintf(nullptr, 0, fmt, v[0], v[1], v[2], v[3], v[4], v[5]);
				if (fmtSize <= 0)
				{
					GigiAssert(false, "Failed to format the assert message '%s'", fmt);
					return;
				}

				std::string assertMsg;
				assertMsg.resize(fmtSize+1);
				std::snprintf(assertMsg.data(), fmtSize+1, fmt, v[0], v[1], v[2], v[3], v[4], v[5]);

				collectedAsserts.push_back({fmtId, fmt, std::move(context), std::move(assertMsg)});
			}
		}
	}
}

void GigiInterpreterPreviewWindowDX12::LogCollectedShaderAsserts() const
{
	for (const FiredAssertInfo& a : collectedAsserts)
	{
		std::string msgHeader = "ASSERT FAILED: [" + a.displayName + "]";
		ShowErrorMessage("%s\nmsg: %s", msgHeader.data(), a.msg.data());
	}
}


bool GigiInterpreterPreviewWindowDX12::BuildDescriptorRanges(const Shader* shader, std::vector<D3D12_DESCRIPTOR_RANGE>& outRanges)
{
    GigiAssert(outRanges.empty(), "Don't call BuildDescriptorRanges() twice and don't reuse the array");

    if (!shader)
        return true;    // no shader means no output needed so no error

    const std::vector<ShaderResource>& resources = shader->resources;

    for (const ShaderResource& resource : resources)
    {
        D3D12_DESCRIPTOR_RANGE desc;

        switch (resource.access)
        {
        case ShaderResourceAccessType::UAV: desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV; break;
        case ShaderResourceAccessType::RTScene:
        case ShaderResourceAccessType::SRV: desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; break;
        case ShaderResourceAccessType::CBV: desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; break;
        default:
        {
            m_logFn(LogLevel::Error, "Unhandled resource access type (%s)\n", EnumToString(resource.access));
            return false;
        }
        }

        desc.NumDescriptors = 1;
        desc.BaseShaderRegister = resource.registerIndex;
        desc.RegisterSpace = 0;
        desc.OffsetInDescriptorsFromTableStart = (UINT)outRanges.size();

        outRanges.push_back(desc);
    }

    // for NVAPI
    extern bool g_nvInitialized;
    if (g_nvInitialized)
    {
        D3D12_DESCRIPTOR_RANGE desc;
        desc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        desc.NumDescriptors = 1;
        desc.BaseShaderRegister = 0;
        // space4318 = can be any number but must be the same in HLSL, here: nvidia hardware vendor id 0x10de=4318
        desc.RegisterSpace = 4318;
        desc.OffsetInDescriptorsFromTableStart = (UINT)outRanges.size();
        outRanges.push_back(desc);
    }

    return true;
}

bool GigiInterpreterPreviewWindowDX12::MakeDescriptorTableDesc(
    std::vector<DescriptorTableCache::ResourceDescriptor>& descs,
    struct RuntimeTypes::RenderGraphNode_Base& runtimeData,
    const std::vector<ResourceDependency>& resourceDependencies,
    const std::vector<LinkProperties>& linkProperties,
    const char* nodeName,
    const Shader* shaderPtr,
    int& pinOffset,
    std::vector<TransitionTracker::Item>& queuedTransitions,
const std::unordered_map<ID3D12Resource*, D3D12_RESOURCE_STATES>& importantResourceStates)
{
    GigiAssert(nodeName != 0, "expected from caller");

    if (!shaderPtr)
         return true;

    const Shader& shader = *shaderPtr;

    for (int resourceIndex = 0; resourceIndex < shader.resources.size(); ++resourceIndex)
	{
		const ShaderResource& shaderResource = shader.resources[resourceIndex];

		int depIndex = 0;
		while (depIndex < resourceDependencies.size() && resourceDependencies[depIndex].pinIndex != (resourceIndex+pinOffset))
			depIndex++;

		if (depIndex >= resourceDependencies.size())
		{
			m_logFn(LogLevel::Error, "Could not find resource dependency for shader resource \"%s\" in node \"%s\"", shaderResource.name.c_str(), nodeName);
			return false;
		}
		const ResourceDependency& dep = resourceDependencies[depIndex];

		DescriptorTableCache::ResourceDescriptor desc;

		const RenderGraphNode& resourceNode = m_renderGraph.nodes[dep.nodeIndex];
		switch (resourceNode._index)
		{
			case RenderGraphNode::c_index_resourceTexture:
			{
				const RuntimeTypes::RenderGraphNode_Resource_Texture& resourceInfo =  GetRuntimeNodeData_RenderGraphNode_Resource_Texture(resourceNode.resourceTexture.name.c_str());
				desc.m_resource = resourceInfo.m_resource;
				desc.m_format = resourceInfo.m_format;

				if (dep.pinIndex < linkProperties.size())
				{
					desc.m_UAVMipIndex = min(linkProperties[dep.pinIndex].UAVMipIndex, resourceInfo.m_numMips - 1);
				}

				switch (resourceNode.resourceTexture.dimension)
				{
					case TextureDimensionType::Texture2D: desc.m_resourceType = DescriptorTableCache::ResourceType::Texture2D; break;
					case TextureDimensionType::Texture2DArray: desc.m_resourceType = DescriptorTableCache::ResourceType::Texture2DArray; desc.m_count = resourceInfo.m_size[2]; break;
					case TextureDimensionType::Texture3D: desc.m_resourceType = DescriptorTableCache::ResourceType::Texture3D; desc.m_count = resourceInfo.m_size[2]; break;
					case TextureDimensionType::TextureCube: desc.m_resourceType = DescriptorTableCache::ResourceType::TextureCube; desc.m_count = 6; break;
					case TextureDimensionType::Texture2DMS: desc.m_resourceType = DescriptorTableCache::ResourceType::Texture2DMS; break;
					default:
						GigiAssert(0, "Unknown TextureDimensionType type");
				}
				break;
			}
			case RenderGraphNode::c_index_resourceShaderConstants:
			{
				const RuntimeTypes::RenderGraphNode_Resource_ShaderConstants& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_ShaderConstants(resourceNode.resourceShaderConstants.name.c_str());
				desc.m_resource = resourceInfo.m_buffer->buffer;
				desc.m_format = DXGI_FORMAT_UNKNOWN;
				desc.m_stride = (UINT)resourceInfo.m_buffer->size;
				desc.m_count = 1;
				break;
			}
			case RenderGraphNode::c_index_resourceBuffer:
			{
                RuntimeTypes::RenderGraphNode_Resource_Buffer& resourceInfo = GetRuntimeNodeData_RenderGraphNode_Resource_Buffer(resourceNode.resourceBuffer.name.c_str());

                if (dep.access == ShaderResourceAccessType::RTScene)
                {
                    if (!AccessIsReadOnly(resourceNode.resourceBuffer.accessedAs))
                    {
	                    m_transitions.Untrack(resourceInfo.m_tlas);
	                    m_delayedRelease.Add(resourceInfo.m_tlas);
	                    resourceInfo.m_tlas = nullptr;
	                    resourceInfo.m_tlasSize = 0;

	                    m_transitions.Untrack(resourceInfo.m_blas);
	                    m_delayedRelease.Add(resourceInfo.m_blas);
	                    resourceInfo.m_blas = nullptr;
	                    resourceInfo.m_blasSize = 0;

	                    m_transitions.Untrack(resourceInfo.m_instanceDescs);
	                    m_delayedRelease.Add(resourceInfo.m_instanceDescs);
	                    resourceInfo.m_instanceDescs = nullptr;
                    }

                    if (!resourceInfo.m_tlas)
                    {
	                    if (!MakeAccelerationStructures(resourceNode.resourceBuffer))
	                    {
					        m_logFn(LogLevel::Error, "Failed to make acceleration structure for buffer \"%s\"", resourceNode.resourceBuffer.name.c_str());
					        return false;
	                    }
                    }
 
					desc.m_resource = resourceInfo.m_tlas;
					desc.m_format = DXGI_FORMAT_UNKNOWN;
					desc.m_stride = resourceInfo.m_tlasSize;
					desc.m_count = 1;
					desc.m_raw = false;
				}
				else
				{
					desc.m_resource = resourceInfo.m_resource;

					const ShaderResourceBuffer& shaderResourceBuffer = shader.resources[resourceIndex].buffer;
					bool isStructuredBuffer = ShaderResourceBufferIsStructuredBuffer(shaderResourceBuffer);
					if (isStructuredBuffer)
					{
						desc.m_format = DXGI_FORMAT_UNKNOWN;
						if (shaderResourceBuffer.typeStruct.structIndex != -1)
							desc.m_stride = (UINT)m_renderGraph.structs[shaderResourceBuffer.typeStruct.structIndex].sizeInBytes;
						else
							desc.m_stride = DataFieldTypeInfo(shaderResourceBuffer.type).typeBytes;
						desc.m_count = resourceInfo.m_size / desc.m_stride;
					}
					else
					{
						desc.m_format = DataFieldTypeInfoDX12(shaderResourceBuffer.type).typeFormat;
						desc.m_stride = 0;
						desc.m_count = resourceInfo.m_count;
					}

                    if (dep.pinIndex < linkProperties.size())
                    {
                        const LinkProperties& linkProperty = linkProperties[dep.pinIndex];

                        unsigned int unitsDivider = 1;
                        if (linkProperty.bufferViewUnits != MemoryUnitOfMeasurement::Items)
                        {
                            unitsDivider = (shaderResourceBuffer.typeStruct.structIndex != -1)
                                ? (UINT)m_renderGraph.structs[shaderResourceBuffer.typeStruct.structIndex].sizeInBytes
                                : DataFieldTypeInfoDX12(shaderResourceBuffer.type).typeBytes;
                            unitsDivider = max(unitsDivider, 1);
                        }

                        unsigned int bufferViewBegin = linkProperty.bufferViewBegin;
                        if (linkProperty.bufferViewBeginVariable.variableIndex != -1)
                        {
                            if (!GetRuntimeVariableAllowCast(linkProperty.bufferViewBeginVariable.variableIndex, bufferViewBegin))
                                return false;
                        }

                        unsigned int bufferViewSize = linkProperty.bufferViewSize;
                        if (linkProperty.bufferViewSizeVariable.variableIndex != -1)
                        {
                            if (!GetRuntimeVariableAllowCast(linkProperty.bufferViewSizeVariable.variableIndex, bufferViewSize))
                                return false;
                        }

                        desc.m_firstElement = bufferViewBegin / unitsDivider;

                        if (desc.m_count >= desc.m_firstElement)
                            desc.m_count -= desc.m_firstElement;
                        else
                            desc.m_count = 0;

                        unsigned int bufferViewNumElements = bufferViewSize / unitsDivider;
                        if (bufferViewNumElements > 0)
                            desc.m_count = min(desc.m_count, bufferViewNumElements);
                    }

					desc.m_raw = shader.resources[resourceIndex].buffer.raw;
				}
				break;
			}
			default:
			{
				m_logFn(LogLevel::Error, "Unhandled dependency node type for node \"%s\"", nodeName);
				return false;
			}
		}

		// This could be a temporary thing, but we can't run the pass if we don't have the resources we need.
        if (!desc.m_resource)
        {
            std::ostringstream ss;
            ss << "Cannot run due to resource not existing:\n" << GetNodeTypeString(resourceNode) << " \"" << GetNodeName(resourceNode) << "\" (\"" << GetNodeOriginalName(resourceNode) << "\")";
            runtimeData.m_renderGraphText = ss.str();
            runtimeData.m_inErrorState = true;
            return false;
        }

		switch (dep.access)
		{
			case ShaderResourceAccessType::UAV:
			{
				desc.m_access = DescriptorTableCache::AccessType::UAV;
				queuedTransitions.push_back({ TRANSITION_DEBUG_INFO_NAMED(desc.m_resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, GetNodeName(resourceNode).c_str()) });
				break;
			}
			case ShaderResourceAccessType::RTScene:
			{
				desc.m_access = DescriptorTableCache::AccessType::SRV;
				queuedTransitions.push_back({ TRANSITION_DEBUG_INFO_NAMED(desc.m_resource, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, GetNodeName(resourceNode).c_str()) });
				break;
			}
			case ShaderResourceAccessType::SRV:
			{
				desc.m_access = DescriptorTableCache::AccessType::SRV;

				if (importantResourceStates.count(desc.m_resource) == 0)
					queuedTransitions.push_back({ TRANSITION_DEBUG_INFO_NAMED(desc.m_resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, GetNodeName(resourceNode).c_str()) });

				break;
			}
			case ShaderResourceAccessType::CBV:
			{
				// constant buffers are upload heap, don't need transitions to be written from CPU or read by shaders
				desc.m_access = DescriptorTableCache::AccessType::CBV;
				break;
			}
			case ShaderResourceAccessType::Indirect:
			{
				// This is handled elsewhere
				continue;
			}
			default:
			{
				m_logFn(LogLevel::Error, "Unhandled shader resource access type \"%s\" for node \"%s\"", EnumToString(dep.access), nodeName);
				return false;
			}
		}

		switch (dep.type)
		{
			case ShaderResourceType::Texture: break;// Handled above
			case ShaderResourceType::Buffer:
			{
				if (dep.access == ShaderResourceAccessType::RTScene)
					desc.m_resourceType = DescriptorTableCache::ResourceType::RTScene;
				else
					desc.m_resourceType = DescriptorTableCache::ResourceType::Buffer;
				break;
			}
			case ShaderResourceType::ConstantBuffer: desc.m_resourceType = DescriptorTableCache::ResourceType::Buffer; break;
			default:
			{
				m_logFn(LogLevel::Error, "Unhandled shader resource type \"%s\" for node \"%s\"", EnumToString(dep.type), nodeName);
				return false;
			}
		}

		descs.push_back(desc);
	}

    pinOffset += (int)shader.resources.size();

    extern bool g_nvInitialized;
    if (g_nvInitialized)
    {
        // dummy UAV needed for NVAPI back door, see space4318
        DescriptorTableCache::ResourceDescriptor desc;
        desc.m_access = DescriptorTableCache::AccessType::UAV;
        desc.m_resourceType = DescriptorTableCache::ResourceType::Buffer;
        // prevent Format can't be used for UAVs
        desc.m_format = DXGI_FORMAT_UNKNOWN;
        desc.m_count = 1;
        descs.push_back(desc);
    }

	return true;
}