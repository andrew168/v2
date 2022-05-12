#pragma once
#include "..\vk.h"

namespace aux
{
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
    Image(VkFormat format, int32_t width, int32_t height);
    Image(VkFormat format, int32_t cubeLength, VkImageCreateFlagBits flags);
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
private:
    VkImageCreateInfo getDefaultCI();
    void createImage();
    void createCubemap();
    void allocMemory();
    void createImageView();
    void createSampler();
};
}
