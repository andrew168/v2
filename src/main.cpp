#include "v2\v2.h"
#include "auxVk\auxVk.h"

/*
	PBR example main class
*/
class VulkanExample : public VulkanExampleBase
{
public:
	struct Textures {
		vks::TextureCubeMap environmentCube;
		vks::Texture2D empty;
		vks::Texture2D lutBrdf;
		vks::TextureCubeMap irradianceCube;
		vks::TextureCubeMap prefilteredCube;
	} textures;

	struct Models {
		vkglTF::Model scene;
		vkglTF::Model skybox;
	} models;

	struct UniformBufferSet {
		Buffer scene;
		Buffer skybox;
		Buffer params;
	};

	struct UBOMatrices {
		glm::mat4 projection;
		glm::mat4 model;
		glm::mat4 view;
		glm::vec3 camPos;
	} shaderValuesScene, shaderValuesSkybox;

	struct shaderValuesParams {
		glm::vec4 lightDir;
		float exposure = 4.5f;
		float gamma = 2.2f;
		float prefilteredCubeMipLevels;
		float scaleIBLAmbient = 1.0f;
		float debugViewInputs = 0;
		float debugViewEquation = 0;
	} shaderValuesParams;

	VkPipelineLayout pipelineLayout;
	aux::PipelineLayout *pAuxPipelineLayout;
	aux::Pipeline* pAuxPipelineBlend;
	aux::Pipeline* pAuxPipelinePbr;
	aux::Pipeline* pAuxPipelineSkybox;
	aux::DescriptorSetLayout* pAuxDSLayoutScene;
	aux::DescriptorSetLayout* pAuxDSLayoutMaterial;
	aux::DescriptorSetLayout* pAuxDSLayoutNode;
	struct DescriptorSets {
		VkDescriptorSet scene;
		VkDescriptorSet skybox;
	};
	std::vector<DescriptorSets> descriptorSets;

	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<UniformBufferSet> uniformBuffers;

	std::vector<VkFence> waitFences;
	std::vector<VkSemaphore> renderCompleteSemaphores;
	std::vector<VkSemaphore> presentCompleteSemaphores;

	const uint32_t renderAhead = 2;
	uint32_t frameIndex = 0;

	int32_t animationIndex = 0;
	float animationTimer = 0.0f;
	bool animate = true;

	bool displayBackground = true;

	struct LightSource {
		glm::vec3 color = glm::vec3(1.0f);
		glm::vec3 rotation = glm::vec3(75.0f, 40.0f, 0.0f);
	} lightSource;

	UI* ui;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	const std::string assetpath = "";
#else
	const std::string assetpath = "./../data/";
#endif

	bool rotateModel = false;
	glm::vec3 modelrot = glm::vec3(0.0f);
	glm::vec3 modelPos = glm::vec3(0.0f);

	enum PBRWorkflows { PBR_WORKFLOW_METALLIC_ROUGHNESS = 0, PBR_WORKFLOW_SPECULAR_GLOSINESS = 1 };

	struct PushConstBlockMaterial {
		glm::vec4 baseColorFactor;
		glm::vec4 emissiveFactor;
		glm::vec4 diffuseFactor;
		glm::vec4 specularFactor;
		float workflow;
		int colorTextureSet;
		int PhysicalDescriptorTextureSet;
		int normalTextureSet;
		int occlusionTextureSet;
		int emissiveTextureSet;
		float metallicFactor;
		float roughnessFactor;
		float alphaMask;
		float alphaMaskCutoff;
	} pushConstBlockMaterial;

	std::map<std::string, std::string> environments;
	std::string selectedEnvironment = "papermill";

#if !defined(_WIN32)
	std::map<std::string, std::string> scenes;
	std::string selectedScene = "DamagedHelmet";
#endif

	int32_t debugViewInputs = 0;
	int32_t debugViewEquation = 0;

	VulkanExample() : VulkanExampleBase()
	{
		title = "Vulkan glTF 2.0 PBR - (C) Sascha Willems (www.saschawillems.de)";
#if defined(TINYGLTF_ENABLE_DRACO)
		std::cout << "Draco mesh compression is enabled" << std::endl;
#endif
	}

