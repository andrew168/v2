#include "auxVk/auxVk.h"
#include "gltf/gltf.h"
#include "pbr/pbr.h"
#include "v2/v2.h"

using namespace aux;
using namespace pbr;
using namespace gltf;
/*
	PBR example main class
*/
class VulkanExample : public VulkanExampleBase
{
public:
	Pbr pbr1;
	gltf::Model sceneModel;
	gltf::Skybox skyboxModel;
	Textures textures;
	// VkPipelineLayout pipelineLayout;

	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkFence> waitFences;
	std::vector<VkSemaphore> renderCompleteSemaphores;
	std::vector<VkSemaphore> presentCompleteSemaphores;

	const uint32_t renderAhead = 2;
	uint32_t frameIndex = 0;
	int32_t animationIndex = 0;
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
		title = "V2--Vulkan library from beginner to professinonal";
#if defined(TINYGLTF_ENABLE_DRACO)
		std::cout << "Draco mesh compression is enabled" << std::endl;
#endif
	}

	~VulkanExample()
	{		
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
		aux::RenderPass auxRenderPass(renderPass);
		VkRenderPassBeginInfo renderPassBeginInfo{};
		auxRenderPass.fillBI(renderPassBeginInfo, width, height, settings.multiSampling ? 3 : 2, clearValues);
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
				
				skyboxModel.draw(skyboxModel, currentCB, static_cast<uint32_t>(i), pbr1);
			}
			sceneModel.drawT(sceneModel, currentCB, static_cast<uint32_t>(i), pbr1);

			// User interface
			ui->draw(currentCB);

			auxRenderPass.end();
			auxCmdBuf.end();
		}
	}

	void loadScene(std::string filename)
	{
		std::cout << "Loading scene from " << filename << std::endl;
		sceneModel.destroy(device);
		sceneModel.loadFromFile(filename, vulkanDevice, queue);
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
		skyboxModel.loadFromFile(assetpath + "models/Box/glTF-Embedded/Box.gltf", vulkanDevice, queue);

		loadEnvironment(envMapFile.c_str());
	}

	void updateLights()
	{
		pbr1.shaderValuesParams.lightDir = glm::vec4(
			sin(glm::radians(lightSource.rotation.x)) * cos(glm::radians(lightSource.rotation.y)),
			sin(glm::radians(lightSource.rotation.y)),
			cos(glm::radians(lightSource.rotation.x)) * cos(glm::radians(lightSource.rotation.y)),
			0.0f);
	}

	void windowResized()
	{
		recordCommandBuffers();
		vkDeviceWaitIdle(device);
		pbr1.updateShaderValues();
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
		pbr1.config(sceneModel, skyboxModel, textures);
		pbr1.init(swapChain.imageCount, camera, renderPass);

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
		Pbr::generateCubemaps(auxCubemaps, skyboxModel, textures.environmentCube);
		auxCubemaps[0].toVKS(textures.irradianceCube);
		auxCubemaps[1].toVKS(textures.prefilteredCube);
		pbr1.shaderValuesParams.prefilteredCubeMipLevels = 
			static_cast<float>(auxCubemaps[1].getMipLevels());
			static_cast<float>(auxCubemaps[1].getMipLevels());
		pbr1.createUB();
		pbr1.updateDS(descriptorPool);
		PbrConfig pbrConfig;
		pbrConfig.multiSampling = settings.multiSampling;
		pbrConfig.sampleCount = settings.sampleCount;
		pbr1.createPipeline(pbrConfig);
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
		pbr1.updateShaderValues();
		sceneModel.applyShaderValues(currentBuffer);
		pbr1.applyShaderValues(currentBuffer);
		skyboxModel.applyShaderValues(currentBuffer);
		
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
			if (animate)
			{
				sceneModel.update(animationIndex, frameTimer);
			}
			updateLights();
			if (rotateModel) {
				pbr1.updateShaderValues();
			}
		}
		if (camera.updated) {
			pbr1.updateShaderValues();
		}
	}
};

VulkanExample* vulkanExample;
#include "osMain.cpp"
#include "mainUI.cpp"