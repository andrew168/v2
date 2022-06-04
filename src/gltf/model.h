#pragma once
#include "..\vk.h"
#include "..\auxVk\auxVk.h"

struct Textures {
	vks::TextureCubeMap environmentCube;
	vks::Texture2D empty;
	vks::Texture2D lutBrdf;
	vks::TextureCubeMap irradianceCube;
	vks::TextureCubeMap prefilteredCube;
};

namespace gltf
{
struct UBOMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	glm::vec3 camPos;
};

class Model: public vkglTF::Model {
	float m_animationTimer = 0.0f;
	UBOMatrices shaderValues;
	aux::DescriptorSetLayout* m_pMaterialDSL;

protected:
	std::vector<Buffer> uniformBuffers;
	std::vector<VkDescriptorSet> ds;  //本model所有Shader参数的descriptorSet
	Textures* m_rTextures;
	std::vector<Buffer>* m_rParamUniformBuffers;
	
	static aux::DescriptorSetLayout* m_pDSL;

public:
	Model();
	~Model();
	void init(uint32_t swapChainCount,
		std::vector<Buffer>& pParamUniformBuffers,
		Textures& pTextures);

	vkglTF::Model* toVkglTF() {
		return static_cast<vkglTF::Model*> (this);
	}
	void updateMaterialDS(VkDescriptorPool &descriptorPool,
		VkDescriptorImageInfo &defaultTextureDesc);
	void updateDS(VkDescriptorPool& descriptorPool);
	void update(int32_t animationIndex, float frameTimer);
	void updateShaderValues(Camera& camera);
	void Model::centerAndScale();
	void applyShaderValues(uint32_t currentBuffer);
	void createUB();
	std::vector<Buffer>& getUB() { return uniformBuffers; }
	VkDescriptorSetLayout* getMaterialDSL() { return m_pMaterialDSL->get(); }
	VkDescriptorSetLayout* getDSL() { return m_pDSL->get(); }
	std::vector<VkDescriptorSet>& getDS() { return ds; }

	static void Model::createDSL(VkDescriptorPool& descriptorPool);
};
}