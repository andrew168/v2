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
};
}
