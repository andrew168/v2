#pragma once
#include "..\vk-all.h"
#include "device.h"
#include "descriptor.h"

namespace aux
{
struct DescriptorSetCI {
    const VkDescriptorImageInfo* pImageInfo;
    const VkDescriptorSetLayout* pSetLayouts;
};

class DescriptorSet
{
    VkDescriptorPool* m_pDescriptorPool;
    VkDescriptorSet* m_pDescriptorSet;
    VkDescriptorSetLayout* m_rDSL;

private:
    bool m_isVK = false;

public:
    explicit DescriptorSet(VkDescriptorSet &ds);
    explicit DescriptorSet(aux::DescriptorSetCI &ci);
    explicit DescriptorSet(VkDescriptorPool& pool, VkDescriptorSetLayout& dsl);

    ~DescriptorSet();

    void write(std::vector<Descriptor> descs);

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

    static void updateW(std::vector<VkWriteDescriptorSet> sets);
    static void updateC(std::vector<VkCopyDescriptorSet> sets);

    VkDescriptorSet* get() { return m_pDescriptorSet; }
};

}
