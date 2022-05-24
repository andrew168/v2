#pragma once
#include "..\vk.h"
#include "..\gltf\gltf.h"
namespace v2
{
class aux::Image;

class Pbr
{
	static aux::Image* m_pBrdfLutImage; // 依赖vks,不能delete，

public:
	static aux::Image& generateBRDFLUT();
	static void generateCubemaps(std::vector<aux::Image>& cubemaps, gltf::Models models, vks::Texture& texture);
};
}

