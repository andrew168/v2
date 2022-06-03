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
	VkCommandBuffer* m_rCmdBuf;
	VkPipelineLayout* m_rPipelineLayout;
	aux::Pipeline* m_rPipeline;
	aux::Pipeline* m_pAuxPipelineBlend;
    std::vector<VkDescriptorSet> skyboxDS;

public: 
    SkyboxRender();
    ~SkyboxRender();
    void init(uint32_t swapChainCount, Camera& camera, VkRenderPass& renderPass);
    void setupDescriptors(VkDescriptorPool& descriptorPool);
	void configPbr(uint32_t dsID,
		VkCommandBuffer& cmdBuf);

	void draw(vkglTF::Model& model);
};
}

