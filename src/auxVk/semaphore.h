#pragma once
#include "..\vk.h"
#include "image.h"

namespace aux
{
class Semaphore
{
public:
    static void create(VkSemaphore& semaphore);
};
}
