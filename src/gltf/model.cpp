#include "model.h"
#include "../auxVk/auxVk.h"
#include "../pbr/pbr.h"

namespace gltf
{
using namespace aux;

aux::DescriptorSetLayout* Model::m_pDSL = nullptr;

Model::Model():
	vkglTF::Model(),
	m_pMaterialDSL(nullptr),
	m_animationTimer(0.0f)
{
}

void Model::init(uint32_t swapChainCount,
	std::vector<Buffer>& paramUniformBuffers,
	Textures& textures)
{
	m_rParamUniformBuffers = &paramUniformBuffers;
	m_rTextures = &textures;
	uniformBuffers.resize(swapChainCount);
	ds.resize(swapChainCount);
}

Model::~Model()
{
	delete m_pDSL;
	m_pDSL = nullptr;

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

void Model::createUB()
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

// shader参数值的更改
// 先更改shaderValues，再memcpy到UB中，
void Model::applyShaderValues(uint32_t currentBuffer)
{
	memcpy(uniformBuffers[currentBuffer].mapped, &shaderValues, sizeof(shaderValues));
}

void Model::updateMaterialDS(VkDescriptorPool& descriptorPool,
	VkDescriptorImageInfo& defaultTextureDesc)
{
	// 5步方法：用DS上传shader参数到device：
	//  binding到槽，建立layout，allocate DS，汇集到DS，update到device
	// Material (samplers)	
	std::vector<VkDescriptorSetLayoutBinding> dslBindings = {
		{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
		{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
		{ 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
		{ 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
		{ 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
	};
	m_pMaterialDSL = new aux::DescriptorSetLayout(dslBindings);

	// 每种材质1个DS，描述其5个texture，如果法矢图/遮挡图/发光图没有特殊描述就用缺省的
	// 材质的DS归material自己保存，各个SwapChain公用（因为不改变）
	for (auto& material : materials) {
		aux::DescriptorSet::allocate(material.descriptorSet,
			descriptorPool, m_pMaterialDSL->get());
		std::vector<VkDescriptorImageInfo> imageDescriptors = {
			defaultTextureDesc,  // 基础颜色图，baseColorTexture;
			defaultTextureDesc,  // 金属粗糙度图，metallicRoughnessTexture;
			material.normalTexture ? material.normalTexture->descriptor : defaultTextureDesc,
			material.occlusionTexture ? material.occlusionTexture->descriptor : defaultTextureDesc,
			material.emissiveTexture ? material.emissiveTexture->descriptor : defaultTextureDesc
		};

		// TODO: glTF specs states that metallic roughness should be preferred, even if specular glosiness is present

		// PBR的两种算法： 金属粗糙度 和 镜面光泽度（ToDo：应该改为金属粗糙度优先）
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


/*设置DS，保存在Device上的DS Pool中
* DS: scene的Uniform Buffer，pbr参数的UB， 3个环境Cubemap
*/
void Model::createDSL(VkDescriptorPool& descriptorPool)
{
	// DSLB: 绑定slot号，UB还是Sampler，几个, 到VS还是FS？
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
		{ 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
		{ 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
		{ 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
		{ 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
	};

	// binding和DSL都是临时的，只为了到pool中allocate出DS,
	m_pDSL = new aux::DescriptorSetLayout(setLayoutBindings);
}

void Model::updateDS(VkDescriptorPool & descriptorPool)
{
	if (m_pDSL == nullptr) {
		Model::createDSL(descriptorPool);
	}
		// 对所有SwapChain用的DS都update，
	// 用1个DS一次性update描述SceneBody的5个D (1个整体UB + 1个pbr UB + 3个环境Sampler）
	for (auto i = 0; i < ds.size(); i++) { // ds数量就是swapChain数量，
		aux::DescriptorSet::allocate(ds[i], descriptorPool, m_pDSL->get());

		// VkWriteDescriptorSet本来可以集合多个D，为了简化，这里只存1个D
		std::vector<VkWriteDescriptorSet> writeDescriptorSets(5);
		aux::Describe::buffer(writeDescriptorSets[0], ds[i], 0, &(uniformBuffers[i].descriptor));
		aux::Describe::buffer(writeDescriptorSets[1], ds[i], 1, &(*m_rParamUniformBuffers)[i].descriptor);
		aux::Describe::image(writeDescriptorSets[2], ds[i], 2, &m_rTextures->irradianceCube.descriptor);
		aux::Describe::image(writeDescriptorSets[3], ds[i], 3, &m_rTextures->prefilteredCube.descriptor);
		aux::Describe::image(writeDescriptorSets[4], ds[i], 4, &m_rTextures->lutBrdf.descriptor);
		// 如果此DS已经被正在执行的某CmdX绑定，则CmdX立即变成invalid，
		// 所有需要多个DS供swapchain轮流使用
		aux::DescriptorSet::updateW(writeDescriptorSets);
	}
}
void Model::config(VkDescriptorSet& sceneDescriptorSet,
	VkCommandBuffer& cmdBuf,
	VkPipelineLayout & pipelineLayout,
	aux::Pipeline& pipeline,
	aux::Pipeline* pPipelineBlend)
{
	m_rSceneDescriptorSet = &sceneDescriptorSet;
	m_rCmdBuf = &cmdBuf;
	m_rPipelineLayout = &pipelineLayout;
	m_rPipeline = &pipeline;
	m_pAuxPipelineBlend = pPipelineBlend;
}

void Model::drawT(vkglTF::Model& model)
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

void Model::drawNode(vkglTF::Node* node,
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
					pushConstBlockMaterial.PhysicalDescriptorTextureSet =
						primitive->material.extension.specularGlossinessTexture != nullptr ?
						primitive->material.texCoordSets.specularGlossiness : -1;
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
