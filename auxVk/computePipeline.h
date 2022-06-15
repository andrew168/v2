#pragma once
#include "..\vk-all.h"
#include "device.h"

namespace aux
{

class DescriptorSetLayout;
struct ShaderDescription;

class ComputePipeline : public PipelineBase
{
    VkPipelineLayout m_layout;

public:
    explicit ComputePipeline(VkDescriptorSetLayout& dsl,
        aux::ShaderDescription& shaderDesc);
    VkPipelineLayout* getLayout() { return &m_layout; }
};

}