	~VulkanExample()
	{
		delete pAuxPipelineBlend;
		delete pAuxPipelinePbr;
		delete pAuxPipelineSkybox;
		delete pAuxPipelineLayout;
		delete pAuxDSLayoutScene;
		delete pAuxDSLayoutMaterial;
		delete pAuxDSLayoutNode;

		models.scene.destroy(device);
		models.skybox.destroy(device);

		for (auto buffer : uniformBuffers) {
			buffer.params.destroy();
			buffer.scene.destroy();
			buffer.skybox.destroy();
		}
		for (auto fence : waitFences) {
			vkDestroyFence(device, fence, nullptr);
		}
		for (auto semaphore : renderCompleteSemaphores) {
			vkDestroySemaphore(device, semaphore, nullptr);
		}
		for (auto semaphore : presentCompleteSemaphores) {
			vkDestroySemaphore(device, semaphore, nullptr);
		}

		textures.environmentCube.destroy();
		textures.irradianceCube.destroy();
		textures.prefilteredCube.destroy();
		textures.lutBrdf.destroy();
		textures.empty.destroy();

		delete ui;
	}

	void renderNode(vkglTF::Node* node, uint32_t cbIndex, vkglTF::Material::AlphaMode alphaMode) {
		if (node->mesh) {
			// Render mesh primitives
			for (vkglTF::Primitive* primitive : node->mesh->primitives) {
				if (primitive->material.alphaMode == alphaMode) {

					const std::vector<VkDescriptorSet> descriptorsets = {
						descriptorSets[cbIndex].scene,
						primitive->material.descriptorSet,
						node->mesh->uniformBuffer.descriptorSet,
					};
					vkCmdBindDescriptorSets(commandBuffers[cbIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, static_cast<uint32_t>(descriptorsets.size()), descriptorsets.data(), 0, NULL);

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

					vkCmdPushConstants(commandBuffers[cbIndex], pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstBlockMaterial), &pushConstBlockMaterial);

					if (primitive->hasIndices) {
						vkCmdDrawIndexed(commandBuffers[cbIndex], primitive->indexCount, 1, primitive->firstIndex, 0, 0);
					}
					else {
						vkCmdDraw(commandBuffers[cbIndex], primitive->vertexCount, 1, 0, 0);
					}
				}
			}

		};
		for (auto child : node->children) {
			renderNode(child, cbIndex, alphaMode);
		}
	}

	void recordCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBufferBeginInfo{};
		cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VkClearValue clearValues[3];
		if (settings.multiSampling) {
			clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
			clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
			clearValues[2].depthStencil = { 1.0f, 0 };
		}
		else {
			clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
			clearValues[1].depthStencil = { 1.0f, 0 };
		}

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = settings.multiSampling ? 3 : 2;
		renderPassBeginInfo.pClearValues = clearValues;

		for (size_t i = 0; i < commandBuffers.size(); ++i) {
			renderPassBeginInfo.framebuffer = frameBuffers[i];

			VkCommandBuffer currentCB = commandBuffers[i];

			VK_CHECK_RESULT(vkBeginCommandBuffer(currentCB, &cmdBufferBeginInfo));
			vkCmdBeginRenderPass(currentCB, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport{};
			viewport.width = (float)width;
			viewport.height = (float)height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(currentCB, 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.extent = { width, height };
			vkCmdSetScissor(currentCB, 0, 1, &scissor);

			VkDeviceSize offsets[1] = { 0 };

			if (displayBackground) {
				vkCmdBindDescriptorSets(currentCB, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i].skybox, 0, nullptr);
				pAuxPipelineSkybox->bindToGraphic(currentCB);
				models.skybox.draw(currentCB);
			}

			pAuxPipelinePbr->bindToGraphic(currentCB);

			vkglTF::Model& model = models.scene;

			vkCmdBindVertexBuffers(currentCB, 0, 1, &model.vertices.buffer, offsets);
			if (model.indices.buffer != VK_NULL_HANDLE) {
				vkCmdBindIndexBuffer(currentCB, model.indices.buffer, 0, VK_INDEX_TYPE_UINT32);
			}

			// Opaque primitives first
			for (auto node : model.nodes) {
				renderNode(node, (uint32_t)i, vkglTF::Material::ALPHAMODE_OPAQUE);
			}
			// Alpha masked primitives
			for (auto node : model.nodes) {
				renderNode(node, (uint32_t)i, vkglTF::Material::ALPHAMODE_MASK);
			}
			// Transparent primitives
			// TODO: Correct depth sorting
			pAuxPipelineBlend->bindToGraphic(currentCB);
			for (auto node : model.nodes) {
				renderNode(node, (uint32_t)i, vkglTF::Material::ALPHAMODE_BLEND);
			}

			// User interface
			ui->draw(currentCB);

			vkCmdEndRenderPass(currentCB);
			VK_CHECK_RESULT(vkEndCommandBuffer(currentCB));
		}
	}

