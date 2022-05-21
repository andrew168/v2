#include "descriptorSetLayout.h"

namespace aux
{
DescriptorSetLayout::DescriptorSetLayout(
	std::vector<VkDescriptorSetLayoutBinding> &dslBindings)
{
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
	descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCI.pBindings = dslBindings.data();
	descriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(dslBindings.size());
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(*(Device::get()), &descriptorSetLayoutCI, nullptr, &m_dsLayout));
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(*(Device::get()), m_dsLayout, nullptr);
}
}
