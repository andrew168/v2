#pragma once
#include "..\vk.h"
#include "..\gltf\gltf.h"
namespace v2
{
class aux::Image;
class gltf::Model;

struct Textures {
	vks::TextureCubeMap environmentCube;
	vks::Texture2D empty;
	vks::Texture2D lutBrdf;
	vks::TextureCubeMap irradianceCube;
	vks::TextureCubeMap prefilteredCube;
};

class Cubemap: public vks::TextureCubeMap 
{
};

class Texture2D : public vks::Texture2D
{
};

class Pbr
{
	aux::DescriptorSetLayout* m_pDSL;
	std::vector<VkDescriptorSet> sceneDS;
	static aux::Image* m_pBrdfLutImage; // 依赖vks,不能delete，
	gltf::Model *m_pSceneModel;
	gltf::Model *m_pSkyboxModel;
	std::vector<Buffer>* m_pParamUniformBuffers;
	Textures* m_pTextures;

public:
	Pbr();
	~Pbr();
	void init(uint32_t swapChainCount);
	void config(gltf::Model& sceneModel,
		gltf::Model& skyboxModel,
		std::vector<Buffer>& paramUniformBuffers,
		Textures& textures);

	void setupDSL(VkDescriptorPool& descriptorPool);
	static aux::Image& generateBRDFLUT();
	static void generateCubemaps(std::vector<aux::Image>& cubemaps, gltf::Model& skyboxMmodel, vks::Texture& texture);
	static aux::Image* Pbr::generateCubemap(gltf::Model& skyboxModel, vks::Texture& texture, VkFormat format, int32_t dim,
		std::vector<aux::ShaderDescription> shaders,
		uint32_t constsSize, const void* constsData);
	std::vector<VkDescriptorSet>& getDS() { return sceneDS; }
	VkDescriptorSetLayout* getDSL() { return m_pDSL->get(); }
};
}