	void loadScene(std::string filename)
	{
		std::cout << "Loading scene from " << filename << std::endl;
		models.scene.destroy(device);
		animationIndex = 0;
		animationTimer = 0.0f;
		models.scene.loadFromFile(filename, vulkanDevice, queue);
		camera.setPosition({ 0.0f, 0.0f, 1.0f });
		camera.setRotation({ 0.0f, 0.0f, 0.0f });
	}

	void loadEnvironment(std::string filename)
	{
		std::cout << "Loading environment from " << filename << std::endl;
		if (textures.environmentCube.image) {
			textures.environmentCube.destroy();
			textures.irradianceCube.destroy();
			textures.prefilteredCube.destroy();
		}
		textures.environmentCube.loadFromFile(filename, VK_FORMAT_R16G16B16A16_SFLOAT, vulkanDevice, queue);
		generateCubemaps();
	}

	void loadAssets()
	{
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
		tinygltf::asset_manager = androidApp->activity->assetManager;
		readDirectory(assetpath + "models", "*.gltf", scenes, true);
#else
		const std::string assetpath = "./../data/";
		struct stat info;
		if (stat(assetpath.c_str(), &info) != 0) {
			std::string msg = "Could not locate asset path in \"" + assetpath + "\".\nMake sure binary is run from correct relative directory!";
			std::cerr << msg << std::endl;
			exit(-1);
		}
#endif
		readDirectory(assetpath + "environments", "*.ktx", environments, false);

		textures.empty.loadFromFile(assetpath + "textures/empty.ktx", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, queue);

		std::string sceneFile = assetpath + "models/DamagedHelmet/glTF-Embedded/DamagedHelmet.gltf";
		std::string envMapFile = assetpath + "environments/papermill.ktx";
		for (size_t i = 0; i < args.size(); i++) {
			if (std::string(args[i]).find(".gltf") != std::string::npos) {
				std::ifstream file(args[i]);
				if (file.good()) {
					sceneFile = args[i];
				}
				else {
					std::cout << "could not load \"" << args[i] << "\"" << std::endl;
				}
			}
			if (std::string(args[i]).find(".ktx") != std::string::npos) {
				std::ifstream file(args[i]);
				if (file.good()) {
					envMapFile = args[i];
				}
				else {
					std::cout << "could not load \"" << args[i] << "\"" << std::endl;
				}
			}
		}

		loadScene(sceneFile.c_str());
		models.skybox.loadFromFile(assetpath + "models/Box/glTF-Embedded/Box.gltf", vulkanDevice, queue);

		loadEnvironment(envMapFile.c_str());
	}

	void setupNodeDescriptorSet(vkglTF::Node* node) {
		if (node->mesh) {
			aux::DescriptorSet::allocate(node->mesh->uniformBuffer.descriptorSet,
			 descriptorPool, pAuxDSLayoutNode->get());
			
			VkWriteDescriptorSet writeDescriptorSet{};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.dstSet = node->mesh->uniformBuffer.descriptorSet;
			writeDescriptorSet.dstBinding = 0;
			writeDescriptorSet.pBufferInfo = &node->mesh->uniformBuffer.descriptor;

			vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
		}
		for (auto& child : node->children) {
			setupNodeDescriptorSet(child);
		}
	}

