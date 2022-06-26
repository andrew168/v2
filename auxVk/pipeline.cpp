#include "pipeline.h"
#include "shaderStage.h"

namespace aux
{
VkPipelineCache* PipelineBase::m_pPipelineCache;

Pipeline::Pipeline(VkPipelineLayout& pipelineLayout, VkRenderPass& renderPass, PipelineCI& auxci) :
	m_pipelineLayout(pipelineLayout),
	m_renderPass(renderPass),
	m_auxCI (auxci)
{
	const VkDevice& device = Device::getR();

	// Pipeline
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{};
	inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateCI.topology = m_auxCI.primitiveTopology;
	inputAssemblyStateCI.flags = 0;
	inputAssemblyStateCI.primitiveRestartEnable = VK_FALSE;

	VkPipelineRasterizationStateCreateInfo rasterizationStateCI{};
	rasterizationStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationStateCI.cullMode = m_auxCI.cullMode;		// VK_CULL_MODE_NONE
	rasterizationStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationStateCI.flags = 0;
	rasterizationStateCI.depthClampEnable = VK_FALSE;
	rasterizationStateCI.lineWidth = 1.0f;

	VkPipelineColorBlendAttachmentState blendAttachmentState{};
	blendAttachmentState.colorWriteMask = m_auxCI.colorWriteMask; // 0xf;
	// 0xf == VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendAttachmentState.blendEnable = m_auxCI.blendEnable;
	if (blendAttachmentState.blendEnable) {  // VK_FALSE
		blendAttachmentState.srcColorBlendFactor = m_auxCI.srcColorBlendFactor; // VK_BLEND_FACTOR_SRC_ALPHA
		blendAttachmentState.dstColorBlendFactor = m_auxCI.dstColorBlendFactor; // VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
		blendAttachmentState.colorBlendOp = m_auxCI.colorBlendOp;				// VK_BLEND_OP_ADD
		blendAttachmentState.srcAlphaBlendFactor = m_auxCI.srcAlphaBlendFactor; // VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
		blendAttachmentState.dstAlphaBlendFactor = m_auxCI.dstAlphaBlendFactor; // VK_BLEND_FACTOR_ZERO
		blendAttachmentState.alphaBlendOp = m_auxCI.alphaBlendOp;				// VK_BLEND_OP_ADD
	}

	VkPipelineColorBlendStateCreateInfo colorBlendStateCI{};
	colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateCI.attachmentCount = 1;
	colorBlendStateCI.pAttachments = &blendAttachmentState;

	VkPipelineDepthStencilStateCreateInfo depthStencilStateCI{};
	depthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilStateCI.depthTestEnable = m_auxCI.depthTestEnable;  // VK_FALSE
	depthStencilStateCI.depthWriteEnable = m_auxCI.depthWriteEnable; //VK_FALSE
	depthStencilStateCI.depthCompareOp = m_auxCI.depthCompareOp; // VK_COMPARE_OP_LESS_OR_EQUAL
	depthStencilStateCI.front = depthStencilStateCI.back;
	depthStencilStateCI.back.compareOp = VK_COMPARE_OP_ALWAYS;

	VkPipelineViewportStateCreateInfo viewportStateCI{};
	viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCI.viewportCount = 1;
	viewportStateCI.scissorCount = 1;
	viewportStateCI.flags = 0;

	VkPipelineMultisampleStateCreateInfo multisampleStateCI{};
	multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleStateCI.rasterizationSamples = m_auxCI.rasterizationSamples; // VK_SAMPLE_COUNT_1_BIT

	std::vector<VkDynamicState> dynamicStateEnables = {
		VK_DYNAMIC_STATE_VIEWPORT, 
		VK_DYNAMIC_STATE_SCISSOR 
	};
	VkPipelineDynamicStateCreateInfo dynamicStateCI{};
	dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCI.pDynamicStates = dynamicStateEnables.data();
	dynamicStateCI.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

	VkPipelineVertexInputStateCreateInfo viStateCI{};
	viStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	if (auxci.pVertexInputBindings != nullptr) {
		viStateCI.vertexBindingDescriptionCount = static_cast<uint32_t>(auxci.pVertexInputBindings->size());
		viStateCI.pVertexBindingDescriptions = auxci.pVertexInputBindings->data();
	}
	if (auxci.pVertexInputAttributes != nullptr) {
		viStateCI.vertexAttributeDescriptionCount = static_cast <uint32_t>(auxci.pVertexInputAttributes->size());
		viStateCI.pVertexAttributeDescriptions = auxci.pVertexInputAttributes->data();
	}

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	for (auto item : m_auxCI.shaders) {
		shaderStages.push_back(ShaderStage::loadShader(device, item.m_fileName, item.m_stage));
	}

	VkGraphicsPipelineCreateInfo pipelineCI{};
	pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCI.layout = m_pipelineLayout;
	pipelineCI.renderPass = m_renderPass;
	pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
	pipelineCI.pVertexInputState = &viStateCI;
	pipelineCI.pRasterizationState = &rasterizationStateCI;
	pipelineCI.pColorBlendState = &colorBlendStateCI;
	pipelineCI.pMultisampleState = &multisampleStateCI;
	pipelineCI.pViewportState = &viewportStateCI;
	pipelineCI.pDepthStencilState = &depthStencilStateCI;
	pipelineCI.pDynamicState = &dynamicStateCI;	
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, *(Pipeline::m_pPipelineCache), 1, &pipelineCI, nullptr, &m_pipeline));
	for (auto shaderStage : shaderStages) { // 在建立Pipeline之后，立即destroy 中间文件shader module
		vkDestroyShaderModule(device, shaderStage.module, nullptr);
	}
}

void Pipeline::bindToGraphic(VkCommandBuffer& cmdBuf)
{
	vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, get());
}

Pipeline::~Pipeline()
{
	vkDestroyPipeline(Device::getR(), m_pipeline, nullptr);
}

bool PipelineCI::validate() 
{
	if (pVertexInputBindings != nullptr) {
		for (uint32_t i = 0; i < pVertexInputBindings->size(); i++) {
			assert((*pVertexInputBindings)[i].binding == i);
		}
	}

	if (pVertexInputAttributes != nullptr) {
		for (uint32_t i = 0; i < pVertexInputAttributes->size(); i++) {
			VkVertexInputAttributeDescription* pAttrib = &(*pVertexInputAttributes)[i];
			assert(pAttrib->location == i);
			assert(pAttrib->location > pAttrib->binding);
		}
	}
	return true;
}

}
