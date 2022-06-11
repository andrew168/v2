#include "..\vk-all.h"
#include "..\auxVk\auxVk.h"
#include "pbr.h"

namespace pbr
{
using namespace aux;

aux::Image* Pbr::m_pBrdfLutImage = nullptr; // 依赖vks,不能delete，

/*
	Generate a BRDF integration map storing roughness/NdotV as a look-up-table
*/

void Pbr::generateBRDFLUT()
{
	auto tStart = std::chrono::high_resolution_clock::now();
	auto queue = Device::getQueue();
	const VkFormat format = VK_FORMAT_R16G16_SFLOAT;
	const int32_t dim = 512;

	aux::ImageCI lutBrdfCI(format, dim, dim);
	m_pBrdfLutImage = new aux::Image(lutBrdfCI);
	aux::Image &brdfLutImage = *m_pBrdfLutImage;
	aux::RenderPass auxRenderPass(brdfLutImage);

	VkRenderPass renderpass = *(auxRenderPass.get());
	aux::Framebuffer auxFramebuffer(brdfLutImage, auxRenderPass);
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
	auto tEnd = std::chrono::high_resolution_clock::now();
	auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
	std::cout << "Generating BRDF LUT took " << tDiff << " ms" << std::endl;

	brdfLutImage.toVKS(m_textures.lutBrdf);
}
}
