#pragma once
#include "..\vk.h"
#include "device.h"

namespace aux
{
class CommandBuffer
{
public:
    static void CommandBuffer::allocate(
        VkCommandPool& cmdPool, 
        std::vector<VkCommandBuffer>& cmdBufs);
};
}
