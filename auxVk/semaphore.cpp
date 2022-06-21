#include "auxVk.h"
#include "semaphore.h"

namespace aux
{
Semaphore::Semaphore()
{
}
Semaphore::~Semaphore()
{
	vkDestroySemaphore(m_device, m_semaphore, nullptr);
}

void Semaphore::create()
{
	m_device = Device::getR();
	VkSemaphoreCreateInfo ci{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0 };
	VK_CHECK_RESULT(vkCreateSemaphore(m_device, &ci, nullptr, &m_semaphore));
}

void Semaphore::create(VkSemaphore& semaphore) 
{
	Assert(0, "TBD");
	VkSemaphoreCreateInfo ci{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0 };
	VK_CHECK_RESULT(vkCreateSemaphore(Device::getR(), &ci, nullptr, &semaphore));
}
}
