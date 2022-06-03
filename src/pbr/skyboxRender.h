#pragma once
#include "..\vk.h"
#include "..\gltf\gltf.h"
#include "pbr.h"

namespace v2
{
class SkyboxRender : public Pbr
{
public:
	VkDescriptorSet* m_rDS;
    std::vector<VkDescriptorSet> skyboxDS;

public: 
    SkyboxRender();
    ~SkyboxRender();
    void init(uint32_t swapChainCount, Camera& camera, VkRenderPass& renderPass);
    void createDPool(VkDescriptorPool& descriptorPool);
    void updateSkyboxBodyDS(VkDescriptorPool& descriptorPool);
    void updateSceneMeshUBDS(VkDescriptorPool& descriptorPool);
    void updateDS(VkDescriptorPool& descriptorPool);
	void draw(vkglTF::Model& model, uint32_t dsID, VkCommandBuffer& cmdBuf);
};
}

