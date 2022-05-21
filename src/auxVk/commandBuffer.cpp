#include "commandBuffer.h"

namespace aux
{
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
