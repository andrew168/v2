#include "auxVk.h"

namespace aux 
{
    VkDevice* Device::m_device = nullptr;
    vks::VulkanDevice* Device::m_vksDevice = nullptr;

    Image::Image(VkFormat format, int32_t width, int32_t height) :
        m_format(format),
        m_width(width),
        m_height(height),
        m_deviceMemory(nullptr),
        m_view(nullptr)
    {
        createImage();
        allocMemory();
        VK_CHECK_RESULT(vkBindImageMemory(*(Device::get()), m_image, m_deviceMemory, 0));
        createImageView();
    }

    void Image::createImage()
    {
        VkImageCreateInfo imageCI{};
        imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCI.imageType = VK_IMAGE_TYPE_2D;
        imageCI.format = m_format;
        imageCI.extent.width = m_width;
        imageCI.extent.height = m_height;
        imageCI.extent.depth = 1;
        imageCI.mipLevels = 1;
        imageCI.arrayLayers = 1;
        imageCI.samples = VK_SAMPLE_COUNT_1_BIT; // 1个sample 每pixel 
        imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        VK_CHECK_RESULT(vkCreateImage(*(Device::get()), &imageCI, nullptr, &m_image));
    }

    void Image::allocMemory() {
        VkMemoryRequirements memReqs;
        vkGetImageMemoryRequirements(*Device::get(), m_image, &memReqs);
        VkMemoryAllocateInfo memAllocInfo{};
        memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = Device::getVksDevice()->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(*Device::get(), &memAllocInfo, nullptr, &m_deviceMemory));
    }
   
    void Image::createImageView()
    {
        VkImageViewCreateInfo viewCI{};
        viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCI.format = m_format;
        viewCI.subresourceRange = {};
        viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewCI.subresourceRange.levelCount = 1;
        viewCI.subresourceRange.layerCount = 1;
        viewCI.image = m_image;
        VK_CHECK_RESULT(vkCreateImageView(*Device::get(), &viewCI, nullptr, &m_view));
    }
}
