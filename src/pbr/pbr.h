﻿#pragma once
#include "..\vk.h"
#include "..\gltf\gltf.h"
#include "pbrCore.h"
namespace v2
{
class aux::Image;
class gltf::Model;

class Pbr
{
public:
	uint32_t m_swapChainImageCount;

	static aux::Image* m_pBrdfLutImage; // 依赖vks,不能delete，
	gltf::Model *m_pSceneModel;
	gltf::Skybox *m_pSkyboxModel;
	Textures* m_pTextures; 
	VkRenderPass* m_rRenderPass; // refer point to outside, do not destroy it

public:
	aux::Pipeline* pAuxPipelineBlend;
	aux::Pipeline* pAuxPipelinePbr;
	aux::Pipeline* pAuxPipelineSkybox;
	aux::PipelineLayout* pAuxPipelineLayout;
	std::vector<Buffer> paramUniformBuffers;
	ShaderValuesParams shaderValuesParams;
	Camera *m_pCamera;

public:
	Pbr();
	~Pbr();
	void init(uint32_t swapChainCount, Camera& camera, VkRenderPass& renderPass);
	void config(gltf::Model& sceneModel,
		gltf::Skybox& skyboxModel,
		Textures& textures);
	void createUB();
	void preparePipeline(PbrConfig& settings);
	VkPipelineLayout* getPipelineLayout() { return pAuxPipelineLayout->getP(); }
	void updateMeshUBDS(vkglTF::Node* node, VkDescriptorPool& descriptorPool);
	void applyShaderValues(uint32_t currentBuffer);
	void updateShaderValues();
	static aux::Image& generateBRDFLUT();
	static void generateCubemaps(std::vector<aux::Image>& cubemaps, gltf::Model& skyboxMmodel, vks::Texture& texture);
	static aux::Image* Pbr::generateCubemap(gltf::Model& skyboxModel, vks::Texture& texture, VkFormat format, int32_t dim,
		std::vector<aux::ShaderDescription> shaders,
		uint32_t constsSize, const void* constsData);
};
}
