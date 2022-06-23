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
	inline static VkComputePipelineCreateInfo ci(
		VkPipelineLayout layout,
		VkPipelineCreateFlags flags = 0)
	{
		VkComputePipelineCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		ci.layout = layout;
		ci.flags = flags;
		return ci;
	}

    void bind(VkCommandBuffer& cmdBuf);
};

}


