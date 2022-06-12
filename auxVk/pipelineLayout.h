#pragma once
#include "..\vk-all.h"
#include "device.h"
#include "descriptorSet.h"

namespace aux
{
class Image;
struct DescriptorSetCI;
class DescriptorSet;
        
struct PipelineLayoutCI : public VkPipelineLayoutCreateInfo
{
    const VkDescriptorSetLayoutBinding* pDslBindings;
    const VkPushConstantRange* pPcRange;
    const VkDescriptorImageInfo* pImageInfo;
    const std::vector<VkDescriptorSetLayout> *pSetLayouts;
    PipelineLayoutCI(const VkPushConstantRange* _pPcRange = nullptr, 
        VkDescriptorSetLayoutBinding* _pDslBindings = nullptr,
        VkDescriptorImageInfo* _pImageInfo = nullptr) :
        pPcRange(_pPcRange),
        pDslBindings(_pDslBindings),
        pImageInfo(_pImageInfo),
        pSetLayouts(nullptr)
    {
    }

    PipelineLayoutCI(const std::vector<VkDescriptorSetLayout>& dsls) :
        pPcRange(nullptr),
        pDslBindings(nullptr),
        pImageInfo(nullptr),
        pSetLayouts(&dsls)
    {
    }
};
class PipelineLayout
{
    VkPipelineLayout m_pipelineLayout;
    VkDescriptorSetLayout m_descriptorSetLayout;
    DescriptorSet* m_pDescriptorSet;

public:
    explicit  PipelineLayout(PipelineLayoutCI &ci);
    ~PipelineLayout();
    VkPipelineLayout get() { return m_pipelineLayout; }
    VkPipelineLayout* getP() { return &m_pipelineLayout; }
    VkDescriptorSetLayout* getDSLayout() { return &m_descriptorSetLayout; }
    DescriptorSet* getDSet() { return m_pDescriptorSet; }

private:
};

}
