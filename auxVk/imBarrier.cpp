#include "device.h"
#include "commandBuffer.h"
#include "image.h"
#include "imBarrier.h"

namespace aux
{
void IMBarrier::convertLayoutToTransfer(aux::Image& auxImage,
	VkCommandBuffer &cmdBuf, 
	VkQueue& queue)
{
	// ToDo:: CmdBuf是否必须是新的? 因为此处要begin/end/flush
	vks::VulkanDevice* vulkanDevice = aux::Device::getVksDevice();

	VkImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = auxImage.getMipLevels();
	subresourceRange.layerCount = auxImage.getArrayLayers();

	// Change image layout for all cubemap faces to transfer destination
	{
		CommandBuffer auxCmdBuf(cmdBuf);
		auxCmdBuf.begin();
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.image = auxImage.getImage();
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.subresourceRange = subresourceRange;
		vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	}
}

void IMBarrier::colorAttachment2Transfer(aux::Image& auxImage,
	VkCommandBuffer& cmdBuf)
{
	vks::VulkanDevice* vulkanDevice = aux::Device::getVksDevice();

	VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = auxImage.getMipLevels();
	subresourceRange.layerCount = auxImage.getArrayLayers();

	{
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.image = auxImage.getImage();
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	}
}

void IMBarrier::transfer2ColorAttachment(aux::Image& auxImage,
	VkCommandBuffer& cmdBuf)
{
	VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.image = auxImage.getImage();
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

void IMBarrier::transfer2ShaderRead(aux::Image& auxImage,
	VkCommandBuffer& cmdBuf)
{
	VkImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = auxImage.getMipLevels();
	subresourceRange.layerCount = auxImage.getArrayLayers();

	VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.image = auxImage.getImage();
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarrier.subresourceRange = subresourceRange;
	vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

void IMBarrier::toColorAttachment(aux::Image& auxImage,
	VkQueue& queue)
{
	vks::VulkanDevice* vulkanDevice = aux::Device::getVksDevice();

	VkCommandBuffer cmdBuf = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
	VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.image = auxImage.getImage();
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imageMemoryBarrier.srcAccessMask = 0;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	vulkanDevice->flushCommandBuffer(cmdBuf, queue, true);
}
}
