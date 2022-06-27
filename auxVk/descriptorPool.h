#pragma once
#include "..\vk-all.h"
#include "device.h"

namespace aux
{
class DescriptorPool
{
public:
    static void create(VkDescriptorPool& descriptorPool,
		std::vector<VkDescriptorPoolSize> poolSizes,
		uint32_t maxSets);

	inline static VkDescriptorPoolSize uniformBufferSize(uint32_t descriptorCount)
	{
		VkDescriptorPoolSize dpSize{};
		dpSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		dpSize.descriptorCount = descriptorCount;
		return dpSize;
	}

	inline static VkDescriptorPoolSize combinedImageSamplerSize(uint32_t descriptorCount)
	{
		VkDescriptorPoolSize dpSize{};
		dpSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		dpSize.descriptorCount = descriptorCount;
		return dpSize;
	}

	inline static VkDescriptorPoolSize storageImageSize(uint32_t descriptorCount)
	{
		VkDescriptorPoolSize dpSize{};
		dpSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		dpSize.descriptorCount = descriptorCount;
		return dpSize;
	}

	inline static VkDescriptorPoolSize storageBufferSize(uint32_t descriptorCount)
	{
		VkDescriptorPoolSize dpSize{};
		dpSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		dpSize.descriptorCount = descriptorCount;
		return dpSize;
	}
};
}
