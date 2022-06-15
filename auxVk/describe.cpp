#include "describe.h"

namespace aux
{
void Describe::image(VkWriteDescriptorSet& ds, 
	VkDescriptorSet &dstSet,
	uint32_t dstBinding,
	const VkDescriptorImageInfo* pImageInfo)
{
	ds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	ds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	ds.descriptorCount = 1;
	ds.dstSet = dstSet;
	ds.dstBinding = dstBinding;
	ds.pImageInfo = pImageInfo;
}

void Describe::buffer(VkWriteDescriptorSet& ds,
	VkDescriptorSet& dstSet,
	uint32_t dstBinding,
	const VkDescriptorBufferInfo* pBufferInfo)
{
	ds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	ds.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ds.descriptorCount = 1;
	ds.dstSet = dstSet;
	ds.dstBinding = dstBinding;
	ds.pBufferInfo = pBufferInfo;
}


void Describe::any(VkWriteDescriptorSet& ds,
	VkDescriptorType type,
	VkDescriptorSet& dstSet,
	uint32_t dstBinding,
	const VkDescriptorImageInfo* pImageInfo)
{
	ds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	ds.descriptorType = type;
	ds.descriptorCount = 1;
	ds.dstSet = dstSet;
	ds.dstBinding = dstBinding;
	ds.pImageInfo = pImageInfo;
}

//VkWriteDescriptorSet Describe::image(VkDescriptorSet& dstSet,
//	uint32_t dstBinding,
//	const VkDescriptorImageInfo* pImageInfo)
//{
//	VkWriteDescriptorSet ds;
//	image(ds, dstSet, dstBinding, pImageInfo);
//	return ds;
//}

void Describe::bufferUpdate(VkDescriptorSet& dstSet,
	uint32_t dstBinding,
	const VkDescriptorBufferInfo* pBufferInfo) {
	VkWriteDescriptorSet ds{};
	ds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	ds.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ds.descriptorCount = 1;
	ds.dstSet = dstSet;
	ds.dstBinding = dstBinding;
	ds.pBufferInfo = pBufferInfo;
	vkUpdateDescriptorSets(Device::getR(), 1, &ds, 0, nullptr);
}
}
