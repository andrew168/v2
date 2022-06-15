#pragma once
#include "..\vk-all.h"

namespace aux
{
struct ShaderDescription;
class ShaderStage
{
public:
    static VkPipelineShaderStageCreateInfo loadShader(VkDevice device, std::string filename, VkShaderStageFlagBits stage);
    static VkPipelineShaderStageCreateInfo loadShader(std::string filename, VkShaderStageFlagBits stage);
    static VkPipelineShaderStageCreateInfo loadShader(ShaderDescription& shaderDesc);
};
}
