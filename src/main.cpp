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
	title = "V2-- make Vulkan faster, easier";
}

VulkanExample::~VulkanExample()
{
	delete ui;
}

void VulkanExample::recordCommandBuffers()
{
	// Clear Value数组为BI所用，
	// 在auxRenderPass Begin之前，它必须存在，否则会出错
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
	const std::string assetpath = "./../data/";
	struct stat info;
	if (stat(assetpath.c_str(), &info) != 0) {
		std::string msg = "Could not locate asset path in \"" + assetpath + "\".\nMake sure binary is run from correct relative directory!";
		std::cerr << msg << std::endl;
		exit(-1);
	}

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
	
	// fence和semaphore有2套替换使用
	// fence有两套替换: （不是3套）：因为1套彻底结束之后，2套执行，3套才能开始写，所以1套可以被3重复使用。
	fenceMgr.create(renderAhead);
	presentSemaphoreMgr.create(renderAhead);
	renderSemaphoreMgr.create(renderAhead);
	
	//CmdBuf, FrameBuffers, UniformBuffers, DS, 3套，与swapChain.imageCount一样多
	//1 - 在执行，2 - 刚刚提交等待执行；3等待1完成才开始写; 确保GPU有任务
	//必须至少3套，因为提交到执行需要时间上传，执行|上传|写任务 三者并行
	commandBuffers.resize(swapChain.imageCount);
	CommandBuffer::allocate(cmdPool, commandBuffers);
	loadAssets();

	PbrConfig pbrConfig;
	pbrConfig.multiSampling = settings.multiSampling;
	pbrConfig.sampleCount = settings.sampleCount;
	pbr1.config(sceneModel, skyboxModel);
	pbr1.init(pbrConfig, descriptorPool, swapChain.imageCount, camera, renderPass);
	ui = new UI(vulkanDevice, renderPass, queue, pipelineCache, settings.sampleCount);
	updateOverlay();
	recordCommandBuffers();
	prepared = true;
}

void VulkanExample::render()
{
	// 这就是Game的mainloop， 包括update和render
	// 此app的模型不改变，只改变UI的参数，模型的旋转角， 
	// 所以,绘制模型的CmdBuf是提前record好的， 不需要改动。
	// 每次只改变曝光度、旋转角等有限的参数，加上预先录制的CmdBuff
	if (!prepared) {
		return;
	}
	// update 到内存
	updateOverlay();
	pbr1.updateShaderValues();
	update();

	// draw，上传、提交，etc
	// 确保以前此ID提交的任务完成，先等待，再reset为unsignaled以重复使用
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
