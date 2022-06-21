#pragma once
#include "..\vk-all.h"
#include "image.h"

namespace aux
{
class Queue
{
    // VkQueue 本身是一个Handle，i.e. 指针，所以，可以直接作为member，不必再用指针
    // #define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;
    // VK_DEFINE_HANDLE(VkQueue)
    
    VkDevice m_device;
    VkQueue m_queue;
public:
    Queue();
    Queue(VkQueue& queue);
    void acquire(
        VkDevice device,
        uint32_t queueFamilyIndex,
        uint32_t queueIndex = 0);

    const VkQueue& getR() { return m_queue; }
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
