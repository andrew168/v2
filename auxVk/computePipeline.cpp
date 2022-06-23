#include "pipelineLayout.h"
#include "pipeline.h"
#include "computePipeline.h"
#include "descriptorSetLayout.h"
#include "shaderStage.h"

namespace aux
{

ComputePipeline::ComputePipeline(VkDescriptorSetLayout& dsl, 
	aux::ShaderDescription& shaderDesc)
{
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = PipelineLayout::ci(&dsl, 1);
	VK_CHECK_RESULT(vkCreatePipelineLayout(Device::getR(), &pPipelineLayoutCreateInfo, nullptr, &m_layout));
	VkComputePipelineCreateInfo computePipelineCreateInfo = ComputePipeline::ci(m_layout, 0);
	computePipelineCreateInfo.stage = ShaderStage::loadShader(shaderDesc);
		
	VK_CHECK_RESULT(vkCreateComputePipelines(Device::getR(), *m_pPipelineCache, 1, &computePipelineCreateInfo, nullptr, &m_pipeline));
}

void ComputePipeline::bind(VkCommandBuffer& cmdBuf)
{
	vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, get());
}

ComputePipeline::~ComputePipeline()
{
	vkDestroyPipelineLayout(Device::getR(), m_layout, nullptr);
}

}
