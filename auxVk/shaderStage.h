#pragma once
#include "..\vk-all.h"

namespace aux
{
class ShaderStage
{
public:
    static VkPipelineShaderStageCreateInfo loadShader(VkDevice device, std::string filename, VkShaderStageFlagBits stage);
};
}
