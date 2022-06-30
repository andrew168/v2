#pragma once
#include "..\vk-all.h"
#include "device.h"
#include "descriptor.h"

namespace aux
{
class PipelineBase;
class Pipeline;

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
    
    void DescriptorSet::bind(VkCommandBuffer& cmdBuf,
        PipelineBase& pipeline,
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

    inline static VkDescriptorSetAllocateInfo ai(
        VkDescriptorPool descriptorPool,
        const VkDescriptorSetLayout* pSetLayouts,
        uint32_t descriptorSetCount)
    {
        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
        descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocateInfo.descriptorPool = descriptorPool;
        descriptorSetAllocateInfo.pSetLayouts = pSetLayouts;
        descriptorSetAllocateInfo.descriptorSetCount = descriptorSetCount;
        return descriptorSetAllocateInfo;
    }

    void updateW(std::vector<Descriptor> descs);
    
    static void updateW(std::vector<VkWriteDescriptorSet> sets);
    static void updateC(std::vector<VkCopyDescriptorSet> sets);
   
    VkDescriptorSet* get() { return m_pDescriptorSet; }
    VkDescriptorSet& getR() { return *m_pDescriptorSet; }
};

}
