#include "gltf.h"
#include "../auxVk/auxVk.h"

namespace gltf
{
using namespace aux;
Model::Model():
	vkglTF::Model(),
	m_pMaterialDSL(nullptr),
	m_animationTimer(0.0f)
{
}

Model::~Model()
{
	destroy(Device::getR());
	for (auto buffer : uniformBuffers) {
		buffer.destroy();
	}

	if (m_pMaterialDSL != nullptr)
	{
		delete m_pMaterialDSL;
		m_pMaterialDSL = nullptr;
	}
}

void Model::update(int32_t animationIndex, float frameTimer)
{
	if (animations.size() > 0) {
		m_animationTimer += frameTimer;
		if (m_animationTimer > animations[animationIndex].end) {
			m_animationTimer -= animations[animationIndex].end;
		}
		updateAnimation(animationIndex, m_animationTimer);
	}
}

void Model::prepareUniformBuffers()
{
	for (auto& uniformBuffer : uniformBuffers) {
		uniformBuffer.create(Device::getVksDevice(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(shaderValues));
	}
}

void Model::updateShaderValues(Camera& camera)
{
	// Scene
	shaderValues.projection = camera.matrices.perspective;
	shaderValues.view = camera.matrices.view;

	// 此Model Matrix， 只适合Skybox， 不适合其它元素
	shaderValues.model = glm::mat4(glm::mat3(camera.matrices.view));

	shaderValues.camPos = glm::vec3(
		-camera.position.z * sin(glm::radians(camera.rotation.y)) * cos(glm::radians(camera.rotation.x)),
		-camera.position.z * sin(glm::radians(camera.rotation.x)),
		camera.position.z * cos(glm::radians(camera.rotation.y)) * cos(glm::radians(camera.rotation.x))
	);
}

void Model::centerAndScale()
{
	// Center and scale model
	float scale = (1.0f / std::max(aabb[0][0], std::max(aabb[1][1], aabb[2][2]))) * 0.5f;
	glm::vec3 translate = -glm::vec3(aabb[3][0], aabb[3][1], aabb[3][2]);
	translate += -0.5f * glm::vec3(aabb[0][0], aabb[1][1], aabb[2][2]);

	shaderValues.model = glm::mat4(1.0f);
	shaderValues.model[0][0] = scale;
	shaderValues.model[1][1] = scale;
	shaderValues.model[2][2] = scale;
	shaderValues.model = glm::translate(shaderValues.model, translate);
}

void Model::applyShaderValues(uint32_t currentBuffer)
{
	memcpy(uniformBuffers[currentBuffer].mapped, &shaderValues, sizeof(shaderValues));
}

void Model::setupMaterialDSL(VkDescriptorPool& descriptorPool,
	VkDescriptorImageInfo& defaultTextureDesc)
{
	// Material (samplers)	
	std::vector<VkDescriptorSetLayoutBinding> dslBindings = {
		{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
		{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
		{ 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
		{ 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
		{ 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
	};
	m_pMaterialDSL = new aux::DescriptorSetLayout(dslBindings);

	// Per-Material descriptor sets
	for (auto& material : materials) {
		aux::DescriptorSet::allocate(material.descriptorSet,
			descriptorPool, m_pMaterialDSL->get());
		std::vector<VkDescriptorImageInfo> imageDescriptors = {
			defaultTextureDesc,
			defaultTextureDesc,
			material.normalTexture ? material.normalTexture->descriptor : defaultTextureDesc,
			material.occlusionTexture ? material.occlusionTexture->descriptor : defaultTextureDesc,
			material.emissiveTexture ? material.emissiveTexture->descriptor : defaultTextureDesc
		};

		// TODO: glTF specs states that metallic roughness should be preferred, even if specular glosiness is present

		if (material.pbrWorkflows.metallicRoughness) {
			if (material.baseColorTexture) {
				imageDescriptors[0] = material.baseColorTexture->descriptor;
			}
			if (material.metallicRoughnessTexture) {
				imageDescriptors[1] = material.metallicRoughnessTexture->descriptor;
			}
		}

		if (material.pbrWorkflows.specularGlossiness) {
			if (material.extension.diffuseTexture) {
				imageDescriptors[0] = material.extension.diffuseTexture->descriptor;
			}
			if (material.extension.specularGlossinessTexture) {
				imageDescriptors[1] = material.extension.specularGlossinessTexture->descriptor;
			}
		}

		std::vector<VkWriteDescriptorSet> writeDescriptorSets(5);
		for (size_t i = 0; i < imageDescriptors.size(); i++) {
			aux::Describe::image(writeDescriptorSets[i], material.descriptorSet,
				static_cast<uint32_t>(i), &imageDescriptors[i]);
		}

		DescriptorSet::updateW(writeDescriptorSets);
	}
}
}
