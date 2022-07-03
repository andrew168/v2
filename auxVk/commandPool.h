#pragma once
#include "..\vk-all.h"
#include "device.h"

namespace aux
{
class CommandPool
{
    VkCommandPool m_pool;
public:
    CommandPool();
    CommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    ~CommandPool();

    void create(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    VkCommandPool* get() { return &m_pool; }
    VkCommandPool& getR() { return m_pool; }
};
}
