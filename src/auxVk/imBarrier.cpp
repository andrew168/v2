#include "device.h"
#include "image.h"
#include "imBarrier.h"

namespace aux
{
void IMBarrier::convertLayoutToTransfer(aux::Image& auxImage,
	VkCommandBuffer &cmdBuf, 
	VkQueue& queue)
{
	vks::VulkanDevice* vulkanDevice = aux::Device::getVksDevice();

	VkImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = auxImage.getMipLevels();
	subresourceRange.layerCount = auxImage.getArrayLayers();

	// Change image layout for all cubemap faces to transfer destination
	{
		vulkanDevice->beginCommandBuffer(cmdBuf);
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.image = auxImage.getImage();
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.subresourceRange = subresourceRange;
		vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		vulkanDevice->flushCommandBuffer(cmdBuf, queue, false);
	}
}
}
