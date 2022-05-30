#include "..\vk.h"
#include "..\auxVk\auxVk.h"
#include "..\gltf\gltf.h"
#include "pbr.h"
namespace v2
{
using namespace aux;

struct PushBlockPrefilterEnv {
	glm::mat4 mvp;
	float roughness;
	uint32_t numSamples = 32u;
};

struct PushBlockIrradiance {
	glm::mat4 mvp;
	float deltaPhi = (2.0f * float(M_PI)) / 180.0f;
	float deltaTheta = (0.5f * float(M_PI)) / 64.0f;
};
/*
* 用offscreen渲染生成PBR Lighting的2个cubemap：
	- 亮度，辐照度(Irradiance) cube map
	- 预过滤的环境图： (Pre-filterd environment) cubemap, 
先生成每一个面、每一个mip level，再合成到cubemap中。
*/
void Pbr::generateCubemaps(std::vector<Image>& cubemaps, gltf::Model& skyboxModel, vks::Texture& texture)
{
	VkFormat format;
	int32_t dim;

	format = VK_FORMAT_R32G32B32A32_SFLOAT;
	dim = 64;
	std::vector<aux::ShaderDescription> shaders = {
	{"filtercube.vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
	{ "irradiancecube.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT }
	};
	cubemaps.push_back(*generateCubemap(skyboxModel, texture, format, dim, shaders, sizeof(PushBlockIrradiance), &shaders));

	format = VK_FORMAT_R16G16B16A16_SFLOAT;
	dim = 512;

	std::vector<aux::ShaderDescription> shadersEnv = {
			{"filtercube.vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
			{ "prefilterenvmap.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT }
	};

	cubemaps.push_back(*generateCubemap(skyboxModel, texture, 
		format, dim, shadersEnv, sizeof(PushBlockPrefilterEnv), nullptr));
}

/*
* 生成Cubemap：
* 用offscreen渲染生成每一个面、每一个mip level，
* 再copy到cubemap中。
*/
aux::Image* Pbr::generateCubemap(gltf::Model& skyboxModel, vks::Texture& texture,
	VkFormat format, int32_t dim,
	std::vector<aux::ShaderDescription> shaders,
	uint32_t constsSize, const void* constsData)
{
	VkQueue& queue = Device::getQueue();

	PushBlockPrefilterEnv  pushBlockPrefilterEnv;
	auto tStart = std::chrono::high_resolution_clock::now();
	// Create target cubemap
	const uint32_t numMips = static_cast<uint32_t>(floor(log2(dim))) + 1;
	aux::ImageCI cubeCI(format, dim, dim, numMips, 6);
	cubeCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	cubeCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	cubeCI.isCubemap = true;
	aux::Image auxCube(cubeCI);
	aux::SubpassDescription auxSubpassDescription(auxCube);
	aux::RenderPass auxRenderPass(auxCube);

	// Create offscreen framebuffer
	aux::ImageCI offscreenFBCI(format, dim, dim);
	offscreenFBCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	offscreenFBCI.usage =
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	offscreenFBCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	aux::Image auxImageOffscreen(offscreenFBCI);
	aux::Framebuffer auxFramebufferOffscreen(auxImageOffscreen, auxRenderPass);
	aux::IMBarrier::toColorAttachment(auxImageOffscreen, queue);
	// Pipeline layout
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags =
		VK_SHADER_STAGE_VERTEX_BIT |
		VK_SHADER_STAGE_FRAGMENT_BIT;

	pushConstantRange.size = constsSize;

	// Descriptors
	VkDescriptorSetLayoutBinding setLayoutBinding =
	{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr };

	aux::PipelineLayoutCI auxPipelineLayoutCI{};
	auxPipelineLayoutCI.pDslBindings = &setLayoutBinding;
	auxPipelineLayoutCI.pImageInfo = &texture.descriptor;
	auxPipelineLayoutCI.pPushConstantRanges = &pushConstantRange;

	aux::PipelineLayout auxPipelineLayout(auxPipelineLayoutCI);
	VkPipelineLayout pipelinelayout = auxPipelineLayout.get();

	aux::PipelineCI auxPipelineCI{};
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

	aux::CommandBuffer auxCmdBuf;
	VkCommandBuffer cmdBuf = *(auxCmdBuf.get());
	aux::IMBarrier::convertLayoutToTransfer(auxCube, cmdBuf, queue);

	// Cube的6个face，每个MipLevel, 逐个渲染
	for (uint32_t m = 0; m < numMips; m++) {
		for (uint32_t f = 0; f < 6; f++) {

			auxCmdBuf.begin();

			// Render scene from cube face's point of view
			auxRenderPass.begin(&cmdBuf, &auxFramebufferOffscreen, { 0.0f, 0.0f, 0.2f, 0.0f });
			// Pass parameters for current pass using a push constant block

			if (constsData == nullptr) {
				pushBlockPrefilterEnv.mvp = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];
				pushBlockPrefilterEnv.roughness = (float)m / (float)(numMips - 1);
				auxCmdBuf.pushConstantsToVsFs(pipelinelayout, 0, constsSize, &pushBlockPrefilterEnv);
			}
			else {
				PushBlockIrradiance  pushBlockIrradiance;
				pushBlockIrradiance.mvp = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];
				auxCmdBuf.pushConstantsToVsFs(pipelinelayout, 0, constsSize, &pushBlockIrradiance);
			}
			uint32_t vpDim = static_cast<uint32_t>(dim * std::pow(0.5f, m));
			auxCmdBuf.setViewport(vpDim, vpDim);
			auxCmdBuf.setScissor(dim, dim);
			auxPipeline.bindToGraphic(cmdBuf);
			auxPipelineLayout.getDSet()->bindToGraphics(cmdBuf, pipelinelayout);

			VkDeviceSize offsets[1] = { 0 };

			skyboxModel.draw(cmdBuf);
			auxRenderPass.end();

			aux::IMBarrier::colorAttachment2Transfer(auxImageOffscreen, cmdBuf);
			VkExtent3D region;
			region.height = vpDim;
			region.width = vpDim;
			aux::Image::copyOneMip2Cube(cmdBuf, auxImageOffscreen, region, auxCube, f, m);
			aux::IMBarrier::transfer2ColorAttachment(auxImageOffscreen, cmdBuf);
			auxCmdBuf.flush(queue, false);
		}
	}

	auxCmdBuf.begin();
	aux::IMBarrier::transfer2ShaderRead(auxCube, cmdBuf);
	auxCmdBuf.flush(queue, false);
	auto tEnd = std::chrono::high_resolution_clock::now();
	auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
	std::cout << "Generating cube map with " << numMips << " mip levels took " << tDiff << " ms" << std::endl;
	return &auxCube;
}
}
