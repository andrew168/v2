#include "../auxVk/auxVk.h"
#include "../pbr/pbr.h"
#include "model.h"
#include "skybox.h"

namespace gltf
{
using namespace aux;
Skybox::Skybox() :
	Model::Model()
{
}

Skybox::~Skybox()
{
}

//DS: skybox的Uniform Buffer，pbr参数的UB， 1个环境Cubemap: Prefilted
void Skybox::updateDS(VkDescriptorPool& descriptorPool)
{
	if (m_pDSL == nullptr) {
		Model::createDSL(descriptorPool);
	}

	// Skybox (fixed set)
	// 用1个DS一次性update描述SkyboxBody的3个D (1个整体UB + 1个pbr UB + 1个环境Sampler）
	for (auto i = 0; i < ds.size(); i++) {
		// DSL可公用，借用SceneBody的DSL的前3个，只要一致即可。
		// 先从Dpool中allocate 1个DS（唯一的），再update
		aux::DescriptorSet::allocate(ds[i], descriptorPool, m_pDSL->get());

		std::vector<VkWriteDescriptorSet> writeDescriptorSets(3);
		aux::Describe::buffer(writeDescriptorSets[0], ds[i], 0, &(uniformBuffers[i].descriptor));
		aux::Describe::buffer(writeDescriptorSets[1], ds[i], 1, &(*m_rParamUniformBuffers)[i].descriptor);
		aux::Describe::image(writeDescriptorSets[2], ds[i], 2, &m_rTextures->prefilteredCube.descriptor);
		aux::DescriptorSet::updateW(writeDescriptorSets);
	}
}

void Skybox::draw(Model& model)
{
	aux::DescriptorSet dsSkybox(*m_rCurrentDS);
	dsSkybox.bindToGraphics(*m_rCmdBuf, *m_rPipelineLayout);
	m_rPipeline->bindToGraphic(*m_rCmdBuf);
	model.draw(*m_rCmdBuf);
}

}