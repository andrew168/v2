#pragma once
#include "..\vk-all.h"
#include "image.h"

namespace aux
{
struct ShaderDescription
{
    std::string m_fileName;
    VkShaderStageFlagBits m_stage;
    ShaderDescription(std::string fileName, VkShaderStageFlagBits stage) : 
        m_fileName(fileName),
        m_stage(stage)
    {        
    }
};
}

