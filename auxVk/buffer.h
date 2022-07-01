#pragma once
#include "auxVk.h"

namespace aux
{
struct Vertex {
    float pos[3];
    float uv[2];
};

/*
*/
class BufferBase {  
protected:
	VkBufferUsageFlags m_bufferType = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vks::Buffer m_buffer;
public:
	void* mapped = nullptr;

	BufferBase(VkBufferUsageFlagBits bufferType = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	~BufferBase();
	template<typename T>
	inline void create(std::vector<T>& data);
	inline void create(
		VkBufferUsageFlags usageFlags,
		VkMemoryPropertyFlags memoryPropertyFlags,
		VkDeviceSize size,
		void* data = nullptr)
	{
		m_bufferType = usageFlags;
		VK_CHECK_RESULT(Device::getVksDevice()->createBuffer(m_bufferType, 
			memoryPropertyFlags, &m_buffer, size, data));
	};

	inline VkBufferMemoryBarrier barrier(
	VkAccessFlags      srcAccessMask = 0,
	VkAccessFlags      dstAccessMask = 0,
	uint32_t           srcQueue  = VK_QUEUE_FAMILY_IGNORED,
	uint32_t           dstQueue = VK_QUEUE_FAMILY_IGNORED) 
	{
		VkBufferMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.srcAccessMask = srcAccessMask;
		barrier.dstAccessMask = dstAccessMask;
		barrier.srcQueueFamilyIndex = srcQueue; //  VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = dstQueue; //  VK_QUEUE_FAMILY_IGNORED;
		barrier.buffer = m_buffer.buffer;
		barrier.size = m_buffer.size; //  m_buffer.descriptor.range;
		barrier.offset = 0;
		return barrier;
	}
	
	inline static VkBufferCreateInfo ci()
	{
		VkBufferCreateInfo bufCreateInfo{};
		bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		return bufCreateInfo;
	}

	VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

	vks::Buffer* getVks() { return &m_buffer; }
	VkBuffer get() { return m_buffer.buffer; }
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
	inline static VkVertexInputBindingDescription bindings(
		uint32_t binding,
		uint32_t stride,
		VkVertexInputRate inputRate)
	{
		VkVertexInputBindingDescription vInputBindDescription{};
		vInputBindDescription.binding = binding;
		vInputBindDescription.stride = stride;
		vInputBindDescription.inputRate = inputRate;
		return vInputBindDescription;
	}

	inline static VkVertexInputAttributeDescription desc(
		uint32_t binding,
		uint32_t location,
		VkFormat format,
		uint32_t offset = 0)
	{
		VkVertexInputAttributeDescription vInputAttribDescription{};
		vInputAttribDescription.location = location;
		vInputAttribDescription.binding = binding;
		vInputAttribDescription.format = format;
		vInputAttribDescription.offset = offset;
		return vInputAttribDescription;
	}



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

