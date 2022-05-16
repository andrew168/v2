#pragma once
#include "..\vk.h"

namespace aux
{

class IMBarrier {
public:
    static void convertLayoutToTransfer(aux::Image& auxImage,
        VkCommandBuffer& cmbBuf, 
        VkQueue& queue);
};
}
