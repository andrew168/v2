#include "descriptorPool.h"

namespace aux
{
void DescriptorPool::create(VkDescriptorPool &descriptorPool, 
	std::vector<VkDescriptorPoolSize> poolSizes,
	uint32_t maxSets)
{
	VkDescriptorPoolCreateInfo descriptorPoolCI{};
	descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCI.poolSizeCount = 2;
	descriptorPoolCI.pPoolSizes = poolSizes.data();
	descriptorPoolCI.maxSets = maxSets;
	VK_CHECK_RESULT(vkCreateDescriptorPool(*(Device::get()), &descriptorPoolCI, nullptr, &descriptorPool));
}
}
