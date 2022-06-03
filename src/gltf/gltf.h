#pragma once
#include "..\vk.h"
#include "..\auxVk\auxVk.h"

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
	std::vector<Buffer> uniformBuffers;
	UBOMatrices shaderValues;
	aux::DescriptorSetLayout* m_pMaterialDSL;

public:
	Model();
	~Model();
	vkglTF::Model* toVkglTF() {
		return static_cast<vkglTF::Model*> (this);
	}
	void updateMaterialDS(VkDescriptorPool &descriptorPool,
		VkDescriptorImageInfo &defaultTextureDesc);
	void update(int32_t animationIndex, float frameTimer);
	void updateShaderValues(Camera& camera);
	void Model::centerAndScale();
	void applyShaderValues(uint32_t currentBuffer);
	void prepareUniformBuffers();
	std::vector<Buffer>& getUB() { return uniformBuffers; }
	VkDescriptorSetLayout* getMaterialDSL() { return m_pMaterialDSL->get(); }
};
}