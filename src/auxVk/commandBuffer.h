#pragma once
#include "..\vk.h"
#include "device.h"

namespace aux
{
class CommandBuffer
{
    VkCommandBuffer* m_pCmdBuf;
    bool m_isVK = false;

public:
    CommandBuffer();
    CommandBuffer(VkCommandBuffer& cmdBuf);
    ~CommandBuffer();
    
    void begin(const VkCommandBufferBeginInfo* pBeginInfo = nullptr);
    void end();
    void flush(VkQueue& queue, bool free = true);
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

    void draw(uint32_t  vertexCount,
        uint32_t  instanceCount = 1,
        uint32_t  firstVertex = 0,
        uint32_t  firstInstance = 0);

    void drawIndexed(uint32_t  indexCount,
        uint32_t  instanceCount = 1,
        uint32_t  firstIndex = 0,
        int32_t   vertexOffset = 0,
        uint32_t  firstInstance = 0);


    void bindVertexBuffers(
        uint32_t firstBinding,
        uint32_t bindingCount,
        const VkBuffer* pBuffers,
        const VkDeviceSize* pOffsets);

    void bindIndexBuffer(
        VkBuffer        buffer,
        VkDeviceSize    offset = 0,
        VkIndexType     indexType = VK_INDEX_TYPE_UINT32);

    static void CommandBuffer::allocate(
        VkCommandPool& cmdPool, 
        std::vector<VkCommandBuffer>& cmdBufs);

    VkCommandBuffer* get() { return m_pCmdBuf; }
};
}
