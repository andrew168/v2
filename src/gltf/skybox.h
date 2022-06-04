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
	void draw(Model& model, 
		VkCommandBuffer& cmdBuf,
		uint32_t dsID,
		pbr::Pbr& pbr);
};
}