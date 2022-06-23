#pragma once
#include "..\vk-all.h"

namespace aux
{

class IMBarrier {
public:
    inline static VkImageMemoryBarrier create()
    {
        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        return imageMemoryBarrier;
    }

    static void csResult2Sampler(VkImage& image,
        VkCommandBuffer& cmdBuf);

    static void convertLayoutToTransfer(aux::Image& auxImage,
        VkCommandBuffer& cmdBuf, 
        VkQueue& queue);

    static void colorAttachment2Transfer(aux::Image& auxImage,
        VkCommandBuffer& cmdBuf);

    static void transfer2ColorAttachment(aux::Image& auxImage,
        VkCommandBuffer& cmdBuf);

    static void transfer2ShaderRead(aux::Image& auxImage,
        VkCommandBuffer& cmdBuf);

    static void toColorAttachment(aux::Image& auxImage,
        VkQueue& queue);
};
}
