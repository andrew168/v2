#include "auxVk.h"
#include "vertexBuffer.h"

namespace aux
{
BufferBase::BufferBase(VkBufferUsageFlagBits bufferType):
    m_bufferType(bufferType)
{
}

BufferBase::~BufferBase()
{
}

VertexBuffer::VertexBuffer() :
    BufferBase(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
{
}

IndexBuffer::IndexBuffer() :
    BufferBase(VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
{
}
}
