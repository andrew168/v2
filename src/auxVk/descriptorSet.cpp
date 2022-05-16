#include "DescriptorSet.h"

namespace aux
{
DescriptorSet::DescriptorSet(DescriptorSetCI &ci)
{
	VkDevice* pDevice = aux::Device::get();

	VkDescriptorPoolSize poolSize = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 };

	VkDescriptorPoolCreateInfo descriptorPoolCI{};
	descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCI.poolSizeCount = 1;
	descriptorPoolCI.pPoolSizes = &poolSize;
	descriptorPoolCI.maxSets = 2;

	VK_CHECK_RESULT(vkCreateDescriptorPool(*pDevice, &descriptorPoolCI, nullptr, &m_descriptorPool));

	// Descriptor sets
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = m_descriptorPool;
	descriptorSetAllocInfo.pSetLayouts = ci.pSetLayouts;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	VK_CHECK_RESULT(vkAllocateDescriptorSets(*pDevice, &descriptorSetAllocInfo, &m_descriptorset));

	VkWriteDescriptorSet writeDescriptorSet{};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.dstSet = m_descriptorset;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.pImageInfo = ci.pImageInfo ;
	vkUpdateDescriptorSets(*pDevice, 1, &writeDescriptorSet, 0, nullptr);
}

DescriptorSet::~DescriptorSet()
{
	vkDestroyDescriptorPool(*(aux::Device::get()), m_descriptorPool, nullptr);
}
}
