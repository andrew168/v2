#pragma once
#include "..\vk.h"
#include "image.h"

namespace aux
{
class SemaphoreMgr
{
    std::vector<VkSemaphore> m_semaphores;

public:
    SemaphoreMgr();
    ~SemaphoreMgr();
    void create(uint32_t amount);
    VkSemaphore& getR(uint32_t frameIndex);
};
}
