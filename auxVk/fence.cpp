#include "auxVk.h"
#include "fence.h"

namespace aux
{
void Fence::create(VkFence& fence, bool signaled)
{
	VkFenceCreateInfo fenceCI{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 
		signaled? VK_FENCE_CREATE_SIGNALED_BIT: static_cast<VkFenceCreateFlags>(0) }; // 0 转为enum,范围缩小
	VK_CHECK_RESULT(vkCreateFence(Device::getR(), &fenceCI, nullptr, &fence));
}

void Fence::wait(const VkFence& fence,
    VkBool32 waitAll,
    uint64_t timeout)
{
    VK_CHECK_RESULT(vkWaitForFences(Device::getR(), 1, &fence, waitAll, timeout));
}

void Fence::reset(VkFence& fence)
{
	VK_CHECK_RESULT(vkResetFences(Device::getR(), 1, &fence));
}

}
