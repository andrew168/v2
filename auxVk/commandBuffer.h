#pragma once
#include "..\vk-all.h"
#include "device.h"

namespace aux
{
class Image;

class CommandBuffer
{
    VkCommandBuffer* m_pCmdBuf;
    VkCommandPool m_cmdPool;

    bool m_isVK = false;

public:
    CommandBuffer();
    CommandBuffer(VkCommandBuffer& cmdBuf);
    CommandBuffer(VkCommandPool& pool);
    ~CommandBuffer();
    void fillBI(VkCommandBufferBeginInfo& pBeginInfo);
    void begin(VkCommandBufferBeginInfo* pBeginInfo = nullptr);    
    void end();
    void flush(VkQueue queue = VK_NULL_HANDLE, bool free = false); //ToDo: 这个free参数，误导用户， 不留它
    void free();
    void setViewport(
        float width,
        float height,
        float x0 = 0,
        float y0 = 0,
        float minDepth = 0.0f,
        float maxDepth = 1.0f);
    void setScissor(
        uint32_t width, 
        uint32_t height, 
        uint32_t x0 = 0,
        uint32_t y0 = 0);
    void pushConstantsToFS(VkPipelineLayout layout,
        uint32_t offset,
        uint32_t size,
        const void* pConstants);

    void pushConstantsToVsFs(VkPipelineLayout layout,
        uint32_t offset,
        uint32_t size,
        const void* pConstants);

    void copyBufferToImage(
        VkBuffer srcBuffer,
        Image  dstImage,
        VkImageLayout dstLayout,
        std::vector<VkBufferImageCopy>* pSrcRegions = nullptr);

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

    void CommandBuffer::barrier(
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        const VkBufferMemoryBarrier* pBufferMemoryBarriers = nullptr,
        VkDependencyFlags dependencyFlags = 0);

    void barrier(
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        VkDependencyFlags dependencyFlags,
        uint32_t memoryBarrierCount = 0,
        const VkMemoryBarrier* pMemoryBarriers = nullptr,
        uint32_t bufferMemoryBarrierCount = 0,
        const VkBufferMemoryBarrier* pBufferMemoryBarriers = nullptr,
        uint32_t imageMemoryBarrierCount = 0,
        const VkImageMemoryBarrier* pImageMemoryBarriers = nullptr);
    
    void dispatch(
        uint32_t groupCountX,
        uint32_t groupCountY,
        uint32_t groupCountZ);

    static inline VkCommandBufferBeginInfo bi()
    {
        VkCommandBufferBeginInfo cmdBufferBeginInfo{};
        cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        return cmdBufferBeginInfo;
    }

    static void allocate(
        VkCommandPool& cmdPool, 
        std::vector<VkCommandBuffer>& cmdBufs);

    static void allocate(
        VkCommandPool& cmdPool,
        VkCommandBuffer& cmdBuf);

    VkCommandBuffer* get() { return m_pCmdBuf; }
    VkCommandBuffer& getR() { return *m_pCmdBuf; }
};
}
