#include "auxVk.h"
#include "vertexBuffer.h"

namespace aux
{
BufferBase::BufferBase(VkBufferUsageFlagBits bufferType) :
    m_bufferType(bufferType)
{
}

BufferBase::~BufferBase()
{
    m_buffer.destroy();
}

VertexBuffer::VertexBuffer() :
    BufferBase(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
{
}

IndexBuffer::IndexBuffer() :
    BufferBase(VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
{
}

void VertexBuffer::bind(VkCommandBuffer& cmdBuf,
    uint32_t firstBinding,
    uint32_t bindingCount,
    const VkDeviceSize* pOffsets)
{
    vkCmdBindVertexBuffers(cmdBuf, firstBinding, bindingCount, &(m_buffer.buffer), pOffsets);
}

void IndexBuffer::bind(VkCommandBuffer& cmdBuf,
    VkDeviceSize offset,
    VkIndexType indexType)
{
    vkCmdBindIndexBuffer(cmdBuf, m_buffer.buffer, offset, indexType);
}

}
