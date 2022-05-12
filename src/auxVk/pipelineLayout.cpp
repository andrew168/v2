#include "pipelineLayout.h"

namespace aux
{
class Image;

PipelineLayout::PipelineLayout(aux::Image* image)
{
	VkDevice *pDevice = aux::Device::get();

	// Desriptors
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
	descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(*pDevice, &descriptorSetLayoutCI, nullptr, &m_descriptorSetLayout));

	VkPipelineLayoutCreateInfo pipelineLayoutCI{};
	pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCI.setLayoutCount = 1;
	pipelineLayoutCI.pSetLayouts = &m_descriptorSetLayout;
	VK_CHECK_RESULT(vkCreatePipelineLayout(*pDevice, &pipelineLayoutCI, nullptr, &m_pipelineLayout));
}
PipelineLayout::~PipelineLayout()
{
	vkDestroyPipelineLayout(*(aux::Device::get()), m_pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(*(aux::Device::get()),m_descriptorSetLayout, nullptr);
}
}
