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
	VkCommandPool &cmdPool, 
	std::vector<VkCommandBuffer>& cmdBufs)
{
	VkCommandBufferAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	ai.commandPool = cmdPool;
	ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	ai.commandBufferCount = static_cast<uint32_t>(cmdBufs.size());
	VK_CHECK_RESULT(vkAllocateCommandBuffers(Device::getR(), &ai, cmdBufs.data()));
}
}
