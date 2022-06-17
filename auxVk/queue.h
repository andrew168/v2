#pragma once
#include "..\vk-all.h"
#include "image.h"

namespace aux
{
class Queue
{
    VkQueue* m_pQueue;
public:
    Queue(VkQueue& queue);
    const VkQueue& getR() { return *m_pQueue; }
    void submit(VkSemaphore signal = VK_NULL_HANDLE,
        VkFence fence = VK_NULL_HANDLE);

    void submit(VkPipelineStageFlags waitDstStageMask,
        VkSemaphore wait,
        VkSemaphore signal = VK_NULL_HANDLE, 
        VkFence fence = VK_NULL_HANDLE);

    void submit(VkCommandBuffer* pCmdBuf,
        VkPipelineStageFlags waitDstStageMask,
        VkSemaphore wait = VK_NULL_HANDLE,
        VkSemaphore signal = VK_NULL_HANDLE,
        VkFence fence = VK_NULL_HANDLE);

    void submit(std::vector<VkCommandBuffer>& cmdBufs,
        VkPipelineStageFlags waitDstStageMask,
        std::vector<VkSemaphore>& waits,
        std::vector<VkSemaphore>& signals, VkFence fence = VK_NULL_HANDLE);

    void submit(std::vector<VkCommandBuffer>& cmdBufs,
        std::vector<VkPipelineStageFlags>& waitDstStageMasks,
        std::vector<VkSemaphore>& waits,
        std::vector<VkSemaphore>& signals, VkFence fence = VK_NULL_HANDLE);

    void waitIdle();
};

}
