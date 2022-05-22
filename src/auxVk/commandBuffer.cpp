#include "commandBuffer.h"

namespace aux
{
CommandBuffer::CommandBuffer(VkCommandBuffer& cmdBuf) :
	m_pCmdBuf(&cmdBuf)
{
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
}