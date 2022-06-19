#include "device.h"
#include "image.h"

namespace aux
{
Image::Image(ImageCI& ci) :
	m_deviceMemory(nullptr),
	m_view(nullptr)
{
	VK_CHECK_RESULT(vkCreateImage(Device::getR(), static_cast<VkImageCreateInfo*>(&ci), nullptr, &m_image));

	allocMemory(ci);
	VK_CHECK_RESULT(vkBindImageMemory(Device::getR(), m_image, m_deviceMemory, 0));
	createImageView(ci);
	createSampler(ci);

	m_width = ci.extent.width;
	m_height = ci.extent.height;
	m_format = ci.format;
	m_mipLevels = ci.mipLevels;
	m_arrayLayers = ci.arrayLayers;
	m_descriptor.imageLayout = m_layout;
	m_descriptor.imageView = m_view;
	m_descriptor.sampler = m_sampler;
}

void Image::allocMemory(ImageCI& ci) {
	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(Device::getR(), m_image, &memReqs);
	VkMemoryAllocateInfo memAllocInfo{};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = Device::getVksDevice()->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(Device::getR(), &memAllocInfo, nullptr, &m_deviceMemory));
}

void Image::createImageView(ImageCI& ci)
{
	VkImageViewCreateInfo viewCI{};
	viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCI.viewType = ci.isCubemap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
	viewCI.format = ci.format;
	viewCI.subresourceRange = {};
	viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewCI.subresourceRange.baseMipLevel = 0; // 缺省值
	viewCI.subresourceRange.baseArrayLayer = 0; // 缺省值
	viewCI.subresourceRange.levelCount = ci.mipLevels;
	viewCI.subresourceRange.layerCount = ci.arrayLayers;
	viewCI.image = m_image;
	if (ci.pComponentMapping != nullptr) {
		//RGBA分量存储的内容不一定是R、G、B、A，而可能是0， 1，Identiy
		viewCI.components = *ci.pComponentMapping;
	}

	// default value?
	viewCI.flags = 0;
	viewCI.subresourceRange.baseMipLevel = 0;
	viewCI.subresourceRange.baseArrayLayer = 0;

	VK_CHECK_RESULT(vkCreateImageView(Device::getR(), &viewCI, nullptr, &m_view));
}

void Image::createSampler(ImageCI& ci) {
	VkSamplerCreateInfo samplerCI{};
	samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCI.magFilter = VK_FILTER_LINEAR;
	samplerCI.minFilter = VK_FILTER_LINEAR;
	samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCI.minLod = 0.0f;
	samplerCI.maxLod = static_cast<float>(ci.mipLevels);
	samplerCI.maxAnisotropy = 1.0f;
	samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerCI.compareOp = VK_COMPARE_OP_NEVER; // 缺省值
	VK_CHECK_RESULT(vkCreateSampler(Device::getR(), &samplerCI, nullptr, &m_sampler));
}

void Image::copyOneMip2Cube(
	VkCommandBuffer& cmdBuf,
	aux::Image& auxImage,
	VkExtent3D& region,
	aux::Image& cube,
	uint32_t arrayLayers,
	uint32_t mipLevels)
{
	// Copy region for transfer from framebuffer to cube face
	VkImageCopy copyRegion{};

	copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.srcSubresource.baseArrayLayer = 0;
	copyRegion.srcSubresource.mipLevel = 0;
	copyRegion.srcSubresource.layerCount = 1;
	copyRegion.srcOffset = { 0, 0, 0 };

	copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.dstSubresource.baseArrayLayer = arrayLayers;
	copyRegion.dstSubresource.mipLevel = mipLevels;
	copyRegion.dstSubresource.layerCount = 1;
	copyRegion.dstOffset = { 0, 0, 0 };

	copyRegion.extent.width = static_cast<uint32_t>(region.width);
	copyRegion.extent.height = static_cast<uint32_t>(region.height);
	copyRegion.extent.depth = 1;

	vkCmdCopyImage(
		cmdBuf,
		auxImage.getImage(),
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		cube.getImage(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&copyRegion);
}
/////////////////////// extr for vks /////////////////////
void Image::toVKS(vks::Texture& vks)
{
	vks.image = getImage();
	vks.deviceMemory = getDeviceMemory();
	vks.view = getView();
	vks.sampler = getSampler();
	vks.descriptor.imageView = getView();
	vks.descriptor.sampler = vks.sampler;
	vks.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	vks.device = Device::getVksDevice();
}

void Image::changeLayout(VkCommandBuffer& cmdbuffer,
	VkImageLayout newImageLayout,
	VkImageAspectFlags aspectMask,
	VkPipelineStageFlags srcStageMask,
	VkPipelineStageFlags dstStageMask)
{
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = aspectMask;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = 1;


	// Create an image barrier object
	VkImageMemoryBarrier imageMemoryBarrier = vks::initializers::imageMemoryBarrier();
	imageMemoryBarrier.oldLayout = m_layout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = m_image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	// Source layouts (old)
	// Source access mask controls actions that have to be finished on the old layout
	// before it will be transitioned to the new layout
	switch (m_layout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		// Image layout is undefined (or does not matter)
		// Only valid as initial layout
		// No flags required, listed only for completeness
		imageMemoryBarrier.srcAccessMask = 0;
		break;

	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		// Image is preinitialized
		// Only valid as initial layout for linear images, preserves memory contents
		// Make sure host writes have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		// Image is a color attachment
		// Make sure any writes to the color buffer have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image is a depth/stencil attachment
		// Make sure any writes to the depth/stencil buffer have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		// Image is a transfer source
		// Make sure any reads from the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image is a transfer destination
		// Make sure any writes to the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image is read by a shader
		// Make sure any shader reads from the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		// Other source layouts aren't handled (yet)
		break;
	}

	// Target layouts (new)
	// Destination access mask controls the dependency for the new image layout
	switch (newImageLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image will be used as a transfer destination
		// Make sure any writes to the image have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		// Image will be used as a transfer source
		// Make sure any reads from the image have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		// Image will be used as a color attachment
		// Make sure any writes to the color buffer have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image layout will be used as a depth/stencil attachment
		// Make sure any writes to depth/stencil buffer have been finished
		imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image will be read in a shader (sampler, input attachment)
		// Make sure any writes to the image have been finished
		if (imageMemoryBarrier.srcAccessMask == 0)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		// Other source layouts aren't handled (yet)
		break;
	}

	// Put barrier inside setup command buffer
	vkCmdPipelineBarrier(
		cmdbuffer,
		srcStageMask,
		dstStageMask,
		0,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier);

	m_layout = newImageLayout;
}
}
