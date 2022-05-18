#include "pipeline.h"

namespace aux
{

VkPipelineCache* Pipeline::m_pPipelineCache;

Pipeline::Pipeline(aux::PipelineLayout& pipelineLayout, aux::RenderPass& renderPass, PipelineCI& auxci) :
	m_pipelineLayout(pipelineLayout),
	m_renderPass(renderPass),
	m_auxPipelineCI (auxci)
{
	VkDevice* pDevice = aux::Device::get();

	// Pipeline
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{};
	inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateCI.topology = m_auxPipelineCI.primitiveTopology;

	VkPipelineRasterizationStateCreateInfo rasterizationStateCI{};
	rasterizationStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationStateCI.cullMode = VK_CULL_MODE_NONE;
	rasterizationStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationStateCI.lineWidth = 1.0f;

	VkPipelineColorBlendAttachmentState blendAttachmentState{};
	blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendAttachmentState.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendStateCI{};
	colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateCI.attachmentCount = 1;
	colorBlendStateCI.pAttachments = &blendAttachmentState;

	VkPipelineDepthStencilStateCreateInfo depthStencilStateCI{};
	depthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilStateCI.depthTestEnable = VK_FALSE;
	depthStencilStateCI.depthWriteEnable = VK_FALSE;
	depthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilStateCI.front = depthStencilStateCI.back;
	depthStencilStateCI.back.compareOp = VK_COMPARE_OP_ALWAYS;

	VkPipelineViewportStateCreateInfo viewportStateCI{};
	viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCI.viewportCount = 1;
	viewportStateCI.scissorCount = 1;

	VkPipelineMultisampleStateCreateInfo multisampleStateCI{};
	multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicStateCI{};
	dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCI.pDynamicStates = dynamicStateEnables.data();
	dynamicStateCI.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

	VkPipelineVertexInputStateCreateInfo viStateCI{};
	viStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	if (auxci.pVertexInputBinding != nullptr) {
		viStateCI.vertexBindingDescriptionCount = 1;
		viStateCI.pVertexBindingDescriptions = auxci.pVertexInputBinding;
	}

	if (auxci.pVertexInputAttribute != nullptr) {
		viStateCI.vertexAttributeDescriptionCount = 1;
		viStateCI.pVertexAttributeDescriptions = auxci.pVertexInputAttribute;
	}

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	for (auto item : m_auxPipelineCI.shaders) {
		shaderStages.push_back(loadShader(*pDevice, item.m_fileName, item.m_stage));
	}

	VkGraphicsPipelineCreateInfo pipelineCI{};
	pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCI.layout = m_pipelineLayout.get();
	pipelineCI.renderPass = *(m_renderPass.get());
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

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(*pDevice, *(Pipeline::m_pPipelineCache), 1, &pipelineCI, nullptr, &m_pipeline));
	for (auto shaderStage : shaderStages) { // 在建立Pipeline之后，立即destroy 中间文件shader module
		vkDestroyShaderModule(*pDevice, shaderStage.module, nullptr);
	}
}

void Pipeline::bindToGraphic(
	VkCommandBuffer& cmdBuf,
	uint32_t ssWidth,
	uint32_t ssHeight)
{
	bindToGraphic(cmdBuf, ssWidth, ssHeight, ssWidth, ssHeight);
}

void Pipeline::bindToGraphic(
	VkCommandBuffer& cmdBuf, 
	uint32_t ssWidth, 
	uint32_t ssHeight, 
	uint32_t vpWidth, 
	uint32_t vpHeight)
{
	VkViewport viewport{};
	viewport.width = (float)vpWidth;
	viewport.height = (float)vpHeight;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.extent.width = ssWidth;
	scissor.extent.height = ssHeight;

	vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
	vkCmdSetScissor(cmdBuf, 0, 1, &scissor);
	vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, get());
}

Pipeline::~Pipeline()
{
	vkDestroyPipeline(*(aux::Device::get()), m_pipeline, nullptr);
}
}
