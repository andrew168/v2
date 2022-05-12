#pragma once
#include "..\vk.h"
#include "device.h"

namespace aux
{
class Image;

class PipelineLayout
{
    VkPipelineLayout m_pipelineLayout;
    VkDescriptorSetLayout m_descriptorSetLayout;

public:
    explicit  PipelineLayout(aux::Image* image);
    ~PipelineLayout();
    VkPipelineLayout get() { return m_pipelineLayout; }
private:
};

}
