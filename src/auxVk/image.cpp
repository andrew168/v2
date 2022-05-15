#include "device.h"
#include "image.h"

namespace aux
{
Image::Image(ImageCI& auxCi):
    m_deviceMemory(nullptr),
    m_view(nullptr)
{
    VK_CHECK_RESULT(vkCreateImage(*(Device::get()), static_cast<VkImageCreateInfo *>(&auxCi), nullptr, &m_image));

    allocMemory(auxCi);
    VK_CHECK_RESULT(vkBindImageMemory(*(Device::get()), m_image, m_deviceMemory, 0));
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
    vkGetImageMemoryRequirements(*Device::get(), m_image, &memReqs);
    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = Device::getVksDevice()->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(*Device::get(), &memAllocInfo, nullptr, &m_deviceMemory));
}

void Image::createImageView(ImageCI &auxCi)
{
    VkImageViewCreateInfo viewCI{};
    viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCI.viewType = auxCi.isCubemap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
    viewCI.format = auxCi.format;
    viewCI.subresourceRange = {};
    viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewCI.subresourceRange.levelCount = auxCi.mipLevels;
    viewCI.subresourceRange.layerCount = auxCi.arrayLayers;
    viewCI.image = m_image;
    VK_CHECK_RESULT(vkCreateImageView(*Device::get(), &viewCI, nullptr, &m_view));
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
    VK_CHECK_RESULT(vkCreateSampler(*Device::get(), &samplerCI, nullptr, &m_sampler));
}
}
