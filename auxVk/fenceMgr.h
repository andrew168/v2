#pragma once
#include "..\vk-all.h"
#include "image.h"

namespace aux
{
class FenceMgr
{
    std::vector<VkFence> m_fences;

public:
    FenceMgr();
    ~FenceMgr();
    void create(uint32_t amount);
    void wait(uint32_t frameIndex);
    void reset(uint32_t frameIndex);
    VkFence& get(uint32_t frameIndex);
};
}
