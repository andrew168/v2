#pragma once
#include "..\vk.h"
#include "..\auxVk\auxVk.h"

namespace gltf
{

enum PBRWorkflows {
	PBR_WORKFLOW_METALLIC_ROUGHNESS = 0,
	PBR_WORKFLOW_SPECULAR_GLOSINESS = 1
};

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
public:
	Model();
	~Model();
	vkglTF::Model* toVkglTF() {
		return static_cast<vkglTF::Model*> (this);
	}
	void update(int32_t animationIndex, float frameTimer);
	void updateShaderValues(Camera& camera);
	void Model::centerAndScale();
	void applyShaderValues(uint32_t currentBuffer);
	void prepareUniformBuffers();
	std::vector<Buffer>& getUB() { return uniformBuffers; }
};

struct PushConstBlockMaterial {
	glm::vec4 baseColorFactor;
	glm::vec4 emissiveFactor;
	glm::vec4 diffuseFactor;
	glm::vec4 specularFactor;
	float workflow;
	int colorTextureSet;
	int PhysicalDescriptorTextureSet;
	int normalTextureSet;
	int occlusionTextureSet;
	int emissiveTextureSet;
	float metallicFactor;
	float roughnessFactor;
	float alphaMask;
	float alphaMaskCutoff;
};

class Render {
	VkDescriptorSet& m_sceneDescriptorSet;
	VkCommandBuffer& m_cmdBuf;
	VkPipelineLayout& m_pipelineLayout;
	aux::Pipeline & m_pipeline;		
	aux::Pipeline* m_pAuxPipelineBlend;
public:
	Render(VkDescriptorSet& sceneDescriptorSet,
		VkCommandBuffer& cmdBuf,
		VkPipelineLayout& pipelineLayout,
		aux::Pipeline& pipeline,
		aux::Pipeline* pPipelineBlend = nullptr);

	void drawNode(vkglTF::Node* node, 
		vkglTF::Material::AlphaMode alphaMode);

	void drawT(vkglTF::Model& model);
	void draw(vkglTF::Model& model);
};
}