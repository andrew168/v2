#include "auxVk.h"
#include "swapChain.h"

namespace aux
{
SwapChain::SwapChain()
{
};

SwapChain::~SwapChain()
{
}

void SwapChain::init(VulkanSwapChain& vksSwapChain)
{
    m_pVksSwapChain = &vksSwapChain;
}

VkResult SwapChain::acquireNextImage(VkSemaphore presentSemaphore, uint32_t* imageIndex)
{
    //从Swapchain中，获取下一个可用PI image的ID存入imageIndex，
    //当此PI Image完成present之后，触发信号presentSemaphore
    return m_pVksSwapChain->acquireNextImage(presentSemaphore, imageIndex);
}

VkResult SwapChain::queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore)
{
    //present绘图结果，用SwapChain中的PI image （ID为imageIndex），
    // 等到render完成信号(waitSemaphore)出现之后，才执行本任务，i.e. present
    //（当present完成之后，需要触发的信号presentSemaphore在前面获取PI image的时候acquireNextImage已经指定了）
   return m_pVksSwapChain->queuePresent(queue, imageIndex, waitSemaphore);
}

uint32_t SwapChain::imageCount()
{
    return m_pVksSwapChain->imageCount;
}

}
