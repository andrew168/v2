#pragma once
#include "..\vk.h"
#include "device.h"

namespace aux
{
class CommandBuffer
{
    VkCommandBuffer* m_pCmdBuf;

public:
    CommandBuffer(VkCommandBuffer& cmdBuf);
    void setViewport(uint32_t width, uint32_t height);
    void setScissor(uint32_t width, uint32_t height);


    static void CommandBuffer::allocate(
        VkCommandPool& cmdPool, 
        std::vector<VkCommandBuffer>& cmdBufs);
};
}
