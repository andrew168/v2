#include "auxVk.h"
#include "SubpassDescription.h"

namespace aux
{
void Semaphore::create(VkSemaphore& semaphore) 
{
	VkSemaphoreCreateInfo ci{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0 };
	VK_CHECK_RESULT(vkCreateSemaphore(Device::getR(), &ci, nullptr, &semaphore));
}
}
