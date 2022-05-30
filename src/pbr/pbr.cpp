#include "..\vk.h"
#include "..\auxVk\auxVk.h"
#include "..\gltf\gltf.h"
#include "pbr.h"

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
	std::vector<VkDescriptorSet> sceneDS;
}

void Pbr::init(uint32_t swapChainCount)
{
	sceneDS.resize(swapChainCount);
}
void Pbr::config(gltf::Model& sceneModel,
	gltf::Model& skyboxModel,
	std::vector<Buffer>& paramUniformBuffers,
	Textures &textures)
{
	m_pSceneModel = &sceneModel;
	m_pSkyboxModel = &skyboxModel;
	m_pParamUniformBuffers = &paramUniformBuffers;
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
		aux::Describe::buffer(writeDescriptorSets[1], sceneDS[i], 1, &(*m_pParamUniformBuffers)[i].descriptor);
		aux::Describe::image(writeDescriptorSets[2], sceneDS[i], 2, &m_pTextures->irradianceCube.descriptor);
		aux::Describe::image(writeDescriptorSets[3], sceneDS[i], 3, &m_pTextures->prefilteredCube.descriptor);
		aux::Describe::image(writeDescriptorSets[4], sceneDS[i], 4, &m_pTextures->lutBrdf.descriptor);

		aux::DescriptorSet::updateW(writeDescriptorSets);
	}
}

}
