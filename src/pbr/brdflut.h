#pragma once
#include "..\vk.h"
#include "..\gltf\gltf.h"
namespace v2
{
class aux::Image;
class gltf::Model;

class Pbr
{
	static aux::Image* m_pBrdfLutImage; // 依赖vks,不能delete，

public:
	static aux::Image& generateBRDFLUT();
	static void generateCubemaps(std::vector<aux::Image>& cubemaps, gltf::Model& skyboxMmodel, vks::Texture& texture);
	static aux::Image* Pbr::generateCubemap(gltf::Model& skyboxModel, vks::Texture& texture, VkFormat format, int32_t dim,
		std::vector<aux::ShaderDescription> shaders,
		uint32_t constsSize, const void* constsData);
};
}

