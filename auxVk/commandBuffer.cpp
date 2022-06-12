#include "commandBuffer.h"

namespace aux
{
CommandBuffer::CommandBuffer()
{
	m_pCmdBuf = new VkCommandBuffer();
	*m_pCmdBuf = Device::getVksDevice()->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
}

CommandBuffer::CommandBuffer(VkCommandBuffer& cmdBuf) :
	m_pCmdBuf(&cmdBuf),
	m_isVK(true)
{
}

void CommandBuffer::begin(VkCommandBufferBeginInfo* pBeginInfo)
{
	if (!pBeginInfo) {
		VkCommandBufferBeginInfo bi{};
		fillBI(bi);
		pBeginInfo = &bi;
	}
	VK_CHECK_RESULT(vkBeginCommandBuffer(*m_pCmdBuf, pBeginInfo));
}

void CommandBuffer::end()
{
	VK_CHECK_RESULT(vkEndCommandBuffer(*m_pCmdBuf));
}

// vks::Flush已经包括了：
// ** end CmdBuf，
// ** 创建fence
// ** submit
// ** 最后 free CmdBuf
void CommandBuffer::flush(VkQueue &queue, bool free)
{
	Device::getVksDevice()->flushCommandBuffer(*m_pCmdBuf, queue, free);
}

void CommandBuffer::setViewport(uint32_t width,
	uint32_t height)
{
	VkViewport viewport{};
	viewport.width = (float)width;
	viewport.height = (float)height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(*m_pCmdBuf, 0, 1, &viewport);
}

void CommandBuffer::setScissor(uint32_t width,
	uint32_t height)
{
	VkRect2D scissor{};
	scissor.extent = { width, height };
	vkCmdSetScissor(*m_pCmdBuf, 0, 1, &scissor);
}

void CommandBuffer::allocate(
	VkCommandPool& cmdPool,
	std::vector<VkCommandBuffer>& cmdBufs)
{
	VkCommandBufferAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	ai.commandPool = cmdPool;
	ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	ai.commandBufferCount = static_cast<uint32_t>(cmdBufs.size());
	VK_CHECK_RESULT(vkAllocateCommandBuffers(Device::getR(), &ai, cmdBufs.data()));
}

void CommandBuffer::pushConstantsToFS(VkPipelineLayout layout,
	uint32_t offset,
	uint32_t size,
	const void* pConstants)
{
	vkCmdPushConstants(*m_pCmdBuf, layout, VK_SHADER_STAGE_FRAGMENT_BIT,
		offset, size, pConstants);
}

void CommandBuffer::pushConstantsToVsFs(VkPipelineLayout layout,
	uint32_t offset,
	uint32_t size,
	const void* pConstants)
{
	vkCmdPushConstants(*m_pCmdBuf, layout,
		VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		offset, size, pConstants);
}
void CommandBuffer::draw(uint32_t  vertexCount,
	uint32_t  instanceCount,
	uint32_t  firstVertex,
	uint32_t  firstInstance)
{
	vkCmdDraw(*m_pCmdBuf, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::drawIndexed(
	uint32_t  indexCount,
	uint32_t  instanceCount,
	uint32_t  firstIndex,
	int32_t   vertexOffset,
	uint32_t  firstInstance)
{
	vkCmdDrawIndexed(*m_pCmdBuf, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBuffer::bindVertexBuffers(
	uint32_t firstBinding,
	uint32_t bindingCount,
	const VkBuffer* pBuffers,
	const VkDeviceSize* pOffsets)
{
	vkCmdBindVertexBuffers(*m_pCmdBuf, firstBinding, bindingCount,
		pBuffers, pOffsets);
}

void CommandBuffer::bindIndexBuffer(
	VkBuffer        buffer,
	VkDeviceSize    offset,
	VkIndexType     indexType) 
{
	vkCmdBindIndexBuffer(*m_pCmdBuf, buffer, offset, indexType);
}

CommandBuffer::~CommandBuffer()
{
	if (m_isVK) {
		return;
	}

	delete m_pCmdBuf;
	m_pCmdBuf = nullptr;
}

void CommandBuffer::fillBI(VkCommandBufferBeginInfo& bi)
{
	bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
}

}