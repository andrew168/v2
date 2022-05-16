#include "pipelineLayout.h"

namespace aux
{
class Image;

PipelineLayout::PipelineLayout(PipelineLayoutCI &ci):
	m_pDescriptorSet(nullptr)
{
	VkDevice *pDevice = aux::Device::get();

	// Desriptors
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
	descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	if (ci.pDslBindings != nullptr) {
		descriptorSetLayoutCI.pBindings = ci.pDslBindings;
		descriptorSetLayoutCI.bindingCount = 1;
	}
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(*pDevice, &descriptorSetLayoutCI, nullptr, &m_descriptorSetLayout));

	VkPipelineLayoutCreateInfo pipelineLayoutCI{};
	pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCI.setLayoutCount = 1;
	pipelineLayoutCI.pSetLayouts = &m_descriptorSetLayout;
	if (ci.pPcRange != nullptr) {
		pipelineLayoutCI.pushConstantRangeCount = 1;  // ToDo:: Support N>1 cases
		pipelineLayoutCI.pPushConstantRanges = ci.pPcRange;
	}

	VK_CHECK_RESULT(vkCreatePipelineLayout(*pDevice, &pipelineLayoutCI, nullptr, &m_pipelineLayout));

	if (ci.pImageInfo != nullptr) {
		aux::DescriptorSetCI dsci{};
		dsci.pImageInfo = ci.pImageInfo;
		dsci.pSetLayouts = &m_descriptorSetLayout;
		m_pDescriptorSet = new aux::DescriptorSet(dsci);
	}

}

PipelineLayout::~PipelineLayout()
{
	vkDestroyPipelineLayout(*(aux::Device::get()), m_pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(*(aux::Device::get()), m_descriptorSetLayout, nullptr);
	delete m_pDescriptorSet;
	m_pDescriptorSet = nullptr;
}
}
