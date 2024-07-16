///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#include "GigiInterpreterPreviewWindowDX12.h"
#include "DX12Utils/CreateResources.h"
#include "DX12Utils/Utils.h"
#include <d3dx12/d3dx12.h>

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

void RuntimeTypes::RenderGraphNode_Base::HandleViewableResource(GigiInterpreterPreviewWindowDX12& interpreter, ViewableResource::Type type, const char* displayName, ID3D12Resource* resource, DXGI_FORMAT resourceFormat, int resourceFormatCount, int structIndex, const int _resourceSize[3], int numMips, int stride, int count, bool hideFromUI, bool isResultOfWrite)
{
	// View depth stencil format textures as two resources
	if (resourceFormat == DXGI_FORMAT_D24_UNORM_S8_UINT || resourceFormat == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
	{
		DXGI_FORMAT format1 = (resourceFormat == DXGI_FORMAT_D24_UNORM_S8_UINT) ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		DXGI_FORMAT format2 = (resourceFormat == DXGI_FORMAT_D24_UNORM_S8_UINT) ? DXGI_FORMAT_X24_TYPELESS_G8_UINT : DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;

		std::string longerDisplayName = std::string(displayName) + " (Depth)";
		HandleViewableResource(interpreter, type, longerDisplayName.c_str(), resource, format1, resourceFormatCount, structIndex, _resourceSize, numMips, stride, count, hideFromUI, isResultOfWrite);

		longerDisplayName = std::string(displayName) + " (Stencil)";
		HandleViewableResource(interpreter, type, longerDisplayName.c_str(), resource, format2, resourceFormatCount, structIndex, _resourceSize, numMips, stride, count, hideFromUI, isResultOfWrite);

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
			if (viewableFormat == DXGI_FORMAT_X32_TYPELESS_G8X24_UINT)
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

				// create the new resource
				switch (res.m_type)
				{
					case RuntimeTypes::ViewableResource::Type::Texture2D:
					case RuntimeTypes::ViewableResource::Type::Texture2DArray:
					case RuntimeTypes::ViewableResource::Type::Texture3D:
					case RuntimeTypes::ViewableResource::Type::TextureCube:
					{
						unsigned int size[3] = { (unsigned int)res.m_size[0], (unsigned int)res.m_size[1], (unsigned int)res.m_size[2] };
						if (res.m_type != RuntimeTypes::ViewableResource::Type::Texture3D)
							size[2] = 1;

						res.m_resource = CreateTexture(interpreter.m_device, size, 1, res.m_format, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, ResourceType::Texture2D, (std::string(displayName) + " Copy").c_str());
						interpreter.m_transitions.Track(TRANSITION_DEBUG_INFO(res.m_resource, D3D12_RESOURCE_STATE_COPY_DEST));

						// Make a readback buffer
						DXGI_FORMAT_Info viewableFormatInfo = Get_DXGI_FORMAT_Info(viewableFormat);
						int unalignedPitch = res.m_size[0] * viewableFormatInfo.bytesPerPixel;
						int alignedPitch = ALIGN((D3D12_TEXTURE_DATA_PITCH_ALIGNMENT * viewableFormatInfo.planeCount), unalignedPitch);

						res.m_resourceReadback = CreateBuffer(interpreter.m_device, alignedPitch * res.m_size[1] * res.m_size[2], D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_READBACK, (std::string(displayName) + " Readback").c_str());
						interpreter.m_transitions.Track(TRANSITION_DEBUG_INFO(res.m_resourceReadback, D3D12_RESOURCE_STATE_COPY_DEST));
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
				// Transition
				interpreter.m_transitions.Transition(TRANSITION_DEBUG_INFO(resource, D3D12_RESOURCE_STATE_COPY_SOURCE));
				interpreter.m_transitions.Transition(TRANSITION_DEBUG_INFO(res.m_resource, D3D12_RESOURCE_STATE_COPY_DEST));
				interpreter.m_transitions.Flush(interpreter.m_commandList);

				// multi dimensional textures only get a 2d slice
				if (res.m_type == RuntimeTypes::ViewableResource::Type::Texture3D)
				{
					res.m_arrayIndex = min(res.m_arrayIndex, res.m_size[2] - 1);

					D3D12_BOX srcBox;
					srcBox.left = 0;
					srcBox.right = res.m_size[0];
					srcBox.top = 0;
					srcBox.bottom = res.m_size[1];
					srcBox.front = res.m_arrayIndex;
					srcBox.back = res.m_arrayIndex + 1;

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
				else if (res.m_type == RuntimeTypes::ViewableResource::Type::Texture2DArray || res.m_type == RuntimeTypes::ViewableResource::Type::TextureCube || res.m_type == RuntimeTypes::ViewableResource::Type::Texture2D)
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
				interpreter.m_transitions.Transition(TRANSITION_DEBUG_INFO(resource, D3D12_RESOURCE_STATE_COPY_SOURCE));
				interpreter.m_transitions.Transition(TRANSITION_DEBUG_INFO(res.m_resourceReadback, D3D12_RESOURCE_STATE_COPY_DEST));
				interpreter.m_transitions.Flush(interpreter.m_commandList);

				if (res.m_type == RuntimeTypes::ViewableResource::Type::Texture3D)
				{
					DXGI_FORMAT_Info resourceFormatInfo = Get_DXGI_FORMAT_Info(resourceFormat);

					D3D12_BOX srcBox;
					srcBox.left = 0;
					srcBox.right = res.m_size[0];
					srcBox.top = 0;
					srcBox.bottom = res.m_size[1];
					srcBox.front = 0;
					srcBox.back = res.m_size[2];

					D3D12_RESOURCE_DESC resourceDesc = resource->GetDesc();
					std::vector<unsigned char> layoutMem(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64));
					D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutMem.data();

					UINT firstSubResource = D3D12CalcSubresource(res.m_mipIndex, 0, resourceFormatInfo.planeIndex, res.m_numMips, 1);
					interpreter.m_device->GetCopyableFootprints(&resourceDesc, firstSubResource, 1, 0, layout, nullptr, nullptr, nullptr);

					D3D12_TEXTURE_COPY_LOCATION src = {};
					src.pResource = resource;
					src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
					src.SubresourceIndex = firstSubResource;

					D3D12_TEXTURE_COPY_LOCATION dest = {};
					dest.pResource = res.m_resourceReadback;
					dest.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
					dest.PlacedFootprint = *layout;

					interpreter.m_commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, &srcBox);
				}
				else if (res.m_type == RuntimeTypes::ViewableResource::Type::Texture2D ||
					res.m_type == RuntimeTypes::ViewableResource::Type::Texture2DArray ||
					res.m_type == RuntimeTypes::ViewableResource::Type::TextureCube)
				{
					DXGI_FORMAT_Info resourceFormatInfo = Get_DXGI_FORMAT_Info(resourceFormat);

					D3D12_RESOURCE_DESC resourceDesc = resource->GetDesc();
					resourceDesc.Format = resourceFormat;

					std::vector<unsigned char> layoutMem(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64));
					D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layout = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)layoutMem.data();

					UINT firstSubResource = D3D12CalcSubresource(res.m_mipIndex, res.m_arrayIndex, resourceFormatInfo.planeIndex, res.m_numMips, res.m_size[2]);
					interpreter.m_device->GetCopyableFootprints(&resourceDesc, firstSubResource, 1, 0, layout, nullptr, nullptr, nullptr);

					D3D12_TEXTURE_COPY_LOCATION src = {};
					src.pResource = resource;
					src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
					src.SubresourceIndex = D3D12CalcSubresource(res.m_mipIndex, res.m_arrayIndex, resourceFormatInfo.planeIndex, res.m_numMips, res.m_size[2]);


					D3D12_TEXTURE_COPY_LOCATION dest = {};
					dest.pResource = res.m_resourceReadback;
					dest.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
					dest.PlacedFootprint = *layout;

					interpreter.m_commandList->CopyTextureRegion(&dest, 0, 0, 0, &src, nullptr);
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
			res.m_type == RuntimeTypes::ViewableResource::Type::TextureCube)
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
			res.m_type == RuntimeTypes::ViewableResource::Type::TextureCube)
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