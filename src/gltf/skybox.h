#pragma once
#include "..\vk.h"
#include "..\auxVk\auxVk.h"
#include "model.h"

class aux::Image;

namespace gltf
{
class Skybox: public Model {

public:
	Skybox();
	~Skybox();
	void updateDS(VkDescriptorPool& descriptorPool);
};
}