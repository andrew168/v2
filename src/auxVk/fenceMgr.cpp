#include "auxVk.h"
#include "fence.h"

namespace aux
{
FenceMgr::FenceMgr()
{

}

FenceMgr::~FenceMgr()
{
	auto device = Device::getR();
	for (auto fence : m_fences) {
		vkDestroyFence(device, fence, nullptr);
	}
}

void FenceMgr::create(uint32_t amount)
{
	m_fences.resize(amount);
	// Command buffer execution fences
	for (auto& waitFence : m_fences) {
		Fence::create(waitFence, true);
	}
}

void FenceMgr::wait(uint32_t frameIndex)
{
	Fence::wait(m_fences[frameIndex]);
}

void FenceMgr::reset(uint32_t frameIndex)
{
	Fence::reset(m_fences[frameIndex]);
}
VkFence &FenceMgr::get(uint32_t frameIndex)
{
	return m_fences[frameIndex];
}

}
