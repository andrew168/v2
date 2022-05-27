#include "gltf.h"
#include "../auxVk/auxVk.h"

namespace gltf
{
Render::Render(VkDescriptorSet& sceneDescriptorSet, 
	VkCommandBuffer& cmdBuf, 
	VkPipelineLayout& pipelineLayout):
	m_sceneDescriptorSet(sceneDescriptorSet),
	m_cmdBuf(cmdBuf),
	m_pipelineLayout(pipelineLayout)
{
}

void Render::drawNode(vkglTF::Node* node, 
	vkglTF::Material::AlphaMode alphaMode) 
{
	if (node->mesh) {
		// Render mesh primitives
		for (vkglTF::Primitive* primitive : node->mesh->primitives) {
			if (primitive->material.alphaMode == alphaMode) {

				const std::vector<VkDescriptorSet> dSets = {
					m_sceneDescriptorSet,
					primitive->material.descriptorSet,
					node->mesh->uniformBuffer.descriptorSet,
				};
				aux::DescriptorSet::bindToGraphics(dSets,
					m_cmdBuf,
					m_pipelineLayout);

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
				aux::CommandBuffer auxCmdBuf(m_cmdBuf);
				auxCmdBuf.pushConstantsToFS(m_pipelineLayout, 0,
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