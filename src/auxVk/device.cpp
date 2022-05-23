#include "auxVk.h"

namespace aux 
{
VkDevice* Device::m_pDevice = nullptr;
VkQueue* Device::m_pQueue = nullptr;
vks::VulkanDevice* Device::m_vksDevice = nullptr;
}
