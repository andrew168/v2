#include "commandBuffer.h"
#include "image.h"

namespace aux
{
CommandBuffer::CommandBuffer()
{
	// ToDo: 从Device剥离CmdPool。
	// 因为CmdPool与Queue family有关，图形和计算Queue有各自的pool，
	m_pCmdBuf = new VkCommandBuffer();
	m_cmdPool = (Device::getVksDevice())->commandPool;
	CommandBuffer::allocate(m_cmdPool, *m_pCmdBuf);
}

CommandBuffer::CommandBuffer(VkCommandPool& pool)
{
	m_cmdPool = pool;
	m_pCmdBuf = new VkCommandBuffer();
	CommandBuffer::allocate(m_cmdPool, *m_pCmdBuf);
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
void CommandBuffer::flush(VkQueue queue, bool free)
{
	if (queue == VK_NULL_HANDLE) {
		queue = Device::getQueue();
	}
	Device::getVksDevice()->flushCommandBuffer(*m_pCmdBuf, queue, free);
}

void CommandBuffer::free()
{
	//ToDo: CmdBuf应该记录自己属于哪一个Pool，哪一个Device，唯一的， （记录上级、上线）
	// 而Device可能有多个Pool
	vkFreeCommandBuffers(Device::getR(), m_cmdPool, 1, m_pCmdBuf);
}

void CommandBuffer::setViewport(float width,
	float height,
	float x0,
	float y0,
	float minDepth, 
	float maxDepth)
{
	VkViewport viewport{};
	viewport.width = (float)width;
	viewport.height = (float)height;
	viewport.x = x0;
	viewport.y = y0;
	viewport.minDepth = minDepth;
	viewport.maxDepth = maxDepth;
	vkCmdSetViewport(*m_pCmdBuf, 0, 1, &viewport);
}

void CommandBuffer::setScissor(uint32_t width,
	uint32_t height,
	uint32_t x0,
	uint32_t y0)
{
	VkRect2D rect2D{};
	rect2D.extent = { width, height };
	rect2D.offset.x = x0;
	rect2D.offset.y = y0;
	vkCmdSetScissor(*m_pCmdBuf, 0, 1, &rect2D);
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

void CommandBuffer::allocate(
	VkCommandPool& cmdPool,
	VkCommandBuffer& cmdBuf)
{
	VkCommandBufferAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	ai.commandPool = cmdPool;
	ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	ai.commandBufferCount = 1;
	VK_CHECK_RESULT(vkAllocateCommandBuffers(Device::getR(), &ai, &cmdBuf));
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
	// 使用已经bind的vertex、Index和texture绘制
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

void CommandBuffer::barrier(
	VkPipelineStageFlags srcStageMask,
	VkPipelineStageFlags dstStageMask,
	const VkBufferMemoryBarrier* pBufferMemoryBarriers,
	VkDependencyFlags dependencyFlags)
{
	vkCmdPipelineBarrier(
		*m_pCmdBuf,
		srcStageMask,
		dstStageMask,
		dependencyFlags, // 0, 
		0, nullptr,
		1, pBufferMemoryBarriers,
		0, nullptr);
}

void CommandBuffer::barrier(
	VkPipelineStageFlags srcStageMask,
	VkPipelineStageFlags dstStageMask,
	VkDependencyFlags dependencyFlags,
	uint32_t memoryBarrierCount,
	const VkMemoryBarrier* pMemoryBarriers,
	uint32_t bufferMemoryBarrierCount,
	const VkBufferMemoryBarrier* pBufferMemoryBarriers,
	uint32_t imageMemoryBarrierCount,
	const VkImageMemoryBarrier* pImageMemoryBarriers)
{
	vkCmdPipelineBarrier(
		*m_pCmdBuf,
		srcStageMask,
		dstStageMask,
		dependencyFlags, // 0, 
		memoryBarrierCount, pMemoryBarriers, // 0, nullptr,
		bufferMemoryBarrierCount, pBufferMemoryBarriers, // 1, &buffer_barrier,
		imageMemoryBarrierCount, pImageMemoryBarriers);
}

void CommandBuffer::dispatch(
	uint32_t groupCountX,
	uint32_t groupCountY,
	uint32_t groupCountZ)
{
	vkCmdDispatch(*m_pCmdBuf, groupCountX, groupCountY, groupCountZ);
}

void CommandBuffer::copyBufferToImage(
	VkBuffer srcBuffer,
	Image  dstImage,
	VkImageLayout dstLayout,
	std::vector<VkBufferImageCopy>* pSrcRegions)
{
	if (pSrcRegions == nullptr) {
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = 1;
		// 欲copy图像中的哪些layer：从baseArrayLayer（0）开始，总共layerCount个
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = std::max(1u, dstImage.getWidth());
		bufferCopyRegion.imageExtent.height = std::max(1u, dstImage.getHeight());
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = 0;

		vkCmdCopyBufferToImage(*m_pCmdBuf, srcBuffer, dstImage.getImageR(), dstLayout, 1, &bufferCopyRegion);
	}
	else 
	{
		vkCmdCopyBufferToImage(*m_pCmdBuf, srcBuffer, dstImage.getImageR(), dstLayout,
			static_cast<uint32_t>(pSrcRegions->size()), pSrcRegions->data());
	}
}

}
