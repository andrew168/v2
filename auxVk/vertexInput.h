#pragma once
#include "auxVk.h"

namespace aux
{

class VertexInput {
    VulkanSwapChain* m_pVksSwapChain;

public:
    // usecase 1： 1次binding，2次attrib (i.e. 把uv和xyz放在一个大buffer）
    // usecase 2:  2次binding，2次attrib（i.e. uv和xyz分开存放）
    static VkVertexInputBindingDescription bindingDesc(
        uint32_t             binding,
        uint32_t             stride,
        VkVertexInputRate    inputRate)
    {
        VkVertexInputBindingDescription bindingDesc{};
        bindingDesc.binding = binding;
        bindingDesc.stride = stride;
        bindingDesc.inputRate = inputRate;
        return bindingDesc;
    }

    static VkVertexInputAttributeDescription AttribiteDesc(
        uint32_t    location,
        uint32_t    binding,
        VkFormat    format,
        uint32_t    offset)
    {
        VkVertexInputAttributeDescription attributeDesc{};
        attributeDesc.location = location;
        attributeDesc.binding = binding;
        attributeDesc.format = format;
        attributeDesc.offset = offset;
        return attributeDesc;
    }
};
}

