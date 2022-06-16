#pragma once
#include "auxVk.h"

namespace aux
{
struct Vertex {
    float pos[3];
    float uv[2];
};

class BufferBase {
protected:
	VkBufferUsageFlagBits m_bufferType = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vks::Buffer m_buffer;
public:
	BufferBase(VkBufferUsageFlagBits bufferType);
	~BufferBase();
	template<typename T>
	inline void create(std::vector<T>& data);

	vks::Buffer* get() { return &m_buffer; }
};

class VertexBuffer: public BufferBase
{
public:
    VertexBuffer();
};

class IndexBuffer : public BufferBase
{
public:
	IndexBuffer();
};

template<typename T>
inline void BufferBase::create(std::vector<T>& data)
{
	// Create buffers
	// For the sake of simplicity we won't stage the vertex data to the gpu memory
	// Vertex buffer
	VK_CHECK_RESULT(Device::getVksDevice()->createBuffer(
		m_bufferType,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&m_buffer,
		data.size() * sizeof(T),
		data.data()));
}

}

