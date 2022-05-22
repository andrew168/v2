#include "descriptorSet.h"

namespace aux
{
DescriptorSet::DescriptorSet(DescriptorSetCI &ci)
{
	const VkDevice& device = Device::getR();

	VkDescriptorPoolSize poolSize = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 };

	VkDescriptorPoolCreateInfo descriptorPoolCI{};
	descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCI.poolSizeCount = 1;
	descriptorPoolCI.pPoolSizes = &poolSize;
	descriptorPoolCI.maxSets = 2;

	VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCI, nullptr, &m_descriptorPool));

	DescriptorSet::allocate(m_descriptorset, m_descriptorPool, ci.pSetLayouts);
	
	VkWriteDescriptorSet writeDescriptorSet{};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.dstSet = m_descriptorset;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.pImageInfo = ci.pImageInfo ;
	vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
}

void DescriptorSet::bindToGraphics(VkCommandBuffer &cmdBuf,
		VkPipelineLayout layout,
		uint32_t dynamicOffsetCount,
		const uint32_t* pDynamicOffsets) 
{
	vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, 
		layout, 0, 1, get(), dynamicOffsetCount, pDynamicOffsets);
}

void DescriptorSet::bindToGraphics(const std::vector<VkDescriptorSet> &sets, 
	VkCommandBuffer& cmdBuf,
	VkPipelineLayout &layout,
	uint32_t dynamicOffsetCount,
	const uint32_t* pDynamicOffsets)
{
	vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
		layout, 0, static_cast<uint32_t>(sets.size()), sets.data(), dynamicOffsetCount, pDynamicOffsets);
}

void DescriptorSet::allocate(VkDescriptorSet& dSet, 
	const VkDescriptorPool& pool, 
	const VkDescriptorSetLayout* pLayout) 
{
	VkDescriptorSetAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	ai.descriptorPool = pool;
	ai.pSetLayouts = pLayout;
	ai.descriptorSetCount = 1;
	VK_CHECK_RESULT(vkAllocateDescriptorSets(Device::getR(), &ai, &dSet));
}

DescriptorSet::~DescriptorSet()
{
	vkDestroyDescriptorPool(Device::getR(), m_descriptorPool, nullptr);
}
}
