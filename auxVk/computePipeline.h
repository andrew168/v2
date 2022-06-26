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

	explicit ComputePipeline(VkPipelineLayout& layout,
		aux::ShaderDescription& shaderDesc,
		std::vector<VkSpecializationMapEntry> *pSpecializationMapEntries = nullptr,
		uint32_t dataSize = 0,
		void* pData = nullptr);

	void init(aux::ShaderDescription& shaderDesc,
		std::vector<VkSpecializationMapEntry>* pSpecializationMapEntries = nullptr,
		uint32_t dataSize = 0,
		void* pData = nullptr);

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


