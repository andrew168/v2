#pragma once
#include "..\vk.h"
#include "device.h"

namespace aux
{
class Describe
{
public:
    static void image(VkWriteDescriptorSet& ds,
        VkDescriptorSet& dstSet,
        uint32_t dstBinding,
        const VkDescriptorImageInfo* pImageInfo);
};

}
