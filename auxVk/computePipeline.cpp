#include "pipeline.h"
#include "computePipeline.h"
#include "descriptorSetLayout.h"
#include "shaderStage.h"

namespace aux
{

ComputePipeline::ComputePipeline(VkDescriptorSetLayout& dsl, 
	aux::ShaderDescription& shaderDesc)
{
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&dsl, 1);
	VK_CHECK_RESULT(vkCreatePipelineLayout(Device::getR(), &pPipelineLayoutCreateInfo, nullptr, &m_layout));

	VkComputePipelineCreateInfo computePipelineCreateInfo = vks::initializers::computePipelineCreateInfo(m_layout, 0);
	computePipelineCreateInfo.stage = ShaderStage::loadShader(shaderDesc);
		
	VK_CHECK_RESULT(vkCreateComputePipelines(Device::getR(), *m_pPipelineCache, 1, &computePipelineCreateInfo, nullptr, &m_pipeline));
}

}
