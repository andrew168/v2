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
    void pushConstantsToFS(VkPipelineLayout layout,
        uint32_t offset,
        uint32_t size,
        const void* pConstants);
    void pushConstantsToVsFs(VkPipelineLayout layout,
        uint32_t offset,
        uint32_t size,
        const void* pConstants);
    static void CommandBuffer::allocate(
        VkCommandPool& cmdPool, 
        std::vector<VkCommandBuffer>& cmdBufs);
};
}
