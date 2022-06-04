#include "..\vk.h"
#include "..\auxVk\auxVk.h"
#include "..\gltf\gltf.h"
#include "pbr.h"

using namespace aux;

namespace v2
{
Pbr::Pbr()
{
}

Pbr::~Pbr()
{
	delete pAuxPipelineBlend;
	delete pAuxPipelinePbr;
	delete pAuxPipelineSkybox;
	delete pAuxPipelineLayout;

	for (auto buffer : paramUniformBuffers) {
		buffer.destroy();
	}
}

void Pbr::init(uint32_t swapChainCount, Camera &camera, VkRenderPass& renderPass)
{
	m_rRenderPass = &renderPass;
	m_pCamera = &camera;
	m_swapChainImageCount = swapChainCount;
	paramUniformBuffers.resize(swapChainCount);
	m_pSceneModel->init(swapChainCount, paramUniformBuffers, *m_pTextures);
	m_pSkyboxModel->init(swapChainCount, paramUniformBuffers, *m_pTextures);
}
void Pbr::config(gltf::Model& sceneModel,
	gltf::Skybox& skyboxModel,
	Textures &textures)
{
	m_pSceneModel = &sceneModel;
	m_pSkyboxModel = &skyboxModel;	
	m_pTextures = &textures;
}

void Pbr::preparePipeline(PbrConfig &settings)
{
	aux::PipelineCI auxPipelineCI{};
	auxPipelineCI.cullMode = VK_CULL_MODE_BACK_BIT;
	if (settings.multiSampling) {
		auxPipelineCI.rasterizationSamples = settings.sampleCount;
	}
	else {
		Assert(0, "which value?");
	}

	// Pipeline layout
	const std::vector<VkDescriptorSetLayout> setLayouts = {
		*(m_pSceneModel->getDSL()),
		*(m_pSceneModel->getMaterialDSL()),
		*(m_pSceneModel->getMeshDSL())
	};

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.size = sizeof(gltf::PushConstBlockMaterial);
	pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	aux::PipelineLayoutCI auxPipelineLayoutCI{ &pushConstantRange };
	auxPipelineLayoutCI.pSetLayouts = &setLayouts;

	pAuxPipelineLayout = new aux::PipelineLayout(auxPipelineLayoutCI);
	// pipelineLayout = pAuxPipelineLayout->get();

	// Vertex bindings an attributes
	std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
		{0, sizeof(vkglTF::Model::Vertex), VK_VERTEX_INPUT_RATE_VERTEX}
	};

	std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
		{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3 },
		{ 2, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 6 },
		{ 3, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 8 },
		{ 4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 10 },
		{ 5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 14 }
	};

	auxPipelineCI.pVertexInputBindings = &vertexInputBindings;
	auxPipelineCI.pVertexInputAttributes = &vertexInputAttributes;

	std::vector<aux::ShaderDescription> shadersSkybox = {
		{"skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
		{"skybox.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT}
	};
	auxPipelineCI.shaders = shadersSkybox;
	pAuxPipelineSkybox = new aux::Pipeline(*pAuxPipelineLayout, *m_rRenderPass, auxPipelineCI);

	// PBR pipeline
	std::vector<aux::ShaderDescription> shadersPbr = {
		{"pbr.vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
		{"pbr_khr.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT}
	};

	auxPipelineCI.shaders = shadersPbr;
	auxPipelineCI.depthWriteEnable = VK_TRUE;
	auxPipelineCI.depthTestEnable = VK_TRUE;
	pAuxPipelinePbr = new aux::Pipeline(*pAuxPipelineLayout, *m_rRenderPass, auxPipelineCI);

	auxPipelineCI.cullMode = VK_CULL_MODE_NONE;
	auxPipelineCI.blendEnable = VK_TRUE;
	auxPipelineCI.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	auxPipelineCI.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	auxPipelineCI.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	auxPipelineCI.colorBlendOp = VK_BLEND_OP_ADD;
	auxPipelineCI.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	auxPipelineCI.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	auxPipelineCI.alphaBlendOp = VK_BLEND_OP_ADD;
	pAuxPipelineBlend = new aux::Pipeline(*pAuxPipelineLayout, *m_rRenderPass, auxPipelineCI);
}

/*
Prepare and initialize uniform buffers containing shader parameters
*/
void Pbr::createUB()
{
	m_pSceneModel->createUB();
	m_pSkyboxModel->createUB();
	for (auto& uniformBuffer : paramUniformBuffers) {
		uniformBuffer.create(Device::getVksDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(shaderValuesParams));
	}
	updateShaderValues();
}

void Pbr::updateShaderValues()
{
	m_pSceneModel->updateShaderValues(*m_pCamera);
	m_pSceneModel->centerAndScale();
	m_pSkyboxModel->updateShaderValues(*m_pCamera);
}

void Pbr::applyShaderValues(uint32_t currentBuffer)
{
	memcpy(paramUniformBuffers[currentBuffer].mapped, &shaderValuesParams, sizeof(ShaderValuesParams));
}
}
