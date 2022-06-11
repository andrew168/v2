#pragma once
#include "..\vk-all.h"
#include "image.h"

namespace aux
{
class Fence
{
public:
    static void create(VkFence& fence, bool signaled = false);
    static void reset(VkFence& fence);
    static void wait(const VkFence& fence,
        VkBool32 waitAll = VK_TRUE,
        uint64_t timeout = UINT64_MAX);
};
}
