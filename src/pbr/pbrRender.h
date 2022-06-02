#pragma once
#include "..\vk.h"
#include "..\gltf\gltf.h"
namespace v2
{
class Render {
	VkDescriptorSet* m_rSceneDescriptorSet;
	VkCommandBuffer* m_rCmdBuf;
	VkPipelineLayout* m_rPipelineLayout;
	aux::Pipeline* m_rPipeline;
	aux::Pipeline* m_pAuxPipelineBlend;
public:
	Render();
	~Render();
	void config(VkDescriptorSet& sceneDescriptorSet,
		VkCommandBuffer& cmdBuf,
		Pbr& pbr,
		aux::Pipeline& pipeline,
		aux::Pipeline* pPipelineBlend = nullptr);

	void drawNode(vkglTF::Node* node,
		vkglTF::Material::AlphaMode alphaMode);

	void drawT(vkglTF::Model& model); //T: Transparent supported, 
	void draw(vkglTF::Model& model);
};
}

