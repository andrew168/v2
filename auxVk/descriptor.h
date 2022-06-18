#pragma once
#include "..\vk-all.h"
#include "device.h"

namespace aux
{
struct Descriptor
{
	uint32_t                     dstBinding;
	uint32_t                     descriptorCount;
	VkDescriptorType             descriptorType;
	const void* pImageInfo;

inline	Descriptor(VkDescriptorType type,
		uint32_t binding,
		void* imageInfo,
		uint32_t count = 1)
	{
		descriptorType = type;
		dstBinding = binding;
		pImageInfo = imageInfo;
		descriptorCount = count;
	}
};

struct StorageImageDescriptor: public Descriptor
{
inline StorageImageDescriptor(uint32_t binding,
		VkDescriptorImageInfo* imageInfo,
		uint32_t descriptorCount):
		Descriptor(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, binding, imageInfo, descriptorCount)
	{
	}
};

struct BufferDescriptor : public Descriptor
{
	inline BufferDescriptor(uint32_t binding,
		VkDescriptorBufferInfo* pBufferInfo,
		uint32_t descriptorCount) :
		Descriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, binding, pBufferInfo, descriptorCount)
	{
	}
};

}
