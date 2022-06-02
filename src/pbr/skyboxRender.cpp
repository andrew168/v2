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
	skyboxDS.resize(swapChainCount);
}

void SkyboxRender::setupDescriptors(VkDescriptorPool& descriptorPool)
{
	/*
		Descriptor Pool
	*/
	uint32_t imageSamplerCount = 0;
	uint32_t materialCount = 0;
	uint32_t meshCount = 0;

	// Environment samplers (radiance, irradiance, brdf lut)
	imageSamplerCount += 3;

	// 每一个GLTF模型都有自己的材质列表和mesh列表
	std::vector<vkglTF::Model*> modellist = { m_pSkyboxModel, m_pSceneModel };
	for (auto& model : modellist) {
		for (auto& material : model->materials) {
			imageSamplerCount += 5;  // 每种PBR材质有5个texture作为sampler
			materialCount++;
		}
		for (auto node : model->linearNodes) {
			if (node->mesh) {
				meshCount++;
			}
		}
	}

	std::vector<VkDescriptorPoolSize> poolSizes = {
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (4 + meshCount) * m_swapChainImageCount },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageSamplerCount * m_swapChainImageCount }
	};
	uint32_t maxSets = (2 + materialCount + meshCount) * m_swapChainImageCount;
	aux::DescriptorPool::create(descriptorPool, poolSizes, maxSets);

	/*
		Descriptor sets
	*/

	setupDSL(descriptorPool);
	m_pSceneModel->setupMaterialDSL(descriptorPool, m_pTextures->empty.descriptor);
	// Model node (matrices)
	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
			{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
		};
		pAuxDSLayoutNode = new aux::DescriptorSetLayout(setLayoutBindings);

		// Per-Node descriptor set
		for (auto& node : m_pSceneModel->nodes) {
			setupNodeDescriptorSet(node, descriptorPool);
		}
	}

	// Skybox (fixed set)
	for (auto i = 0; i < m_pSkyboxModel->getUB().size(); i++) {
		aux::DescriptorSet::allocate(skyboxDS[i],
			descriptorPool, getDSL());

		std::vector<VkWriteDescriptorSet> writeDescriptorSets(3);
		aux::Describe::buffer(writeDescriptorSets[0], skyboxDS[i], 0, &(m_pSkyboxModel->getUB()[i].descriptor));
		aux::Describe::buffer(writeDescriptorSets[1], skyboxDS[i], 1, &paramUniformBuffers[i].descriptor);
		aux::Describe::image(writeDescriptorSets[2], skyboxDS[i], 2, &m_pTextures->prefilteredCube.descriptor);
		aux::DescriptorSet::updateW(writeDescriptorSets);
	}
}

void SkyboxRender::configPbr(uint32_t dsID,
	VkCommandBuffer& cmdBuf)
{
	m_rSceneDescriptorSet = &skyboxDS[dsID];
	m_rCmdBuf = &cmdBuf;
	m_rPipelineLayout = getPipelineLayout();
	m_rPipeline = pAuxPipelineSkybox;
}

void SkyboxRender::draw(vkglTF::Model& model)
{
	aux::DescriptorSet dsSkybox(*m_rSceneDescriptorSet);
	dsSkybox.bindToGraphics(*m_rCmdBuf, *m_rPipelineLayout);
	m_rPipeline->bindToGraphic(*m_rCmdBuf);
	model.draw(*m_rCmdBuf);
}

void SkyboxRender::drawT(vkglTF::Model& model)
{
	m_rPipeline->bindToGraphic(*m_rCmdBuf);

	VkDeviceSize offsets[1] = { 0 };
	aux::CommandBuffer auxCmdBuf(*m_rCmdBuf);
	auxCmdBuf.bindVertexBuffers(0, 1, &model.vertices.buffer, offsets);
	if (model.indices.buffer != VK_NULL_HANDLE) {
		auxCmdBuf.bindIndexBuffer(model.indices.buffer);
	}

	// 先普通材质，alpha mask, 最后透明体
	for (auto node : model.nodes) {
		drawNode(node, vkglTF::Material::ALPHAMODE_OPAQUE);
	}
	for (auto node : model.nodes) {
		drawNode(node, vkglTF::Material::ALPHAMODE_MASK);
	}
	// TODO: Correct depth sorting
	if (m_pAuxPipelineBlend) {
		m_pAuxPipelineBlend->bindToGraphic(*m_rCmdBuf); // 透明体才需要blend
		for (auto node : model.nodes) {
			drawNode(node, vkglTF::Material::ALPHAMODE_BLEND);
		}
	}
}

