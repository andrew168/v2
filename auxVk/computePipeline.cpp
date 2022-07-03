#include "pipelineLayout.h"
#include "pipeline.h"
#include "computePipeline.h"
#include "descriptorSetLayout.h"
#include "shaderStage.h"
#include "specialization.h"

namespace aux
{

ComputePipeline::ComputePipeline(VkDescriptorSetLayout& dsl,
	aux::ShaderDescription& shaderDesc)
{
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = PipelineLayout::ci(&dsl, 1);
	VK_CHECK_RESULT(vkCreatePipelineLayout(Device::getR(), &pPipelineLayoutCreateInfo, nullptr, &m_layout));

	init(shaderDesc);
}

ComputePipeline::ComputePipeline(VkPipelineLayout& layout,
	aux::ShaderDescription& shaderDesc,
	std::vector<VkSpecializationMapEntry>* pSpecializationMapEntries,
	uint32_t dataSize,
	void* pData)
{
	m_layout = layout;
	init(shaderDesc, pSpecializationMapEntries, dataSize, pData);
}

void ComputePipeline::init(aux::ShaderDescription& shaderDesc,
	std::vector<VkSpecializationMapEntry>* pSpecializationMapEntries,
	uint32_t dataSize,
	void* pData)
{
	VkComputePipelineCreateInfo computePipelineCreateInfo = ComputePipeline::ci(m_layout, 0);
	computePipelineCreateInfo.stage = ShaderStage::loadShader(shaderDesc);
	if (pSpecializationMapEntries != nullptr) {
		VkSpecializationInfo specializationInfo = Specialization::info(
			static_cast<uint32_t>(pSpecializationMapEntries->size()), 
			pSpecializationMapEntries->data(), dataSize, pData);
		computePipelineCreateInfo.stage.pSpecializationInfo = &specializationInfo;
	}
		
	VK_CHECK_RESULT(vkCreateComputePipelines(Device::getR(), *m_pPipelineCache, 1, &computePipelineCreateInfo, nullptr, &m_pipeline));
}

void ComputePipeline::bind(VkCommandBuffer& cmdBuf)
{
	vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, get());
}

ComputePipeline::~ComputePipeline()
{
	// !! 不能在此删除m_layout:
	// 1) 如果多个Pipeline公用一个PL，会出现重复删除
	// 2）不是自己created，不能删除 （谁create，谁destroy： 责任唯一化）
	// vkDestroyPipelineLayout(Device::getR(), m_layout, nullptr);
}

}