	void setupDescriptors()
	{
		/*
			Descriptor Pool
		*/
		uint32_t imageSamplerCount = 0;
		uint32_t materialCount = 0;
		uint32_t meshCount = 0;

		// Environment samplers (radiance, irradiance, brdf lut)
		imageSamplerCount += 3;

		std::vector<vkglTF::Model*> modellist = { &models.skybox, &models.scene };
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
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (4 + meshCount) * swapChain.imageCount },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageSamplerCount * swapChain.imageCount }
		};
		uint32_t maxSets = (2 + materialCount + meshCount) * swapChain.imageCount;
		aux::DescriptorPool::create(descriptorPool, poolSizes, maxSets);

		/*
			Descriptor sets
		*/

		// Scene (matrices and environment maps)
		{
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
				{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
				{ 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
				{ 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
				{ 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
				{ 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
			};
			pAuxDSLayoutScene = new aux::DescriptorSetLayout(setLayoutBindings);

			for (auto i = 0; i < descriptorSets.size(); i++) {
				aux::DescriptorSet::allocate(descriptorSets[i].scene,
					descriptorPool, pAuxDSLayoutScene->get());
				
				std::array<VkWriteDescriptorSet, 5> writeDescriptorSets{};
				aux::Describe::buffer(writeDescriptorSets[0], descriptorSets[i].scene, 0, &uniformBuffers[i].scene.descriptor);
				aux::Describe::buffer(writeDescriptorSets[1], descriptorSets[i].scene, 1, &uniformBuffers[i].params.descriptor);
				aux::Describe::image(writeDescriptorSets[2], descriptorSets[i].scene, 2, &textures.irradianceCube.descriptor);
				aux::Describe::image(writeDescriptorSets[3], descriptorSets[i].scene, 3, &textures.prefilteredCube.descriptor);
				aux::Describe::image(writeDescriptorSets[4], descriptorSets[i].scene, 4, &textures.lutBrdf.descriptor);

				vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
			}
		}

		// Material (samplers)
		{
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
				{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
				{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
				{ 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
				{ 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
				{ 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
			};
			pAuxDSLayoutMaterial = new aux::DescriptorSetLayout(setLayoutBindings);

			// Per-Material descriptor sets
			for (auto& material : models.scene.materials) {
				aux::DescriptorSet::allocate(material.descriptorSet, 
					descriptorPool, pAuxDSLayoutMaterial->get());				
				std::vector<VkDescriptorImageInfo> imageDescriptors = {
					textures.empty.descriptor,
					textures.empty.descriptor,
					material.normalTexture ? material.normalTexture->descriptor : textures.empty.descriptor,
					material.occlusionTexture ? material.occlusionTexture->descriptor : textures.empty.descriptor,
					material.emissiveTexture ? material.emissiveTexture->descriptor : textures.empty.descriptor
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

				std::array<VkWriteDescriptorSet, 5> writeDescriptorSets{};
				for (size_t i = 0; i < imageDescriptors.size(); i++) {
					aux::Describe::image(writeDescriptorSets[i], material.descriptorSet, 
						static_cast<uint32_t>(i), &imageDescriptors[i]);
				}

				vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
			}

			// Model node (matrices)
			{
				std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
					{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
				};
				pAuxDSLayoutNode = new aux::DescriptorSetLayout(setLayoutBindings);

				// Per-Node descriptor set
				for (auto& node : models.scene.nodes) {
					setupNodeDescriptorSet(node);
				}
			}

		}

		// Skybox (fixed set)
		for (auto i = 0; i < uniformBuffers.size(); i++) {
			aux::DescriptorSet::allocate(descriptorSets[i].skybox, 
				descriptorPool, pAuxDSLayoutScene->get());

			std::array<VkWriteDescriptorSet, 3> writeDescriptorSets{};
			aux::Describe::buffer(writeDescriptorSets[0], descriptorSets[i].skybox, 0, &uniformBuffers[i].skybox.descriptor);
			aux::Describe::buffer(writeDescriptorSets[1], descriptorSets[i].skybox, 1, &uniformBuffers[i].params.descriptor);
			aux::Describe::image(writeDescriptorSets[2], descriptorSets[i].skybox, 2, &textures.prefilteredCube.descriptor);
			
			vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
		}
	}

	void preparePipelines()
	{
		aux::Pipeline::setCache(&pipelineCache);
		aux::PipelineCI auxPipelineCI{};
		auxPipelineCI.cullMode = VK_CULL_MODE_BACK_BIT;
		if (settings.multiSampling) {
			auxPipelineCI.rasterizationSamples = settings.sampleCount;
		}
		else {
			Assert(0, "which value?");
		}
/////

		// Pipeline layout
		const std::vector<VkDescriptorSetLayout> setLayouts = {
			*(pAuxDSLayoutScene->get()), *(pAuxDSLayoutMaterial->get()), *(pAuxDSLayoutNode->get())
		};

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.size = sizeof(PushConstBlockMaterial);
		pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		aux::PipelineLayoutCI auxPipelineLayoutCI{ &pushConstantRange };
		auxPipelineLayoutCI.pSetLayouts = &setLayouts;

		pAuxPipelineLayout = new aux::PipelineLayout(auxPipelineLayoutCI);
		pipelineLayout = pAuxPipelineLayout->get();
		
		// Vertex bindings an attributes
		std::vector<VkVertexInputBindingDescription> vertexInputBindings = { 
			{0, sizeof(vkglTF::Model::Vertex), VK_VERTEX_INPUT_RATE_VERTEX}
		};

		std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
			{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
			{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3 },
			{ 2, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 6 },
			{ 3, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 8 },
			{ 4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 10 },
			{ 5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 14 }
		};

		auxPipelineCI.pVertexInputBindings = &vertexInputBindings;
		auxPipelineCI.pVertexInputAttributes = &vertexInputAttributes;

		std::vector<aux::ShaderDescription> shadersSkybox = {
			{"skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
			{"skybox.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT}
		};
		auxPipelineCI.shaders = shadersSkybox;
		pAuxPipelineSkybox = new aux::Pipeline(*pAuxPipelineLayout, renderPass, auxPipelineCI);
		
		// PBR pipeline
		std::vector<aux::ShaderDescription> shadersPbr = {
			{"pbr.vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
			{"pbr_khr.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT}
		};

		auxPipelineCI.shaders = shadersPbr;
		auxPipelineCI.depthWriteEnable = VK_TRUE;
		auxPipelineCI.depthTestEnable = VK_TRUE;	
		pAuxPipelinePbr = new aux::Pipeline(*pAuxPipelineLayout, renderPass, auxPipelineCI);

		auxPipelineCI.cullMode = VK_CULL_MODE_NONE;
		auxPipelineCI.blendEnable = VK_TRUE;
		auxPipelineCI.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		auxPipelineCI.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		auxPipelineCI.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		auxPipelineCI.colorBlendOp = VK_BLEND_OP_ADD;
		auxPipelineCI.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		auxPipelineCI.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		auxPipelineCI.alphaBlendOp = VK_BLEND_OP_ADD;
		pAuxPipelineBlend = new aux::Pipeline(*pAuxPipelineLayout, renderPass, auxPipelineCI);
	}

	/*
		Generate a BRDF integration map storing roughness/NdotV as a look-up-table
	*/
	void generateBRDFLUT()
	{
		auto tStart = std::chrono::high_resolution_clock::now();

		const VkFormat format = VK_FORMAT_R16G16_SFLOAT;
		const int32_t dim = 512;

		aux::ImageCI auxImageCI(format, dim, dim);
		aux::Image auxImage(auxImageCI);
		textures.lutBrdf.image = auxImage.getImage();
		textures.lutBrdf.deviceMemory = auxImage.getDeviceMemory();
		textures.lutBrdf.view = auxImage.getView();
		textures.lutBrdf.sampler = auxImage.getSampler();

		aux::RenderPass auxRenderPass(auxImage);

		VkRenderPass renderpass = *(auxRenderPass.get());
		aux::Framebuffer auxFramebuffer(auxImage, auxRenderPass);
		aux::PipelineLayoutCI auxPlCi{};
		aux::PipelineLayout auxPipelineLayout(auxPlCi);
		VkPipelineLayout pipelinelayout = auxPipelineLayout.get();

		aux::Pipeline::setCache(&pipelineCache);
		aux::PipelineCI auxPipelineCI{};
		std::vector<aux::ShaderDescription> shaders = {
			{"genbrdflut.vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
			{"genbrdflut.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT}
		};
		auxPipelineCI.shaders = shaders;
		aux::Pipeline auxPipeline(auxPipelineLayout, *auxRenderPass.get(), auxPipelineCI);

		// Render
		VkCommandBuffer cmdBuf = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		auxRenderPass.begin(&cmdBuf, &auxFramebuffer);
		auxPipeline.bindToGraphic(cmdBuf, dim, dim);
		vkCmdDraw(cmdBuf, 3, 1, 0, 0);
		vkCmdEndRenderPass(cmdBuf);
		vulkanDevice->flushCommandBuffer(cmdBuf, queue);

		vkQueueWaitIdle(queue);

		textures.lutBrdf.descriptor.imageView = textures.lutBrdf.view;
		textures.lutBrdf.descriptor.sampler = textures.lutBrdf.sampler;
		textures.lutBrdf.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		textures.lutBrdf.device = vulkanDevice;

		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
		std::cout << "Generating BRDF LUT took " << tDiff << " ms" << std::endl;
	}

	/*
		Offline generation for the cube maps used for PBR lighting
		- Irradiance cube map
		- Pre-filterd environment cubemap
	*/
	void generateCubemaps()
	{
		enum Target { IRRADIANCE = 0, PREFILTEREDENV = 1 };

		for (uint32_t target = 0; target < PREFILTEREDENV + 1; target++)
		{

			vks::TextureCubeMap cubemap;

			auto tStart = std::chrono::high_resolution_clock::now();

			VkFormat format;
			int32_t dim;

			switch (target) {
			case IRRADIANCE:
				format = VK_FORMAT_R32G32B32A32_SFLOAT;
				dim = 64;
				break;
			case PREFILTEREDENV:
				format = VK_FORMAT_R16G16B16A16_SFLOAT;
				dim = 512;
				break;
			};

			// Create target cubemap
			const uint32_t numMips = static_cast<uint32_t>(floor(log2(dim))) + 1;
			aux::ImageCI auxImageCI(format, dim, dim, numMips, 6);
			auxImageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			auxImageCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
			auxImageCI.isCubemap = true;
			aux::Image auxCube(auxImageCI);
			cubemap.image = auxCube.getImage();
			cubemap.deviceMemory = auxCube.getDeviceMemory();
			cubemap.view = auxCube.getView();
			cubemap.sampler = auxCube.getSampler();

			aux::AttachmentDescription auxAttDesc(auxCube);
			auxAttDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			aux::SubpassDescription auxSubpassDescription(auxCube);
			aux::RenderPass auxRenderPass(auxCube);

			// Create offscreen framebuffer
			aux::ImageCI imageCI(format, dim, dim);
			imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			aux::Image auxImageOffscreen(imageCI);

			// no sampler, auxImage.getSampler();

			aux::Framebuffer auxFramebufferOffscreen(auxImageOffscreen, auxRenderPass);
			aux::IMBarrier::toColorAttachment(auxImageOffscreen, queue);
		
			struct PushBlockIrradiance {
				glm::mat4 mvp;
				float deltaPhi = (2.0f * float(M_PI)) / 180.0f;
				float deltaTheta = (0.5f * float(M_PI)) / 64.0f;
			} pushBlockIrradiance;

			struct PushBlockPrefilterEnv {
				glm::mat4 mvp;
				float roughness;
				uint32_t numSamples = 32u;
			} pushBlockPrefilterEnv;

			// Pipeline layout
			VkPushConstantRange pushConstantRange{};
			pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

			switch (target) {
			case IRRADIANCE:
				pushConstantRange.size = sizeof(PushBlockIrradiance);
				break;
			case PREFILTEREDENV:
				pushConstantRange.size = sizeof(PushBlockPrefilterEnv);
				break;
			};


			// Descriptors
			VkDescriptorSetLayoutBinding setLayoutBinding = { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr };

			aux::PipelineLayoutCI auxPipelineLayoutCI{};
			auxPipelineLayoutCI.pDslBindings = &setLayoutBinding;
			auxPipelineLayoutCI.pImageInfo = &textures.environmentCube.descriptor;
			// auxPipelineLayoutCI.pSetLayouts = &descriptorsetlayout;
			auxPipelineLayoutCI.pPushConstantRanges = &pushConstantRange;
			
			aux::PipelineLayout auxPipelineLayout(auxPipelineLayoutCI);
			VkPipelineLayout pipelinelayout = auxPipelineLayout.get();

			aux::Pipeline::setCache(&pipelineCache);
			aux::PipelineCI auxPipelineCI{};
			std::vector<aux::ShaderDescription> shaders = {
				{"filtercube.vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
			};
			
			switch (target) {
			case IRRADIANCE:
				shaders.push_back({ "irradiancecube.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT });
				break;
			case PREFILTEREDENV:
				shaders.push_back({ "prefilterenvmap.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT });
				break;
			};

			auxPipelineCI.shaders = shaders;


			// Vertex input state
			std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
				{ 0, sizeof(vkglTF::Model::Vertex), VK_VERTEX_INPUT_RATE_VERTEX }
			};
			std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
				{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 }
			};
			auxPipelineCI.pVertexInputBindings = &vertexInputBindings;
			auxPipelineCI.pVertexInputAttributes = &vertexInputAttributes;
			aux::Pipeline auxPipeline(auxPipelineLayout, *auxRenderPass.get(), auxPipelineCI);

			std::vector<glm::mat4> matrices = {
				glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
				glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
				glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
				glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
				glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
				glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			};

			VkCommandBuffer cmdBuf = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, false);
			
			aux::IMBarrier::convertLayoutToTransfer(auxCube, cmdBuf, queue);

			for (uint32_t m = 0; m < numMips; m++) {
				for (uint32_t f = 0; f < 6; f++) {

					vulkanDevice->beginCommandBuffer(cmdBuf);

					// Render scene from cube face's point of view
					auxRenderPass.begin(&cmdBuf, &auxFramebufferOffscreen, { 0.0f, 0.0f, 0.2f, 0.0f });
					// Pass parameters for current pass using a push constant block
					switch (target) {
					case IRRADIANCE:
						pushBlockIrradiance.mvp = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];
						vkCmdPushConstants(cmdBuf, pipelinelayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlockIrradiance), &pushBlockIrradiance);
						break;
					case PREFILTEREDENV:
						pushBlockPrefilterEnv.mvp = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];
						pushBlockPrefilterEnv.roughness = (float)m / (float)(numMips - 1);
						vkCmdPushConstants(cmdBuf, pipelinelayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlockPrefilterEnv), &pushBlockPrefilterEnv);
						break;
					};

					uint32_t vpDim = static_cast<uint32_t>(dim * std::pow(0.5f, m));
					auxPipeline.bindToGraphic(cmdBuf, dim, dim, vpDim, vpDim);
					vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinelayout, 0, 1, (auxPipelineLayout.getDSet()->get()), 0, NULL);

					VkDeviceSize offsets[1] = { 0 };

					models.skybox.draw(cmdBuf);

					vkCmdEndRenderPass(cmdBuf);

					aux::IMBarrier::colorAttachment2Transfer(auxImageOffscreen, cmdBuf);
					VkExtent3D region;
					region.height = vpDim;
					region.width = vpDim;
					aux::Image::copyOneMip2Cube(cmdBuf, auxImageOffscreen, region, auxCube, f, m);
					aux::IMBarrier::transfer2ColorAttachment(auxImageOffscreen, cmdBuf);
					vulkanDevice->flushCommandBuffer(cmdBuf, queue, false);
				}
			}

			{
				vulkanDevice->beginCommandBuffer(cmdBuf);
				aux::IMBarrier::transfer2ShaderRead(auxCube, cmdBuf);
				vulkanDevice->flushCommandBuffer(cmdBuf, queue, false);
			}

			cubemap.descriptor.imageView = cubemap.view;
			cubemap.descriptor.sampler = cubemap.sampler;
			cubemap.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			cubemap.device = vulkanDevice;

			switch (target) {
			case IRRADIANCE:
				textures.irradianceCube = cubemap;
				break;
			case PREFILTEREDENV:
				textures.prefilteredCube = cubemap;
				shaderValuesParams.prefilteredCubeMipLevels = static_cast<float>(numMips);
				break;
			};

			auto tEnd = std::chrono::high_resolution_clock::now();
			auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
			std::cout << "Generating cube map with " << numMips << " mip levels took " << tDiff << " ms" << std::endl;
		}
	}

	/*
		Prepare and initialize uniform buffers containing shader parameters
	*/
	void prepareUniformBuffers()
	{
		for (auto& uniformBuffer : uniformBuffers) {
			uniformBuffer.scene.create(vulkanDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(shaderValuesScene));
			uniformBuffer.skybox.create(vulkanDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(shaderValuesSkybox));
			uniformBuffer.params.create(vulkanDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(shaderValuesParams));
		}
		updateUniformBuffers();
	}

	void updateUniformBuffers()
	{
		// Scene
		shaderValuesScene.projection = camera.matrices.perspective;
		shaderValuesScene.view = camera.matrices.view;

		// Center and scale model
		float scale = (1.0f / std::max(models.scene.aabb[0][0], std::max(models.scene.aabb[1][1], models.scene.aabb[2][2]))) * 0.5f;
		glm::vec3 translate = -glm::vec3(models.scene.aabb[3][0], models.scene.aabb[3][1], models.scene.aabb[3][2]);
		translate += -0.5f * glm::vec3(models.scene.aabb[0][0], models.scene.aabb[1][1], models.scene.aabb[2][2]);

		shaderValuesScene.model = glm::mat4(1.0f);
		shaderValuesScene.model[0][0] = scale;
		shaderValuesScene.model[1][1] = scale;
		shaderValuesScene.model[2][2] = scale;
		shaderValuesScene.model = glm::translate(shaderValuesScene.model, translate);

		shaderValuesScene.camPos = glm::vec3(
			-camera.position.z * sin(glm::radians(camera.rotation.y)) * cos(glm::radians(camera.rotation.x)),
			-camera.position.z * sin(glm::radians(camera.rotation.x)),
			camera.position.z * cos(glm::radians(camera.rotation.y)) * cos(glm::radians(camera.rotation.x))
		);

		// Skybox
		shaderValuesSkybox.projection = camera.matrices.perspective;
		shaderValuesSkybox.view = camera.matrices.view;
		shaderValuesSkybox.model = glm::mat4(glm::mat3(camera.matrices.view));
	}

	void updateParams()
	{
		shaderValuesParams.lightDir = glm::vec4(
			sin(glm::radians(lightSource.rotation.x)) * cos(glm::radians(lightSource.rotation.y)),
			sin(glm::radians(lightSource.rotation.y)),
			cos(glm::radians(lightSource.rotation.x)) * cos(glm::radians(lightSource.rotation.y)),
			0.0f);
	}

	void windowResized()
	{
		recordCommandBuffers();
		vkDeviceWaitIdle(device);
		updateUniformBuffers();
		updateOverlay();
	}

	void prepare()
	{
		VulkanExampleBase::prepare();

		aux::Device::setVksDevice(vulkanDevice);
		aux::Device::set(&device);

		camera.type = Camera::CameraType::lookat;

		camera.setPerspective(45.0f, (float)width / (float)height, 0.1f, 256.0f);
		camera.rotationSpeed = 0.25f;
		camera.movementSpeed = 0.1f;
		camera.setPosition({ 0.0f, 0.0f, 1.0f });
		camera.setRotation({ 0.0f, 0.0f, 0.0f });

		waitFences.resize(renderAhead);
		presentCompleteSemaphores.resize(renderAhead);
		renderCompleteSemaphores.resize(renderAhead);
		commandBuffers.resize(swapChain.imageCount);
		uniformBuffers.resize(swapChain.imageCount);
		descriptorSets.resize(swapChain.imageCount);
		// Command buffer execution fences
		for (auto& waitFence : waitFences) {
			VkFenceCreateInfo fenceCI{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT };
			VK_CHECK_RESULT(vkCreateFence(device, &fenceCI, nullptr, &waitFence));
		}
		// Queue ordering semaphores
		for (auto& semaphore : presentCompleteSemaphores) {
			VkSemaphoreCreateInfo semaphoreCI{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0 };
			VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCI, nullptr, &semaphore));
		}
		for (auto& semaphore : renderCompleteSemaphores) {
			VkSemaphoreCreateInfo semaphoreCI{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0 };
			VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCI, nullptr, &semaphore));
		}
		// Command buffers
		{
			VkCommandBufferAllocateInfo cmdBufAllocateInfo{};
			cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdBufAllocateInfo.commandPool = cmdPool;
			cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufAllocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
			VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, commandBuffers.data()));
		}

		loadAssets();
		generateBRDFLUT();
		generateCubemaps();
		prepareUniformBuffers();
		setupDescriptors();
		preparePipelines();

		ui = new UI(vulkanDevice, renderPass, queue, pipelineCache, settings.sampleCount);
		updateOverlay();

		recordCommandBuffers();

		prepared = true;
	}

	void updateOverlay();

	virtual void render()
	{
		if (!prepared) {
			return;
		}

		updateOverlay();

		VK_CHECK_RESULT(vkWaitForFences(device, 1, &waitFences[frameIndex], VK_TRUE, UINT64_MAX));
		VK_CHECK_RESULT(vkResetFences(device, 1, &waitFences[frameIndex]));

		VkResult acquire = swapChain.acquireNextImage(presentCompleteSemaphores[frameIndex], &currentBuffer);
		if ((acquire == VK_ERROR_OUT_OF_DATE_KHR) || (acquire == VK_SUBOPTIMAL_KHR)) {
			windowResize();
		}
		else {
			VK_CHECK_RESULT(acquire);
		}

		// Update UBOs
		updateUniformBuffers();
		UniformBufferSet currentUB = uniformBuffers[currentBuffer];
		memcpy(currentUB.scene.mapped, &shaderValuesScene, sizeof(shaderValuesScene));
		memcpy(currentUB.params.mapped, &shaderValuesParams, sizeof(shaderValuesParams));
		memcpy(currentUB.skybox.mapped, &shaderValuesSkybox, sizeof(shaderValuesSkybox));

		const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitDstStageMask = &waitDstStageMask;
		submitInfo.pWaitSemaphores = &presentCompleteSemaphores[frameIndex];
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &renderCompleteSemaphores[frameIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[currentBuffer];
		submitInfo.commandBufferCount = 1;
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, waitFences[frameIndex]));

		VkResult present = swapChain.queuePresent(queue, currentBuffer, renderCompleteSemaphores[frameIndex]);
		if (!((present == VK_SUCCESS) || (present == VK_SUBOPTIMAL_KHR))) {
			if (present == VK_ERROR_OUT_OF_DATE_KHR) {
				windowResize();
				return;
			}
			else {
				VK_CHECK_RESULT(present);
			}
		}

		frameIndex += 1;
		frameIndex %= renderAhead;

		if (!paused) {
			if (rotateModel) {
				modelrot.y += frameTimer * 35.0f;
				if (modelrot.y > 360.0f) {
					modelrot.y -= 360.0f;
				}
			}
			if ((animate) && (models.scene.animations.size() > 0)) {
				animationTimer += frameTimer;
				if (animationTimer > models.scene.animations[animationIndex].end) {
					animationTimer -= models.scene.animations[animationIndex].end;
				}
				models.scene.updateAnimation(animationIndex, animationTimer);
			}
			updateParams();
			if (rotateModel) {
				updateUniformBuffers();
			}
		}
		if (camera.updated) {
			updateUniformBuffers();
		}
	}
};

VulkanExample* vulkanExample;
#include "osMain.cpp"
#include "mainUI.cpp"