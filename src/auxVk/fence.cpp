#include "auxVk.h"
#include "fence.h"

namespace aux
{
void Fence::create(VkFence& fence, bool signaled)
{
	VkFenceCreateInfo fenceCI{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 
		signaled? VK_FENCE_CREATE_SIGNALED_BIT:0 };
	VK_CHECK_RESULT(vkCreateFence(Device::getR(), &fenceCI, nullptr, &fence));
}
}
