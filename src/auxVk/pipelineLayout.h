#pragma once
#include "..\vk.h"
#include "device.h"

namespace aux
{
class PipelineLayout
{
    VkPipelineLayout m_pipelineLayout;
    VkDescriptorSetLayout m_descriptorSetLayout;

public:
    explicit PipelineLayout();
    ~PipelineLayout();
    VkPipelineLayout get() { return m_pipelineLayout; }
private:
};

}
