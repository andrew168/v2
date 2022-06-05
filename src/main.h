﻿#include "auxVk/auxVk.h"
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

	std::vector<VkCommandBuffer> commandBuffers;
	FenceMgr fenceMgr;
	SemaphoreMgr renderSemaphoreMgr;
	SemaphoreMgr presentSemaphoreMgr;
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

	VulkanExample();

	~VulkanExample();
	void recordCommandBuffers();
	void loadScene(std::string filename);
	void destroyCubemaps();
	void loadEnvironment(std::string filename);
	void loadAssets();
	void updateLights();
	void windowResized();
	void prepare();
	void updateOverlay();
	virtual void render();
	void update();
};