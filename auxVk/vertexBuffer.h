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
	void* mapped = nullptr;

	BufferBase(VkBufferUsageFlagBits bufferType);
	~BufferBase();
	template<typename T>
	inline void create(std::vector<T>& data);
	inline static VkBufferCreateInfo ci()
	{
		VkBufferCreateInfo bufCreateInfo{};
		bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		return bufCreateInfo;
	}

	VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

	vks::Buffer* get() { return &m_buffer; }
	VkDescriptorBufferInfo& getDescriptor() { return m_buffer.descriptor; }
};

class VertexBuffer: public BufferBase
{
public:
    VertexBuffer();
	void bind(VkCommandBuffer& cmdBuf,
		uint32_t firstBinding,
		uint32_t bindingCount = 1,
		const VkDeviceSize* pOffsets = nullptr);
};

class IndexBuffer : public BufferBase
{
public:
	IndexBuffer();
	void bind(VkCommandBuffer& cmdBuf,
		VkDeviceSize  offset = 0,
		VkIndexType indexType = VK_INDEX_TYPE_UINT32);
};

class UniformBuffer : public BufferBase
{
public:
	UniformBuffer();
	template<typename T>
	inline void create();
	void bind(VkCommandBuffer& cmdBuf,
		VkDeviceSize  offset = 0,
		VkIndexType indexType = VK_INDEX_TYPE_UINT32);
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

template<typename T>
inline void UniformBuffer::create()
{
	VK_CHECK_RESULT(Device::getVksDevice()->createBuffer(
		m_bufferType,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&m_buffer,
		sizeof(T)));
}

}

