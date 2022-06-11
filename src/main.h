#include "v2/v2.h"
#include "VulkanExampleBase.h"
#include "VulkanTexture.hpp"
#include "VulkanglTFModel.h"
#include "VulkanUtils.hpp"
#include "ui.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
	SwapChain auxSwapChain;
	const uint32_t renderAhead = 2; //ToDo: 需要与UB，FB的数量保持一致吗？
	uint32_t frameIndex = 0;
	int32_t animationIndex = 0;
	bool animate = true;
	bool displayBackground = true;

	struct LightSource {
		glm::vec3 color = glm::vec3(1.0f);
		glm::vec3 rotation = glm::vec3(75.0f, 40.0f, 0.0f);
	} lightSource;

	UI* ui;

	const std::string assetpath = "./../data/";

	bool rotateModel = false;
	glm::vec3 modelrot = glm::vec3(0.0f);
	glm::vec3 modelPos = glm::vec3(0.0f);

	std::string selectedEnvironment = "papermill";

	int32_t debugViewInputs = 0;
	int32_t debugViewEquation = 0;

	VulkanExample();

	~VulkanExample();
	void recordCommandBuffers();
	void loadScene(std::string filename);
	void loadAssets();
	void updateLights();
	void windowResized();
	void prepare();
	void updateOverlay();
	virtual void render();
	void update();
};
