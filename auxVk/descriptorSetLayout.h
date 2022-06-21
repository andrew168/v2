#pragma once
#include "..\vk-all.h"
#include "device.h"

namespace aux
{
class DescriptorSetLayout
{
    VkDescriptorSetLayout m_dsLayout;
public:
    explicit DescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding>& dslBindings);
    ~DescriptorSetLayout();

	static inline VkDescriptorSetLayoutBinding bindingDesc(
		VkDescriptorType type,
		VkShaderStageFlags stageFlags,
		uint32_t binding,
		uint32_t descriptorCount = 1)
	{
		VkDescriptorSetLayoutBinding dslBinding{};
		dslBinding.descriptorType = type;
		dslBinding.stageFlags = stageFlags;
		dslBinding.binding = binding;
		dslBinding.descriptorCount = descriptorCount;
		return dslBinding;
	}

    VkDescriptorSetLayout* get() { return &m_dsLayout; }
    VkDescriptorSetLayout& getR() { return m_dsLayout; }
};
}
