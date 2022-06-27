#include "device.h"
#include "memory.h"
#include "image.h"
#include "imBarrier.h"
#include "VertexBuffer.h"
#include "commandBuffer.h"
#include "../util/util.h"
#include "../util/log.h"

namespace aux
{
Image::Image() :
	m_deviceMemory(nullptr),
	m_view(nullptr)
{ // for load from file
}

Image::Image(ImageCI& ci) :
	m_deviceMemory(nullptr),
	m_view(nullptr)
{
	init(ci, m_layout);
}

/**
* Initialize a Image through a 2D texture file
*/
Image::Image(
	std::string filename, 
	VkFormat format, 
	VkImageUsageFlags imageUsageFlags, 
	VkImageLayout imageLayout)
{
	vks::VulkanDevice& device = *Device::getVksDevice();
	const VkQueue& queue = Device::getQueue();
	ktxTexture* ktxTexture;
	ktxResult result = loadKTXFile(filename, &ktxTexture);
	assert(result == KTX_SUCCESS);

	ImageCI ci(format, ktxTexture->baseWidth, ktxTexture->baseHeight, ktxTexture->numLevels, ktxTexture->numLayers,
		ktxTexture_GetData(ktxTexture),
		static_cast<uint32_t>(ktxTexture_GetSize(ktxTexture)));

	// 确保有TRANSFER_DST标识
	if (!(imageUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
	{
		imageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	ci.usage = imageUsageFlags;

	init(ci, imageLayout, ktxTexture);
	// ToDo: 能不能立即删除？ 是否需要等upload结束之后再destroy？
	ktxTexture_Destroy(ktxTexture);
}

void Image::init(ImageCI& ci, VkImageLayout imageLayout, ktxTexture* pKtxTexture)
{
	createImage(ci);

	if (ci.m_pData != nullptr)
	{
		if (pKtxTexture->numLevels == 1) {
			copyData(ci, *pKtxTexture, imageLayout);
		}
		else {
			std::vector<VkBufferImageCopy> regions;
			calRegions(regions, pKtxTexture);
			copyData(ci, *pKtxTexture, imageLayout, &regions);
		}
	}

	init9(ci.format, ci);
}

ktxResult Image::loadKTXFile(std::string filename, ktxTexture** target)
{
	// 使用KTX的lib
	ktxResult result = KTX_SUCCESS;
	if (!fileExists(filename)) {
		Log::fatal("Could not load texture from " + filename);
	}

	result = ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, target);
	return result;
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
	// 只有Optimal tiling方式下，才设置mip map（因为Linear tiling 一般不支持mipmap)
	viewCI.subresourceRange.levelCount = ci.mipLevels;
	viewCI.subresourceRange.layerCount = ci.arrayLayers;
	viewCI.image = m_image;
	if (ci.pComponentMapping != nullptr) {
		//RGBA分量存储的内容不一定是R、G、B、A，而可能是0， 1，Identiy
		viewCI.components = *ci.pComponentMapping;
	}

	// default value?
	viewCI.flags = 0;
	VK_CHECK_RESULT(vkCreateImageView(Device::getR(), &viewCI, nullptr, &m_view));
}

void Image::createSampler(ImageCI& ci) {
	VkSamplerCreateInfo samplerCI{};
	const vks::VulkanDevice& device = *Device::getVksDevice();
	samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCI.magFilter = VK_FILTER_LINEAR;
	samplerCI.minFilter = VK_FILTER_LINEAR;
	samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;  // VK_SAMPLER_ADDRESS_MODE_REPEAT
	samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCI.mipLodBias = 0.0f;
	samplerCI.minLod = 0.0f;
	samplerCI.maxLod = static_cast<float>(ci.mipLevels);
	samplerCI.anisotropyEnable = device.enabledFeatures.samplerAnisotropy;
	samplerCI.maxAnisotropy = device.enabledFeatures.samplerAnisotropy ?
		device.properties.limits.maxSamplerAnisotropy : 1.0f;
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
	VkImageMemoryBarrier imageMemoryBarrier = IMBarrier::create();
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

void Image::copyData(ImageCI& ci,
	ktxTexture& ktxTexture, 
	VkImageLayout imageLayout,
	std::vector<VkBufferImageCopy>* pRegions)
{
	vks::VulkanDevice& device = *Device::getVksDevice();
	const VkQueue& queue = Device::getQueue();

	// 1 copy image data to stage, 
	if (Device::hasLinearTiling(ci.format))
	{
		Log::info("linear tiling supportted");
	}
	//使用optimal tiling，因为linear tiling只支持少量formats和features (mip maps, cubemaps, arrays等)		
	// Create a host-visible staging buffer that contains the raw image data
	VkBufferCreateInfo bufferCreateInfo = BufferBase::ci();
	bufferCreateInfo.size = ci.m_dataSize;
	// This buffer is used as a transfer source for the buffer copy
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK_RESULT(vkCreateBuffer(device.logicalDevice, &bufferCreateInfo, nullptr, &stagingBuffer));

	// Get memory requirements for the staging buffer (alignment, memory type bits)
	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(device.logicalDevice, stagingBuffer, &memReqs);

	VkMemoryAllocateInfo memAllocInfo = Memory::ai();
	memAllocInfo.allocationSize = memReqs.size;
	// Get memory type index for a host visible buffer
	memAllocInfo.memoryTypeIndex = device.getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VK_CHECK_RESULT(vkAllocateMemory(device.logicalDevice, &memAllocInfo, nullptr, &stagingMemory));
	VK_CHECK_RESULT(vkBindBufferMemory(device.logicalDevice, stagingBuffer, stagingMemory, 0));

	// Copy texture data into staging buffer
	uint8_t* data;
	VK_CHECK_RESULT(vkMapMemory(device.logicalDevice, stagingMemory, 0, memReqs.size, 0, (void**)&data));
	memcpy(data, ci.m_pData, ci.m_dataSize);
	vkUnmapMemory(device.logicalDevice, stagingMemory);

	// 建立image： image， 分配并bind内存：deviceMemory

    // Use a separate command buffer for texture loading
	VkCommandBuffer copyCmd = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
	CommandBuffer auxCmd;
	auxCmd.begin();
	//changeLayout(auxCmd.getR(), VK_IMAGE_LAYOUT_GENERAL);
	//auxCmd.flush();


	// Image barrier for optimal image (target)
	// Optimal image will be used as destination for the copy
	changeLayout(auxCmd.getR(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// 从staging buffer 到image，复制image数据
	auxCmd.copyBufferToImage(stagingBuffer, *this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, pRegions);

	//修改Image的layout，准备shader 采样
	changeLayout(auxCmd.getR(), imageLayout);

	auxCmd.flush(); //ToDo: 用copyQueue，是否会更快？ 
	auxCmd.free();

	// Clean up staging resources
	vkFreeMemory(device.logicalDevice, stagingMemory, nullptr);
	vkDestroyBuffer(device.logicalDevice, stagingBuffer, nullptr);
}

void Image::loadFromFile(
	std::string filename,
	VkFormat format,
	VkQueue copyQueue,
	VkImageUsageFlags imageUsageFlags,
	VkImageLayout imageLayout)
{
	ktxTexture* pKtxTexture;
	loadToStage(&pKtxTexture, filename, format, copyQueue, imageUsageFlags, imageLayout);

	auto& ktxTexture = pKtxTexture;

	ImageCI ci(format, ktxTexture->baseWidth, ktxTexture->baseHeight,
		ktxTexture->numLevels, ktxTexture->numLayers,
		ktxTexture_GetData(ktxTexture),
		static_cast<uint32_t>(ktxTexture_GetSize(ktxTexture)));

	// 确保有TRANSFER_DST标识
	if (!(imageUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
	{
		imageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	ci.usage = imageUsageFlags;

	createImage(ci);
	fromStageToImage(format, imageUsageFlags, imageLayout, ktxTexture, ci);
	ktxTexture_Destroy(ktxTexture);
	init9(format, ci);
}

void Image::createImage(ImageCI &ci)
{
	vks::VulkanDevice* device = Device::getVksDevice();
	const VkQueue& queue = Device::getQueue();

	VK_CHECK_RESULT(vkCreateImage(device->logicalDevice, &ci, nullptr, &m_image));

	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(device->logicalDevice, m_image, &memReqs);

	allocMemory(ci);
	VK_CHECK_RESULT(vkBindImageMemory(Device::getR(), m_image, m_deviceMemory, 0));
}

void Image::loadToStage(
	ktxTexture** ppKtxTexture,
	std::string filename,
	VkFormat format,
	VkQueue copyQueue,
	VkImageUsageFlags imageUsageFlags,
	VkImageLayout imageLayout)
{
    vks::VulkanDevice* device = Device::getVksDevice();
	const VkQueue& queue = Device::getQueue();
	ktxTexture* ktxTexture = *ppKtxTexture;
	ktxResult result = loadKTXFile(filename, &ktxTexture);
	*ppKtxTexture = ktxTexture;
	assert(result == KTX_SUCCESS);

	// this->device = device;
	m_width = ktxTexture->baseWidth;
	m_height = ktxTexture->baseHeight;
	m_mipLevels = ktxTexture->numLevels;

	ktx_uint8_t* ktxTextureData = ktxTexture_GetData(ktxTexture);
	ktx_size_t ktxTextureSize = ktxTexture_GetSize(ktxTexture);

	// Get device properties for the requested texture format
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(device->physicalDevice, format, &formatProperties);

	// Only use linear tiling if requested (and supported by the device)
	// Support for linear tiling is mostly limited, so prefer to use
	// optimal tiling instead
	// On most implementations linear tiling will only support a very
	// limited amount of formats and features (mip maps, cubemaps, arrays, etc.)
	VkMemoryAllocateInfo memAllocInfo = Memory::ai();
	VkMemoryRequirements memReqs;
	// Create a host-visible staging buffer that contains the raw image data

	VkBufferCreateInfo bufferCreateInfo = BufferBase::ci();
	bufferCreateInfo.size = ktxTextureSize;
	// This buffer is used as a transfer source for the buffer copy
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK_RESULT(vkCreateBuffer(device->logicalDevice, &bufferCreateInfo, nullptr, &stagingBuffer));

	// Get memory requirements for the staging buffer (alignment, memory type bits)
	vkGetBufferMemoryRequirements(device->logicalDevice, stagingBuffer, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;
	// Get memory type index for a host visible buffer
	memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllocInfo, nullptr, &stagingMemory));
	VK_CHECK_RESULT(vkBindBufferMemory(device->logicalDevice, stagingBuffer, stagingMemory, 0));

	// Copy texture data into staging buffer
	uint8_t* data;
	VK_CHECK_RESULT(vkMapMemory(device->logicalDevice, stagingMemory, 0, memReqs.size, 0, (void**)&data));
	memcpy(data, ktxTextureData, ktxTextureSize);
	vkUnmapMemory(device->logicalDevice, stagingMemory);
}

void Image::fromStageToImage(
	VkFormat format,
	VkImageUsageFlags imageUsageFlags,
	VkImageLayout imageLayout, 
	ktxTexture* ktxTexture, 
	ImageCI& ci)
{
	vks::VulkanDevice* device = Device::getVksDevice();
	const VkQueue& queue = Device::getQueue();
	
	// Use a separate command buffer for texture loading
	CommandBuffer auxCmd;
	auxCmd.begin();
	VkCommandBuffer copyCmd = auxCmd.getR();
	// Image barrier for optimal image (target)
	// Optimal image will be used as destination for the copy
	changeLayout(auxCmd.getR(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	if (m_mipLevels > 1) {
		// Copy mip levels from staging buffer
		std::vector<VkBufferImageCopy> bufferCopyRegions;
		calRegions(bufferCopyRegions, ktxTexture);
		auxCmd.copyBufferToImage(stagingBuffer, *this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &bufferCopyRegions);
	}
	else {
		auxCmd.copyBufferToImage(stagingBuffer, *this, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	}

	//修改Image的layout，准备shader 采样
	changeLayout(auxCmd.getR(), imageLayout);

	auxCmd.flush(); //ToDo: 用copyQueue，是否会更快？ 
	auxCmd.free();

	// Clean up staging resources
	vkFreeMemory(device->logicalDevice, stagingMemory, nullptr);
	vkDestroyBuffer(device->logicalDevice, stagingBuffer, nullptr);
}

void Image::init9(
	VkFormat format,
	ImageCI& ci)
{
	createSampler(ci);
	createImageView(ci);

	m_width = ci.extent.width;
	m_height = ci.extent.height;
	m_format = ci.format;
	m_mipLevels = ci.mipLevels;
	m_arrayLayers = ci.arrayLayers;
	m_descriptor.imageLayout = m_layout;
	m_descriptor.imageView = m_view;
	m_descriptor.sampler = m_sampler;
}

void Image::calRegions(std::vector<VkBufferImageCopy>& bufferCopyRegions, ktxTexture* ktxTexture)
{
	auto mipLevels = ktxTexture->numLevels;
	for (uint32_t i = 0; i < mipLevels; i++)
	{
		ktx_size_t offset;
		KTX_error_code result = ktxTexture_GetImageOffset(ktxTexture, i, 0, 0, &offset);
		assert(result == KTX_SUCCESS);

		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = i;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = std::max(1u, ktxTexture->baseWidth >> i);
		bufferCopyRegion.imageExtent.height = std::max(1u, ktxTexture->baseHeight >> i);
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = offset;

		bufferCopyRegions.push_back(bufferCopyRegion);
	}
}

}
