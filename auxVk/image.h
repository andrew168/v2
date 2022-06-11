#pragma once
#include "..\vk-all.h"

namespace aux
{

struct ImageCI: public VkImageCreateInfo
{
    bool isCubemap;
    ImageCI(VkFormat _format = VK_FORMAT_R32G32B32A32_SFLOAT, 
        int32_t _width = 1, 
        int32_t _height = 1,
        int32_t _mipLevels = 1,
        int32_t _arrayLayers = 1) :
        VkImageCreateInfo()
    {
        sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageType = VK_IMAGE_TYPE_2D;
        format = _format;
        extent.width = _width;
        extent.height = _height;
        extent.depth = 1;
        mipLevels = _mipLevels;
        arrayLayers = _arrayLayers;
        samples = VK_SAMPLE_COUNT_1_BIT; // 1个sample 每pixel 
        tiling = VK_IMAGE_TILING_OPTIMAL;
        usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        isCubemap = false;
    }
};

class Image {
    uint32_t m_width;
    uint32_t m_height;
    VkFormat m_format;
    uint32_t m_mipLevels;
    uint32_t m_arrayLayers;
    bool m_isCubemap;
    VkImage m_image;
    VkDevice m_device;
    VkDeviceMemory m_deviceMemory;
    VkImageView m_view;
    VkSampler m_sampler;

public:
    static void copyOneMip2Cube(
        VkCommandBuffer& cmdBuf, 
        aux::Image& auxImage,
        VkExtent3D& region,
        aux::Image& cube, 
        uint32_t arrayLayers,
        uint32_t mipLevels);
    Image(ImageCI &ci);
    VkFormat getFormat() { return m_format; }
    VkImage getImage() { return m_image; }
    VkDeviceMemory getDeviceMemory() { return m_deviceMemory; }
    VkImageView getView() { return m_view; }
    VkImageView* getViewP() { return &m_view; }
    uint32_t getMipLevels() { return m_mipLevels; }
    uint32_t getArrayLayers() { return m_arrayLayers; }
    VkSampler getSampler() { return m_sampler; }
    uint32_t getWidth() { return m_width; }
    uint32_t getHeight() { return m_height; }
    void toVKS(vks::Texture& vks);
private:
    void allocMemory(ImageCI& auxCi);
    void createImageView(ImageCI& auxCi);
    void createSampler(ImageCI& auxCi);
};
}
