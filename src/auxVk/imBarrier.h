#pragma once
#include "..\vk.h"

namespace aux
{

class IMBarrier {
public:
    static void convertLayoutToTransfer(aux::Image& auxImage,
        VkCommandBuffer& cmdBuf, 
        VkQueue& queue);

    static void colorAttachment2Transfer(aux::Image& auxImage,
        VkCommandBuffer& cmdBuf);

    static void transfer2ColorAttachment(aux::Image& auxImage,
        VkCommandBuffer& cmdBuf);

    static void toColorAttachment(aux::Image& auxImage,
        VkQueue& queue);

};
}