void SkyboxRender::drawNode(vkglTF::Node* node,
	vkglTF::Material::AlphaMode alphaMode)
{
	if (node->mesh) {
		// Render mesh primitives
		for (vkglTF::Primitive* primitive : node->mesh->primitives) {
			if (primitive->material.alphaMode == alphaMode) {

				const std::vector<VkDescriptorSet> dSets = {
					*m_rSceneDescriptorSet,
					primitive->material.descriptorSet,
					node->mesh->uniformBuffer.descriptorSet,
				};
				aux::DescriptorSet::bindToGraphics(dSets,
					*m_rCmdBuf,
					*m_rPipelineLayout);

				// Pass material parameters as push constants
				PushConstBlockMaterial pushConstBlockMaterial{};
				pushConstBlockMaterial.emissiveFactor = primitive->material.emissiveFactor;
				// To save push constant space, availabilty and texture coordiante set are combined
				// -1 = texture not used for this material, >= 0 texture used and index of texture coordinate set
				pushConstBlockMaterial.colorTextureSet = primitive->material.baseColorTexture != nullptr ? primitive->material.texCoordSets.baseColor : -1;
				pushConstBlockMaterial.normalTextureSet = primitive->material.normalTexture != nullptr ? primitive->material.texCoordSets.normal : -1;
				pushConstBlockMaterial.occlusionTextureSet = primitive->material.occlusionTexture != nullptr ? primitive->material.texCoordSets.occlusion : -1;
				pushConstBlockMaterial.emissiveTextureSet = primitive->material.emissiveTexture != nullptr ? primitive->material.texCoordSets.emissive : -1;
				pushConstBlockMaterial.alphaMask = static_cast<float>(primitive->material.alphaMode == vkglTF::Material::ALPHAMODE_MASK);
				pushConstBlockMaterial.alphaMaskCutoff = primitive->material.alphaCutoff;

				// TODO: glTF specs states that metallic roughness should be preferred, even if specular glosiness is present

				if (primitive->material.pbrWorkflows.metallicRoughness) {
					// Metallic roughness workflow
					pushConstBlockMaterial.workflow = static_cast<float>(PBR_WORKFLOW_METALLIC_ROUGHNESS);
					pushConstBlockMaterial.baseColorFactor = primitive->material.baseColorFactor;
					pushConstBlockMaterial.metallicFactor = primitive->material.metallicFactor;
					pushConstBlockMaterial.roughnessFactor = primitive->material.roughnessFactor;
					pushConstBlockMaterial.PhysicalDescriptorTextureSet = primitive->material.metallicRoughnessTexture != nullptr ? primitive->material.texCoordSets.metallicRoughness : -1;
					pushConstBlockMaterial.colorTextureSet = primitive->material.baseColorTexture != nullptr ? primitive->material.texCoordSets.baseColor : -1;
				}

				if (primitive->material.pbrWorkflows.specularGlossiness) {
					// Specular glossiness workflow
					pushConstBlockMaterial.workflow = static_cast<float>(PBR_WORKFLOW_SPECULAR_GLOSINESS);
					pushConstBlockMaterial.PhysicalDescriptorTextureSet = primitive->material.extension.specularGlossinessTexture != nullptr ? primitive->material.texCoordSets.specularGlossiness : -1;
					pushConstBlockMaterial.colorTextureSet = primitive->material.extension.diffuseTexture != nullptr ? primitive->material.texCoordSets.baseColor : -1;
					pushConstBlockMaterial.diffuseFactor = primitive->material.extension.diffuseFactor;
					pushConstBlockMaterial.specularFactor = glm::vec4(primitive->material.extension.specularFactor, 1.0f);
				}
				aux::CommandBuffer auxCmdBuf(*m_rCmdBuf);
				auxCmdBuf.pushConstantsToFS(*m_rPipelineLayout, 0,
					sizeof(PushConstBlockMaterial),
					&pushConstBlockMaterial);

				if (primitive->hasIndices) {
					auxCmdBuf.drawIndexed(primitive->indexCount, 1, primitive->firstIndex);
				}
				else {
					auxCmdBuf.draw(primitive->vertexCount);
				}
			}
		}

	};
	for (auto child : node->children) {
		drawNode(child, alphaMode);
	}
}

}
