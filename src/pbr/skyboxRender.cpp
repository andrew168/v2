#include "..\vk.h"
#include "..\auxVk\auxVk.h"
#include "..\gltf\gltf.h"
#include "pbr.h"
#include "skyboxRender.h"

using namespace aux;

namespace v2
{
SkyboxRender::SkyboxRender():
	Pbr()
{
}

SkyboxRender::~SkyboxRender()
{
}

void SkyboxRender::init(uint32_t swapChainCount, Camera& camera, VkRenderPass& renderPass)
{
	Pbr::init(swapChainCount, camera, renderPass);
}

void SkyboxRender::createDPool(VkDescriptorPool& descriptorPool)
{
	uint32_t imageSamplerCount = 0;
	uint32_t materialCount = 0;
	uint32_t meshCount = 0;

	// Environment samplers (radiance, irradiance, brdf lut)
	imageSamplerCount += 3;

	// 创建DPool在Device上，容纳Render需要的所有sample和UB的descriptor。
	// DPool的大小计算：
	// * Sampler数量：
	//	+ 每一个Model的每一种材质有5张texture
	//	+ 整体环境的3个Cubemap
	// * UB (Uniform Buffer)数量：
	//	+ 每mesh一个UB
	//	+ 4(每model整体的UB, pbr参数的UB, ??)
	// * 每一个swapChainImage，需要1份，==》 放大倍数 swapChainImageCount
	std::vector<vkglTF::Model*> modellist = { m_pSkyboxModel, m_pSceneModel };
	for (auto& model : modellist) {
		for (auto& material : model->materials) {
			imageSamplerCount += 5;  // 每种PBR材质有5个texture作为sampler
			materialCount++;		  //这个种材质的5个sampler放在1个set中描述
		}
		for (auto node : model->linearNodes) {
			if (node->mesh) {	// 每一个GLTF模型都有自己的mesh列表
				meshCount++;	// 每一个mesh的UB，放在1个set中
			}
		}
	}

	std::vector<VkDescriptorPoolSize> poolSizes = {
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (4 + meshCount) * m_swapChainImageCount },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageSamplerCount * m_swapChainImageCount }
	};
	uint32_t maxSets = (2 + materialCount + meshCount) * m_swapChainImageCount;
	aux::DescriptorPool::create(descriptorPool, poolSizes, maxSets);
}

void SkyboxRender::updateDS(VkDescriptorPool& descriptorPool)
{
	//上传shader参数到device的5步方法：用DS：
	//binding到槽，建立layout，allocate DS，汇集到DS，update到device
	//
	//Shader的参数有2大类：UB(Uniform Buffer)和Sampler，常见的有：
	//pbr的UB，body整体的UB和Sampler，每mesh的UB，每material的Sampler
	//
	//他们都必须用D(Descriptor)描述，汇集到DS(DescriptorSet)，再上传给Device。
	createDPool(descriptorPool);
	m_pSceneModel->updateDS(descriptorPool);  //DS: scene的Uniform Buffer，pbr参数的UB， 3个环境Cubemap
	// 材质的DS归material自己保存，各个SwapChain公用（因为不改变）
	m_pSceneModel->updateMaterialDS(descriptorPool, m_pTextures->empty.descriptor);
	m_pSceneModel->updateMeshUBDS(descriptorPool); // 更新每一个Mesh's UB的DS，mesh记录自己的DS
	m_pSkyboxModel->updateDS(descriptorPool); //DS: skybox的Uniform Buffer，pbr参数的UB， 1个环境Cubemap: Prefilted
}

void SkyboxRender::draw(gltf::Model& model, uint32_t dsID, VkCommandBuffer& cmdBuf)
{
	aux::DescriptorSet dsSkybox(model.getDS()[dsID]);
	dsSkybox.bindToGraphics(cmdBuf, *getPipelineLayout());
	pAuxPipelineSkybox->bindToGraphic(cmdBuf);
	model.draw(cmdBuf);
}
}
