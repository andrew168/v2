#include "auxVk.h"
#include "queue.h"

namespace aux
{
Queue::Queue(VkQueue& queue) :
	m_pQueue(&queue)
{

}
void Queue::submit(std::vector< VkCommandBuffer> &cmdBufs,
	VkPipelineStageFlags waitDstStageMask,
	std::vector< VkSemaphore>& waits,
	std::vector< VkSemaphore>& signals, VkFence fence)
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pWaitDstStageMask = &waitDstStageMask;
	submitInfo.pWaitSemaphores = waits.data();
	submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waits.size());
	submitInfo.pSignalSemaphores = signals.data();
	submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signals.size());
	submitInfo.pCommandBuffers = cmdBufs.data();
	submitInfo.commandBufferCount = static_cast<uint32_t>(cmdBufs.size());
	VK_CHECK_RESULT(vkQueueSubmit(*m_pQueue, 1, &submitInfo, fence));
}
}
