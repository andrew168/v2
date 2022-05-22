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
    VkDescriptorPool* m_pDescriptorPool;
    VkDescriptorSet* m_pDescriptorset;
private:
    bool m_isVK = false;

public:
    explicit DescriptorSet(VkDescriptorSet &ds);
    explicit DescriptorSet(aux::DescriptorSetCI &ci);
    ~DescriptorSet();
    void bindToGraphics(VkCommandBuffer& cmdBuf,
        VkPipelineLayout layout,
        uint32_t dynamicOffsetCount = 0,
        const uint32_t* pDynamicOffsets = NULL);
    static void DescriptorSet::bindToGraphics(const std::vector<VkDescriptorSet> &sets,
        VkCommandBuffer& cmdBuf,
        VkPipelineLayout &layout,
        uint32_t dynamicOffsetCount = 0,
        const uint32_t* pDynamicOffsets = NULL);
    static void allocate(VkDescriptorSet& dSet,
        const VkDescriptorPool& pool, 
        const VkDescriptorSetLayout* pLayout);
    VkDescriptorSet* get() { return m_pDescriptorset; }
};

}
