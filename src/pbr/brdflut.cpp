#include "..\vk.h"
#include "..\auxVk\auxVk.h"
#include "brdflut.h"

namespace pbr
{
vks::Texture2D brdfLutTexture;
using namespace aux;

/*
	Generate a BRDF integration map storing roughness/NdotV as a look-up-table
*/

aux::Image& generateBRDFLUT()
{
	auto tStart = std::chrono::high_resolution_clock::now();
	auto queue = Device::getQueue();
	const VkFormat format = VK_FORMAT_R16G16_SFLOAT;
	const int32_t dim = 512;

	aux::ImageCI lutBrdfCI(format, dim, dim);
	auto p = new aux::Image(lutBrdfCI);
	aux::Image &lutBrdfImage = *p;
	brdfLutTexture.image = lutBrdfImage.getImage();
	brdfLutTexture.deviceMemory = lutBrdfImage.getDeviceMemory();
	brdfLutTexture.view = lutBrdfImage.getView();
	brdfLutTexture.sampler = lutBrdfImage.getSampler();

	aux::RenderPass auxRenderPass(lutBrdfImage);

	VkRenderPass renderpass = *(auxRenderPass.get());
	aux::Framebuffer auxFramebuffer(lutBrdfImage, auxRenderPass);
	aux::PipelineLayoutCI auxPlCi{};
	aux::PipelineLayout auxPipelineLayout(auxPlCi);
	VkPipelineLayout pipelinelayout = auxPipelineLayout.get();

	aux::PipelineCI auxPipelineCI{};
	std::vector<aux::ShaderDescription> shaders = {
		{"genbrdflut.vert.spv", VK_SHADER_STAGE_VERTEX_BIT},
		{"genbrdflut.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT}
	};
	auxPipelineCI.shaders = shaders;
	aux::Pipeline auxPipeline(auxPipelineLayout, *auxRenderPass.get(), auxPipelineCI);

	// Render
	aux::CommandBuffer auxCmdBuf;
	VkCommandBuffer cmdBuf = *(auxCmdBuf.get());
	auxRenderPass.begin(&cmdBuf, &auxFramebuffer);
	auxCmdBuf.setViewport(dim, dim);
	auxCmdBuf.setScissor(dim, dim);
	auxPipeline.bindToGraphic(cmdBuf);
	auxCmdBuf.draw(3, 1, 0, 0);
	auxRenderPass.end();
	auxCmdBuf.flush(queue);

	vkQueueWaitIdle(queue);

	brdfLutTexture.descriptor.imageView = brdfLutTexture.view;
	brdfLutTexture.descriptor.sampler = brdfLutTexture.sampler;
	brdfLutTexture.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	brdfLutTexture.device = Device::getVksDevice();


	brdfLutTexture.view = lutBrdfImage.getView();

	brdfLutTexture.sampler = lutBrdfImage.getSampler();

	brdfLutTexture.image = lutBrdfImage.getImage();
	brdfLutTexture.deviceMemory = lutBrdfImage.getDeviceMemory();

	auto tEnd = std::chrono::high_resolution_clock::now();
	auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
	std::cout << "Generating BRDF LUT took " << tDiff << " ms" << std::endl;

	return lutBrdfImage;
}
}
