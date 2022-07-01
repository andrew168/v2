#include "auxVk.h"
#include "buffer.h"

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

VkResult BufferBase::map(VkDeviceSize size, VkDeviceSize offset)
{
    VkResult result = m_buffer.map(size, offset);
    mapped = m_buffer.mapped;
    return result;
}

VertexBuffer::VertexBuffer() :
    BufferBase(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
{
}

IndexBuffer::IndexBuffer() :
    BufferBase(VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
{
}

UniformBuffer::UniformBuffer() :
    BufferBase(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
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

void UniformBuffer::bind(VkCommandBuffer& cmdBuf,
    VkDeviceSize offset,
    VkIndexType indexType)
{
    vkCmdBindIndexBuffer(cmdBuf, m_buffer.buffer, offset, indexType);
}

}
