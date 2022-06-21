#pragma once
#include "..\vk-all.h"
#include "image.h"

namespace aux
{
class Semaphore
{
    VkSemaphore m_semaphore;
    VkDevice m_device;
public:
    Semaphore();
    ~Semaphore();
    void create();
    static void create(VkSemaphore& semaphore);

    VkSemaphore getR() { return m_semaphore; }
};
}
