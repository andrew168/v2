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

    static void Describe::buffer(VkWriteDescriptorSet& ds,
        VkDescriptorSet& dstSet,
        uint32_t dstBinding,
        const VkDescriptorBufferInfo* pBufferInfo);

    static void Describe::bufferUpdate(VkDescriptorSet& dstSet,
        uint32_t dstBinding,
        const VkDescriptorBufferInfo* pBufferInfo);
};

}
