#pragma once
#include "auxVk.h"
#include "..\base\VulkanSwapChain.h"

namespace aux
{
class RenderPass;


class SwapChain {
    VulkanSwapChain *m_pVksSwapChain;

public:
    SwapChain();
    ~SwapChain();
    void init(VulkanSwapChain& vksSwapChain);
    uint32_t imageCount();
    VkResult queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);
    VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex);

};
}

