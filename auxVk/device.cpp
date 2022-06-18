#include "auxVk.h"

namespace aux 
{
VkPhysicalDevice* Device::m_pPhysicalDevice = nullptr;
VkDevice* Device::m_pDevice = nullptr;
VkQueue* Device::m_pQueue = nullptr;
vks::VulkanDevice* Device::m_vksDevice = nullptr;

bool Device::hasStorageImage(VkFormat format)
{
    // Get device properties for the requested texture format
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(Device::getPhysicalDeviceR(), format, &formatProperties);
    return (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);
}

bool Device::hasLinearTiling(VkFormat format)
{
    // Get device properties for the requested texture format
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(Device::getPhysicalDeviceR(), format, &formatProperties);
    // 检查是否支持Linear Tiling
    return (formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
}

}
