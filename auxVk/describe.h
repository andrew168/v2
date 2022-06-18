#pragma once
#include "..\vk-all.h"
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

    static void buffer(VkWriteDescriptorSet& ds,
        VkDescriptorSet& dstSet,
        uint32_t dstBinding,
        const VkDescriptorBufferInfo* pBufferInfo);

    static void any(VkWriteDescriptorSet& ds,
        VkDescriptorType type,
        VkDescriptorSet& dstSet,
        uint32_t dstBinding,
        const void* pImageOrBufferInfo);

    static void Describe::bufferUpdate(VkDescriptorSet& dstSet,
        uint32_t dstBinding,
        const VkDescriptorBufferInfo* pBufferInfo);
};

}
