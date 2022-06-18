#include "device.h"
#include "image.h"

namespace aux
{
Image::Image(ImageCI& auxCi) :
    m_deviceMemory(nullptr),
    m_view(nullptr)
{
    VK_CHECK_RESULT(vkCreateImage(Device::getR(), static_cast<VkImageCreateInfo*>(&auxCi), nullptr, &m_image));

    allocMemory(auxCi);
    VK_CHECK_RESULT(vkBindImageMemory(Device::getR(), m_image, m_deviceMemory, 0));
    createImageView(auxCi);
    createSampler(auxCi);

    m_width = auxCi.extent.width;
    m_height = auxCi.extent.height;
    m_format = auxCi.format;
    m_mipLevels = auxCi.mipLevels;
    m_arrayLayers = auxCi.arrayLayers;
}

void Image::allocMemory(ImageCI& auxCi) {
    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(Device::getR(), m_image, &memReqs);
    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = Device::getVksDevice()->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(Device::getR(), &memAllocInfo, nullptr, &m_deviceMemory));
}

void Image::createImageView(ImageCI& auxCi)
{
    VkImageViewCreateInfo viewCI{};
    viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCI.viewType = auxCi.isCubemap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
    viewCI.format = auxCi.format;
    viewCI.subresourceRange = {};
    viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewCI.subresourceRange.baseMipLevel = 0; // 缺省值
    viewCI.subresourceRange.baseArrayLayer = 0; // 缺省值
    viewCI.subresourceRange.levelCount = auxCi.mipLevels;
    viewCI.subresourceRange.layerCount = auxCi.arrayLayers;
    viewCI.image = m_image;
    if (auxCi.pComponentMapping != nullptr) {
        //RGBA分量存储的内容不一定是R、G、B、A，而可能是0， 1，Identiy
        viewCI.components = *auxCi.pComponentMapping;
    }

    // default value?
    viewCI.flags = 0;
    viewCI.subresourceRange.baseMipLevel = 0;
    viewCI.subresourceRange.baseArrayLayer = 0;

    VK_CHECK_RESULT(vkCreateImageView(Device::getR(), &viewCI, nullptr, &m_view));
}

void Image::createSampler(ImageCI& auxCi) {
    VkSamplerCreateInfo samplerCI{};
    samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCI.magFilter = VK_FILTER_LINEAR;
    samplerCI.minFilter = VK_FILTER_LINEAR;
    samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCI.minLod = 0.0f;
    samplerCI.maxLod = static_cast<float>(auxCi.mipLevels);
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
}
