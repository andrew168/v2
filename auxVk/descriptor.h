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
	const VkDescriptorImageInfo* pImageInfo;

inline	Descriptor(VkDescriptorType type,
		uint32_t binding,
		VkDescriptorImageInfo* imageInfo,
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

}
