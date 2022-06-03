#include "..\vk.h"
#include "..\auxVk\auxVk.h"
#include "..\gltf\gltf.h"
#include "pbr.h"
#include "skyboxRender.h"

using namespace aux;

namespace v2
{
SkyboxRender::SkyboxRender():
	Pbr()
{
}

SkyboxRender::~SkyboxRender()
{
}

void SkyboxRender::init(uint32_t swapChainCount, Camera& camera, VkRenderPass& renderPass)
{
	Pbr::init(swapChainCount, camera, renderPass);
	skyboxDS.resize(swapChainCount);
}

void SkyboxRender::setupDescriptors(VkDescriptorPool& descriptorPool)
{
	/*
		Descriptor Pool
	*/
	uint32_t imageSamplerCount = 0;
	uint32_t materialCount = 0;
	uint32_t meshCount = 0;

	// Environment samplers (radiance, irradiance, brdf lut)
	imageSamplerCount += 3;

	// descriptorPool是本次绘制中所有模型公用的。
	// 容纳所有Sampler，对应每一个模型的每一种材质的5张texture
	// 创建descriptorPool，容纳所有Sampler，对应每一个模型的每一种材质的5张texture
	std::vector<vkglTF::Model*> modellist = { m_pSkyboxModel, m_pSceneModel };
	for (auto& model : modellist) {
		for (auto& material : model->materials) {
			imageSamplerCount += 5;  // 每种PBR材质有5个texture作为sampler
			materialCount++;
		}
		for (auto node : model->linearNodes) {
			if (node->mesh) {  // 	// 每一个GLTF模型都有自己的mesh列表
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

void SkyboxRender::configPbr(uint32_t dsID,
	VkCommandBuffer& cmdBuf)
{
	m_rDS = &skyboxDS[dsID];
	m_rCmdBuf = &cmdBuf;
	m_rPipelineLayout = getPipelineLayout();
	m_rPipeline = pAuxPipelineSkybox;
}

void SkyboxRender::draw(vkglTF::Model& model)
{
	aux::DescriptorSet dsSkybox(*m_rDS);
	dsSkybox.bindToGraphics(*m_rCmdBuf, *m_rPipelineLayout);
	m_rPipeline->bindToGraphic(*m_rCmdBuf);
	model.draw(*m_rCmdBuf);
}
}
