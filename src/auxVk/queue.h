#pragma once
#include "..\vk.h"
#include "image.h"

namespace aux
{
class Queue
{
    VkQueue* m_pQueue;
public:
    Queue(VkQueue& queue);
    const VkQueue& getR() { return *m_pQueue; }
    void submit(std::vector< VkCommandBuffer>& cmdBufs,
        VkPipelineStageFlags waitDstStageMask,
        std::vector< VkSemaphore>& waits,
        std::vector< VkSemaphore>& signals, VkFence fence = VK_NULL_HANDLE);
};
}
