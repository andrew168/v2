#include "auxVk/auxVk.h"
#include "gltf/gltf.h"
#include "pbr/pbr.h"
#include "v2/v2.h"
#include "main.h"
using namespace aux;
using namespace pbr;
using namespace gltf;
VulkanExample::VulkanExample() : VulkanExampleBase()
{
	title = "V2--Vulkan library from beginner to professinonal";
#if defined(TINYGLTF_ENABLE_DRACO)
	std::cout << "Draco mesh compression is enabled" << std::endl;
#endif
}

VulkanExample::~VulkanExample()
{
	delete ui;
}

void VulkanExample::recordCommandBuffers()
{
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
	VkCommandBufferBeginInfo cmdBufferBeginInfo{};
	RenderPass auxRenderPass(renderPass);
	VkRenderPassBeginInfo renderPassBeginInfo{};
	auxRenderPass.fillBI(renderPassBeginInfo, width, height, settings.multiSampling ? 3 : 2, clearValues);
	for (size_t i = 0; i < commandBuffers.size(); ++i) {
		renderPassBeginInfo.framebuffer = frameBuffers[i];

		VkCommandBuffer currentCB = commandBuffers[i];
		CommandBuffer auxCmdBuf(currentCB);
		auxCmdBuf.fillBI(cmdBufferBeginInfo);
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

void VulkanExample::loadScene(std::string filename)
{
	std::cout << "Loading scene from " << filename << std::endl;
	sceneModel.destroy(device);
	sceneModel.loadFromFile(filename, vulkanDevice, queue);
	camera.setPosition({ 0.0f, 0.0f, 1.0f });
	camera.setRotation({ 0.0f, 0.0f, 0.0f });
}

void VulkanExample::loadEnvironment(std::string filename)
{
	std::cout << "Loading environment from " << filename << std::endl;
	pbr1.setEnvMap(filename);
}

void VulkanExample::loadAssets()
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
	pbr1.setEmptyMap(assetpath + "textures/empty.ktx");		
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

void VulkanExample::updateLights()
{
	pbr1.shaderParams.lightDir = glm::vec4(
		sin(glm::radians(lightSource.rotation.x)) * cos(glm::radians(lightSource.rotation.y)),
		sin(glm::radians(lightSource.rotation.y)),
		cos(glm::radians(lightSource.rotation.x)) * cos(glm::radians(lightSource.rotation.y)),
		0.0f);
}

void VulkanExample::windowResized()
{
	recordCommandBuffers();
	vkDeviceWaitIdle(device);
	pbr1.updateShaderValues();
	updateOverlay();
}

void VulkanExample::prepare()
{
	VulkanExampleBase::prepare();
	Device::set(&device, &queue, vulkanDevice);
	Pipeline::setCache(&pipelineCache);

	camera.type = Camera::CameraType::lookat;

	camera.setPerspective(45.0f, (float)width / (float)height, 0.1f, 256.0f);
	camera.rotationSpeed = 0.25f;
	camera.movementSpeed = 0.1f;
	camera.setPosition({ 0.0f, 0.0f, 1.0f });
	camera.setRotation({ 0.0f, 0.0f, 0.0f });

	fenceMgr.create(renderAhead);
	presentSemaphoreMgr.create(renderAhead);
	renderSemaphoreMgr.create(renderAhead);
	commandBuffers.resize(swapChain.imageCount);
	pbr1.config(sceneModel, skyboxModel);
	pbr1.init(swapChain.imageCount, camera, renderPass);
	CommandBuffer::allocate(cmdPool, commandBuffers);
	loadAssets();
	pbr1.generateBRDFLUT();
	pbr1.generateCubemaps(skyboxModel);
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

void VulkanExample::render()
{
	if (!prepared) {
		return;
	}

	updateOverlay();

	// 确保以前此ID提交的任务没有完成，则需要等待，然后reset为unsignaled
	fenceMgr.wait(frameIndex);
	fenceMgr.reset(frameIndex);
	VkResult acquire = swapChain.acquireNextImage(presentSemaphoreMgr.getR(frameIndex), &currentBuffer);
	if ((acquire == VK_ERROR_OUT_OF_DATE_KHR) || (acquire == VK_SUBOPTIMAL_KHR)) {
		windowResize();
	}
	else {
		VK_CHECK_RESULT(acquire);
	}

	// Update UBOs
	pbr1.updateShaderValues();
	pbr1.applyShaderValues(currentBuffer);
	sceneModel.applyShaderValues(currentBuffer);
	skyboxModel.applyShaderValues(currentBuffer);

	Queue auxQueue(queue);
	// 在Color Attachment这个Stage:
	// 等到present完成信号出现才执行本cmd
	// 完成本cmd之后发出render完成信号：
	// 加fence，以确保本任务能够完成。
	auxQueue.submit(std::vector<VkCommandBuffer>({ commandBuffers[currentBuffer] }),
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		std::vector<VkSemaphore>({ presentSemaphoreMgr.getR(frameIndex) }),
		std::vector<VkSemaphore>({ renderSemaphoreMgr.getR(frameIndex)}),
		fenceMgr.get(frameIndex));

	// 等到render完成信号出现之后，才执行本任务，i.e. present
	VkResult present = swapChain.queuePresent(queue, currentBuffer, renderSemaphoreMgr.getR(frameIndex));
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

void VulkanExample::update()
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
