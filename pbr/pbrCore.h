#pragma once
#include "..\vk-all.h"
#include "..\gltf\gltf.h"
namespace pbr
{
struct ShaderParams {
	glm::vec4 lightDir;
	float exposure = 4.5f;
	float gamma = 2.2f;
	float prefilteredCubeMipLevels;
	float scaleIBLAmbient = 1.0f;
	float debugViewInputs = 0;
	float debugViewEquation = 0;
};

struct PbrConfig {
	bool multiSampling = true;
	VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_4_BIT;
};

class Cubemap: public vks::TextureCubeMap 
{
};

class Texture2D : public vks::Texture2D
{
};

}

