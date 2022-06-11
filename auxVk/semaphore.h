#pragma once
#include "..\vk-all.h"
#include "image.h"

namespace aux
{
class Semaphore
{
public:
    static void create(VkSemaphore& semaphore);
};
}
