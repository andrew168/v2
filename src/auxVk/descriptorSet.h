#pragma once
#include "..\vk.h"
#include "device.h"

namespace aux
{
struct DescriptorSetCI {
    const VkDescriptorImageInfo* pImageInfo;
    const VkDescriptorSetLayout* pSetLayouts;
};

class DescriptorSet
{
    VkDescriptorPool m_descriptorPool;
    VkDescriptorSet m_descriptorset;

public:
    explicit DescriptorSet(aux::DescriptorSetCI &ci);
    ~DescriptorSet();

    VkDescriptorSet* get() { return &m_descriptorset; }
};

}