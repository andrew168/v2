#pragma once
#include "..\vk.h"
#include "device.h"

namespace aux
{
class DescriptorSetLayout
{
    VkDescriptorSetLayout m_dsLayout;
public:
    explicit DescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding>& dslBindings);
    ~DescriptorSetLayout();

    VkDescriptorSetLayout* get() { return &m_dsLayout; }
};
}
