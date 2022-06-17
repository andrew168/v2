#include "auxVk.h"
#include "queue.h"

namespace aux
{
Queue::Queue(VkQueue& queue) :
	m_pQueue(&queue)
{

}
void Queue::submit(VkCommandBuffer* pCmdBuf,
	VkPipelineStageFlags waitDstStageMask,
	VkSemaphore wait,
	VkSemaphore signal,
	VkFence fence)
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pWaitDstStageMask = &waitDstStageMask;

	submitInfo.pCommandBuffers = pCmdBuf;
	submitInfo.commandBufferCount = 1;

	if (wait != VK_NULL_HANDLE) {
		submitInfo.pWaitSemaphores = &wait;
		submitInfo.waitSemaphoreCount = 1;
	}

	if (signal != VK_NULL_HANDLE) {
		submitInfo.pSignalSemaphores = &signal;
		submitInfo.signalSemaphoreCount = 1;
	}

	VK_CHECK_RESULT(vkQueueSubmit(*m_pQueue, 1, &submitInfo, fence));
}

void Queue::submit(std::vector< VkCommandBuffer> &cmdBufs,
	VkPipelineStageFlags waitDstStageMask,
	std::vector< VkSemaphore>& waits,
	std::vector< VkSemaphore>& signals, VkFence fence)
{

	std::vector<VkPipelineStageFlags> stages = {waitDstStageMask};
	Queue::submit(cmdBufs, stages, waits, signals, fence);
}

void Queue::submit(std::vector< VkCommandBuffer>& cmdBufs,
	std::vector< VkPipelineStageFlags>& waitDstStageMasks,
	std::vector< VkSemaphore>& waits,
	std::vector< VkSemaphore>& signals, VkFence fence)
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pWaitDstStageMask = waitDstStageMasks.data();
	submitInfo.pWaitSemaphores = waits.data();
	submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waits.size());
	submitInfo.pSignalSemaphores = signals.data();
	submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signals.size());
	submitInfo.pCommandBuffers = cmdBufs.data();
	submitInfo.commandBufferCount = static_cast<uint32_t>(cmdBufs.size());
	VK_CHECK_RESULT(vkQueueSubmit(*m_pQueue, 1, &submitInfo, fence));
}

void Queue::submit(VkPipelineStageFlags waitDstStageMask,
	VkSemaphore wait,
	VkSemaphore signal, 
	VkFence fence)
{
	VkSubmitInfo submitInfo{};

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pCommandBuffers = nullptr;
	submitInfo.commandBufferCount = 0;
	submitInfo.pWaitDstStageMask = &waitDstStageMask;
	
	if (wait != VK_NULL_HANDLE) {
		submitInfo.pWaitSemaphores = &wait;
		submitInfo.waitSemaphoreCount = 1;
	}

	if (signal != VK_NULL_HANDLE) {
		submitInfo.pSignalSemaphores = &signal;
		submitInfo.signalSemaphoreCount = 1;
	}

	VK_CHECK_RESULT(vkQueueSubmit(*m_pQueue, 1, &submitInfo, fence));
}

void Queue::submit(VkSemaphore signal,
	VkFence fence)
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	if (signal != VK_NULL_HANDLE) {
		submitInfo.pSignalSemaphores = &signal;
		submitInfo.signalSemaphoreCount = 1;
	}

	submitInfo.pCommandBuffers = nullptr;
	submitInfo.commandBufferCount = 0;
	VK_CHECK_RESULT(vkQueueSubmit(*m_pQueue, 1, &submitInfo, fence));
}

void Queue::waitIdle()
{
	VK_CHECK_RESULT(vkQueueWaitIdle(*m_pQueue));
}

}
