#pragma once
#include "..\vk.h"
#include "image.h"

namespace aux
{
class Fence
{
public:
    static void create(VkFence& fence, bool signaled = false);
};
}
