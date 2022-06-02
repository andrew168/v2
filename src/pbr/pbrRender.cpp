#include "../gltf/gltf.h"
#include "../auxVk/auxVk.h"
#include "pbr.h"
#include "pbrRender.h"

namespace v2
{
Render::Render()
{
}

Render::~Render()
{
}

void Render::config(VkDescriptorSet& sceneDescriptorSet, 
	VkCommandBuffer& cmdBuf, 
	VkPipelineLayout& pipelineLayout,
	aux::Pipeline& pipeline,
	aux::Pipeline* pPipelineBlend)
{
	m_rSceneDescriptorSet = &sceneDescriptorSet;
	m_rCmdBuf = &cmdBuf;
	m_rPipelineLayout = &pipelineLayout;
	m_rPipeline = &pipeline;
	m_pAuxPipelineBlend = pPipelineBlend;
}

void Render::draw(vkglTF::Model& model)
{
	aux::DescriptorSet dsSkybox(*m_rSceneDescriptorSet);
	dsSkybox.bindToGraphics(*m_rCmdBuf, *m_rPipelineLayout);
	m_rPipeline->bindToGraphic(*m_rCmdBuf);
	model.draw(*m_rCmdBuf);
}

void Render::drawT(vkglTF::Model& model)
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

void Render::drawNode(vkglTF::Node* node, 
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
