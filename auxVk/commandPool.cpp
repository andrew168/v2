#include "commandPool.h"

namespace aux
{
CommandPool::CommandPool()
{
}

CommandPool::CommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
{
	create(queueFamilyIndex, flags);
}

void CommandPool::create(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
{
	// Separate command pool as queue family for compute may be different than graphics
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
	cmdPoolInfo.flags = flags;
	VK_CHECK_RESULT(vkCreateCommandPool(Device::getR(), &cmdPoolInfo, nullptr, &m_pool));
}

CommandPool::~CommandPool()
{
	vkDestroyCommandPool(Device::getR(), m_pool, nullptr);
}

}