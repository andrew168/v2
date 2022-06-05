#include "auxVk.h"
#include "fence.h"

namespace aux
{
SemaphoreMgr::SemaphoreMgr()
{
}

SemaphoreMgr::~SemaphoreMgr()
{
	auto device = Device::getR();
	
	for (auto semaphore : m_semaphores) {
		vkDestroySemaphore(device, semaphore, nullptr);
	}
}

void SemaphoreMgr::create(uint32_t amount)
{
	m_semaphores.resize(amount);
	for (auto& semaphore : m_semaphores) {
		Semaphore::create(semaphore);
	}
}

VkSemaphore &SemaphoreMgr::getR(uint32_t frameIndex)
{
	return m_semaphores[frameIndex];
}

}
