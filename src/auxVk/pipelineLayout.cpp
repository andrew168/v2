#include "pipelineLayout.h"

namespace aux
{
class Image;

PipelineLayout::PipelineLayout(PipelineLayoutCI &ci):
	m_pDescriptorSet(nullptr),
	m_descriptorSetLayout(nullptr)
{
	VkDevice *pDevice = aux::Device::get();

	// Desriptors

	VkPipelineLayoutCreateInfo pipelineLayoutCI{};
	pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	if (ci.pSetLayouts == nullptr) {
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
		descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		if (ci.pDslBindings != nullptr) {
			descriptorSetLayoutCI.pBindings = ci.pDslBindings;
			descriptorSetLayoutCI.bindingCount = 1;
		}
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(*pDevice, &descriptorSetLayoutCI, nullptr, &m_descriptorSetLayout));
		pipelineLayoutCI.setLayoutCount = 1;
		pipelineLayoutCI.pSetLayouts = &m_descriptorSetLayout;
	}
	else {
		pipelineLayoutCI.setLayoutCount = static_cast<uint32_t> (ci.pSetLayouts->size());
		pipelineLayoutCI.pSetLayouts = ci.pSetLayouts->data();
	}
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

	if (m_descriptorSetLayout != nullptr) {
		vkDestroyDescriptorSetLayout(*(aux::Device::get()), m_descriptorSetLayout, nullptr);
		m_descriptorSetLayout = nullptr;
	}

	if (m_pDescriptorSet != nullptr) {
		delete m_pDescriptorSet;
		m_pDescriptorSet = nullptr;
	}
}
}
