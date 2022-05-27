#include "v2\v2.h"
#include "auxVk\auxVk.h"
#include "pbr/brdflut.h"
#include "gltf/gltf.h"

using namespace aux;
using namespace v2;
using namespace gltf;
/*
	PBR example main class
*/
class VulkanExample : public VulkanExampleBase
{
public:
	gltf::Models models;
	struct Textures {
		vks::TextureCubeMap environmentCube;
		vks::Texture2D empty;
		vks::Texture2D lutBrdf;
		vks::TextureCubeMap irradianceCube;
		vks::TextureCubeMap prefilteredCube;
	} textures;

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

	PushConstBlockMaterial pushConstBlockMaterial;
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

		destroyCubemaps();
		textures.lutBrdf.destroy();
		textures.empty.destroy();

		delete ui;
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

		// 1个RenderPass， 1个BeginInfo，被多个CmdBuf使用
		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = settings.multiSampling ? 3 : 2;
		renderPassBeginInfo.pClearValues = clearValues;

		aux::RenderPass auxRenderPass(renderPass);
		for (size_t i = 0; i < commandBuffers.size(); ++i) {
			renderPassBeginInfo.framebuffer = frameBuffers[i];

			VkCommandBuffer currentCB = commandBuffers[i];
			aux::CommandBuffer auxCmdBuf(currentCB);
			auxCmdBuf.begin(&cmdBufferBeginInfo);
			auxRenderPass.begin(currentCB, renderPassBeginInfo);
			auxCmdBuf.setViewport(width, height);
			auxCmdBuf.setScissor(width, height);

			if (displayBackground) {
				//skybox绘制： 先绑定 ds和pipeline，再绘制
				gltf::Render render(descriptorSets[i].skybox, currentCB,
					pipelineLayout, *pAuxPipelineSkybox);
				render.draw(models.skybox);
			}

			gltf::Render render(descriptorSets[i].scene, currentCB,
				pipelineLayout, *pAuxPipelinePbr, pAuxPipelineBlend);
			render.drawT(models.scene);

			// User interface
			ui->draw(currentCB);

			auxRenderPass.end();
			auxCmdBuf.end();
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

	void destroyCubemaps() {
		textures.environmentCube.destroy();
		textures.irradianceCube.destroy();
		textures.prefilteredCube.destroy();
	}

	void loadEnvironment(std::string filename)
	{
		std::cout << "Loading environment from " << filename << std::endl;
		if (textures.environmentCube.image) {
			destroyCubemaps();
		}
		textures.environmentCube.loadFromFile(filename, VK_FORMAT_R16G16B16A16_SFLOAT, vulkanDevice, queue);
		// generateCubemaps();
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
			aux::Describe::bufferUpdate(node->mesh->uniformBuffer.descriptorSet,
				0, &node->mesh->uniformBuffer.descriptor);
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

		// 每一个GLTF模型都有自己的材质列表和mesh列表
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
				
				std::vector<VkWriteDescriptorSet> writeDescriptorSets(5);
				aux::Describe::buffer(writeDescriptorSets[0], descriptorSets[i].scene, 0, &uniformBuffers[i].scene.descriptor);
				aux::Describe::buffer(writeDescriptorSets[1], descriptorSets[i].scene, 1, &uniformBuffers[i].params.descriptor);
				aux::Describe::image(writeDescriptorSets[2], descriptorSets[i].scene, 2, &textures.irradianceCube.descriptor);
				aux::Describe::image(writeDescriptorSets[3], descriptorSets[i].scene, 3, &textures.prefilteredCube.descriptor);
				aux::Describe::image(writeDescriptorSets[4], descriptorSets[i].scene, 4, &textures.lutBrdf.descriptor);

				aux::DescriptorSet::updateW(writeDescriptorSets);
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

				std::vector<VkWriteDescriptorSet> writeDescriptorSets(5);
				for (size_t i = 0; i < imageDescriptors.size(); i++) {
					aux::Describe::image(writeDescriptorSets[i], material.descriptorSet, 
						static_cast<uint32_t>(i), &imageDescriptors[i]);
				}

				DescriptorSet::updateW(writeDescriptorSets);
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

			std::vector<VkWriteDescriptorSet> writeDescriptorSets(3);
			aux::Describe::buffer(writeDescriptorSets[0], descriptorSets[i].skybox, 0, &uniformBuffers[i].skybox.descriptor);
			aux::Describe::buffer(writeDescriptorSets[1], descriptorSets[i].skybox, 1, &uniformBuffers[i].params.descriptor);
			aux::Describe::image(writeDescriptorSets[2], descriptorSets[i].skybox, 2, &textures.prefilteredCube.descriptor);			
			DescriptorSet::updateW(writeDescriptorSets);
		}
	}

	void preparePipelines()
	{
		aux::PipelineCI auxPipelineCI{};
		auxPipelineCI.cullMode = VK_CULL_MODE_BACK_BIT;
		if (settings.multiSampling) {
			auxPipelineCI.rasterizationSamples = settings.sampleCount;
		}
		else {
			Assert(0, "which value?");
		}

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
		aux::Device::set(&device, &queue, vulkanDevice);
		aux::Pipeline::setCache(&pipelineCache);

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
			aux::Fence::create(waitFence, true);
		}
		// Queue ordering semaphores
		for (auto& semaphore : presentCompleteSemaphores) {
			aux::Semaphore::create(semaphore);
		}
		for (auto& semaphore : renderCompleteSemaphores) {
			aux::Semaphore::create(semaphore);
		}
		aux::CommandBuffer::allocate(cmdPool, commandBuffers);		
		loadAssets();
		Pbr::generateBRDFLUT().toVKS(textures.lutBrdf);
		std::vector<aux::Image> auxCubemaps{};
		v2::Pbr::generateCubemaps(auxCubemaps, models, textures.environmentCube);
		auxCubemaps[0].toVKS(textures.irradianceCube);
		auxCubemaps[1].toVKS(textures.prefilteredCube);
		shaderValuesParams.prefilteredCubeMipLevels = 
			static_cast<float>(auxCubemaps[1].getMipLevels());
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

		aux::Fence::wait(waitFences[frameIndex]);
		aux::Fence::reset(waitFences[frameIndex]);

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

		aux::Queue auxQueue(queue);
		auxQueue.submit(std::vector<VkCommandBuffer>({ commandBuffers[currentBuffer] }),
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			std::vector<VkSemaphore>({ presentCompleteSemaphores[frameIndex] }),
			std::vector<VkSemaphore>({ renderCompleteSemaphores[frameIndex] }),
			waitFences[frameIndex]);

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
		update();
	}
	
	void update()
	{
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