#pragma once
#include "..\vk-all.h"
#include "device.h"

namespace aux
{
class CommandPool
{
    VkCommandPool m_pool;
public:
    CommandPool(uint32_t queueFamilyIndex);
    ~CommandPool();

    VkCommandPool* get() { return &m_pool; }
    VkCommandPool& getR() { return m_pool; }
};
}
