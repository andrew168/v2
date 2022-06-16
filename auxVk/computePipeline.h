#pragma once
#include "..\vk-all.h"
#include "device.h"

namespace aux
{

class DescriptorSetLayout;
struct ShaderDescription;

class ComputePipeline : public PipelineBase
{

public:
    explicit ComputePipeline(VkDescriptorSetLayout& dsl,
        aux::ShaderDescription& shaderDesc);
    ~ComputePipeline();
    void bind(VkCommandBuffer& cmdBuf);
};

}


