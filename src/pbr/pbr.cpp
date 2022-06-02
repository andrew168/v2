#include "..\vk.h"
#include "..\auxVk\auxVk.h"
#include "..\gltf\gltf.h"
#include "pbr.h"
#include "..\gltf\gltf.h"

using namespace aux;

namespace v2
{
Pbr::Pbr():
	m_pDSL(nullptr)
{
}

Pbr::~Pbr()
{
	delete m_pDSL;
	m_pDSL = nullptr;

	delete pAuxDSLayoutNode;
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
	sceneDS.resize(swapChainCount);
	skyboxDS.resize(swapChainCount);
}
void Pbr::config(gltf::Model& sceneModel,
	gltf::Model& skyboxModel,
	Textures &textures)
{
	m_pSceneModel = &sceneModel;
	m_pSkyboxModel = &skyboxModel;	
	m_pTextures = &textures;
}

void Pbr::setupDSL(VkDescriptorPool& descriptorPool)
{
	// Scene (matrices and environment maps)
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
		{ 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
		{ 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
		{ 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
		{ 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
	};

	m_pDSL = new aux::DescriptorSetLayout(setLayoutBindings);

	for (auto i = 0; i < sceneDS.size(); i++) {
		aux::DescriptorSet::allocate(sceneDS[i],
			descriptorPool, m_pDSL->get());

		std::vector<VkWriteDescriptorSet> writeDescriptorSets(5);
		aux::Describe::buffer(writeDescriptorSets[0], sceneDS[i], 0, &(m_pSceneModel->getUB()[i].descriptor));
		aux::Describe::buffer(writeDescriptorSets[1], sceneDS[i], 1, &(paramUniformBuffers)[i].descriptor);
		aux::Describe::image(writeDescriptorSets[2], sceneDS[i], 2, &m_pTextures->irradianceCube.descriptor);
		aux::Describe::image(writeDescriptorSets[3], sceneDS[i], 3, &m_pTextures->prefilteredCube.descriptor);
		aux::Describe::image(writeDescriptorSets[4], sceneDS[i], 4, &m_pTextures->lutBrdf.descriptor);

		aux::DescriptorSet::updateW(writeDescriptorSets);
	}
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
		*(getDSL()),
		*(m_pSceneModel->getMaterialDSL()),
		*(pAuxDSLayoutNode->get())
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

void Pbr::setupDescriptors(VkDescriptorPool &descriptorPool)
{
	/*
		Descriptor Pool
	*/
	uint32_t imageSamplerCount = 0;
	uint32_t materialCount = 0;
	uint32_t meshCount = 0;

	// Environment samplers (radiance, irradiance, brdf lut)
	imageSamplerCount += 3;

	// 每一个GLTF模型都有自己的材质列表和mesh列表
	std::vector<vkglTF::Model*> modellist = { m_pSkyboxModel, m_pSceneModel };
	for (auto& model : modellist) {
		for (auto& material : model->materials) {
			imageSamplerCount += 5;  // 每种PBR材质有5个texture作为sampler
			materialCount++;
		}
		for (auto node : model->linearNodes) {
			if (node->mesh) {
				meshCount++;
			}
		}
	}

	std::vector<VkDescriptorPoolSize> poolSizes = {
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (4 + meshCount) * m_swapChainImageCount },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageSamplerCount * m_swapChainImageCount }
	};
	uint32_t maxSets = (2 + materialCount + meshCount) * m_swapChainImageCount;
	aux::DescriptorPool::create(descriptorPool, poolSizes, maxSets);

	/*
		Descriptor sets
	*/

	setupDSL(descriptorPool);
	m_pSceneModel->setupMaterialDSL(descriptorPool, m_pTextures->empty.descriptor);
	// Model node (matrices)
	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
			{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
		};
		pAuxDSLayoutNode = new aux::DescriptorSetLayout(setLayoutBindings);

		// Per-Node descriptor set
		for (auto& node : m_pSceneModel->nodes) {
			setupNodeDescriptorSet(node, descriptorPool);
		}
	}

	// Skybox (fixed set)
	for (auto i = 0; i < m_pSkyboxModel->getUB().size(); i++) {
		aux::DescriptorSet::allocate(skyboxDS[i],
			descriptorPool, getDSL());

		std::vector<VkWriteDescriptorSet> writeDescriptorSets(3);
		aux::Describe::buffer(writeDescriptorSets[0], skyboxDS[i], 0, &(m_pSkyboxModel->getUB()[i].descriptor));
		aux::Describe::buffer(writeDescriptorSets[1], skyboxDS[i], 1, &paramUniformBuffers[i].descriptor);
		aux::Describe::image(writeDescriptorSets[2], skyboxDS[i], 2, &m_pTextures->prefilteredCube.descriptor);
		aux::DescriptorSet::updateW(writeDescriptorSets);
	}
}
void Pbr::setupNodeDescriptorSet(vkglTF::Node* node, VkDescriptorPool& descriptorPool)
{
	if (node->mesh) {
		aux::DescriptorSet::allocate(node->mesh->uniformBuffer.descriptorSet,
			descriptorPool, pAuxDSLayoutNode->get());
		aux::Describe::bufferUpdate(node->mesh->uniformBuffer.descriptorSet,
			0, &node->mesh->uniformBuffer.descriptor);
	}
	for (auto& child : node->children) {
		setupNodeDescriptorSet(child, descriptorPool);
	}
}

/*
Prepare and initialize uniform buffers containing shader parameters
*/
void Pbr::prepareUniformBuffers()
{
	m_pSceneModel->prepareUniformBuffers();
	m_pSkyboxModel->prepareUniformBuffers();
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
