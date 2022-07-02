#include "descriptorSetLayout.h"

namespace aux
{
DescriptorSetLayout::DescriptorSetLayout(
	std::vector<VkDescriptorSetLayoutBinding> &dslBindings)
{
	VkDescriptorSetLayoutCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	ci.pBindings = dslBindings.data();
	ci.bindingCount = static_cast<uint32_t>(dslBindings.size());
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(Device::getR(), &ci, nullptr, &m_dsLayout));
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(Device::getR(), m_dsLayout, nullptr);
}
}
