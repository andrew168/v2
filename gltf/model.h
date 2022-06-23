#pragma once
#include "..\vk-all.h"
#include "..\auxVk\auxVk.h"
#include "..\base\camera.hpp"
#include "..\base\VulkanUtils.hpp"
#include "..\base\VulkanglTFModel.h"
#include "..\pbr\pbrCore.h"

namespace pbr 
{
class Pbr;
}

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

enum PBRWorkflows {
	PBR_WORKFLOW_METALLIC_ROUGHNESS = 0,
	PBR_WORKFLOW_SPECULAR_GLOSINESS = 1
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

class Model: public vkglTF::Model {
	float m_animationTimer = 0.0f;
	UBOMatrices shaderValues;

protected:
	VkCommandBuffer* m_rCmdBuf;
	VkPipelineLayout* m_rPipelineLayout;
	aux::Pipeline* m_rPipeline;
	aux::Pipeline* m_pAuxPipelineBlend;
	std::vector<Buffer> uniformBuffers;
	std::vector<VkDescriptorSet> ds;  //本model所有Shader参数的descriptorSet
	VkDescriptorSet* m_rCurrentDS;
	Textures* m_rTextures;
	std::vector<Buffer>* m_rPbrShaderParamUBs;
	
	static aux::DescriptorSetLayout* m_pDSL;
	aux::DescriptorSetLayout* m_pMaterialDSL;
	aux::DescriptorSetLayout* m_pMeshDSL;
	std::vector<VkDescriptorSetLayout> m_DSLs;

public:
	Model();
	~Model();
	void init(uint32_t swapChainCount,
		std::vector<Buffer>& pParamUniformBuffers,
		Textures& pTextures);

	inline uint32_t getMaterialCount() {
		uint32_t sum = 0;
		for (auto& material : materials) {
			sum++;
		}
		return sum;
	}
	
	inline uint32_t getMeshCount() {
		uint32_t sum = 0;
		for (auto node : linearNodes) {
			if (node->mesh) {	// 每一个GLTF模型都有自己的mesh列表
				sum++;
			}
		}
		return sum;
	}

	vkglTF::Model* toVkglTF() {
		return static_cast<vkglTF::Model*> (this);
	}
	void updateMaterialDS(VkDescriptorPool &descriptorPool,
		VkDescriptorImageInfo &defaultTextureDesc);
	void updateDS(VkDescriptorPool& descriptorPool);
	void updateMeshUBDS(VkDescriptorPool& descriptorPool);
	void updateNodeUBDS(vkglTF::Node* node, VkDescriptorPool& descriptorPool);
	void update(int32_t animationIndex, float frameTimer);
	void updateShaderValues(Camera& camera);
	void centerAndScale();
	void applyShaderValues(uint32_t currentBuffer);
	void createUB();
	std::vector<VkDescriptorSetLayout>& getDSLs();

	void drawNode(vkglTF::Node* node,
		vkglTF::Material::AlphaMode alphaMode);

	void drawT(vkglTF::Model& model, VkCommandBuffer& cmdBuf,
		uint32_t dsID, pbr::Pbr &pbr); //T: Transparent supported, 

	static void Model::createDSL(VkDescriptorPool& descriptorPool);
};
}