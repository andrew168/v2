#pragma once
#include "..\vk-all.h"
#include "device.h"
#include "pipelineLayout.h"
#include "renderPass.h"
#include "shaderDescription.h"

namespace aux
{
class RenderPass;
struct PipelineCI {
    VkPrimitiveTopology primitiveTopology;
    VkCullModeFlags cullMode;
    VkSampleCountFlagBits rasterizationSamples;
    VkBool32  depthTestEnable;
    VkBool32  depthWriteEnable;
    VkCompareOp depthCompareOp;
    VkBool32                 blendEnable;
    VkBlendFactor            srcColorBlendFactor;
    VkBlendFactor            dstColorBlendFactor;
    VkBlendOp                colorBlendOp;
    VkBlendFactor            srcAlphaBlendFactor;
    VkBlendFactor            dstAlphaBlendFactor;
    VkBlendOp                alphaBlendOp;
    VkColorComponentFlags    colorWriteMask;


    std::vector<aux::ShaderDescription> shaders;
    std::vector<VkVertexInputBindingDescription>* pVertexInputBindings;
    std::vector<VkVertexInputAttributeDescription>* pVertexInputAttributes;

    PipelineCI():
        primitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST), // 三角形List （不是Fan)
        cullMode(VK_CULL_MODE_NONE), // 没有cull
        depthWriteEnable(VK_FALSE), // 无深度写测试， 缺省， 为快速
        depthTestEnable(VK_FALSE),  // 无深度测试
        depthCompareOp(VK_COMPARE_OP_LESS_OR_EQUAL),  // 深度测试比较 函数： <= （如果Enable的话）
        rasterizationSamples(VK_SAMPLE_COUNT_1_BIT), // 1点采样
        blendEnable(VK_FALSE),      // 无blend
        srcColorBlendFactor(VK_BLEND_FACTOR_SRC_ALPHA),  // RGB的blend参数方程：  结果= a *Src + (1-a)*Dst
        dstColorBlendFactor(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA),
        colorBlendOp(VK_BLEND_OP_ADD),                   // 彩色blend: add
        srcAlphaBlendFactor(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA),  // ToDo: Alpha的blend参数方程：  
        dstAlphaBlendFactor(VK_BLEND_FACTOR_ZERO),
        alphaBlendOp(VK_BLEND_OP_ADD),    
        shaders{},
        pVertexInputBindings(nullptr),
        pVertexInputAttributes(nullptr)
    {
    }
};

class Pipeline
{
    static VkPipelineCache* m_pPipelineCache;

    VkPipeline m_pipeline;
    aux::PipelineLayout& m_pipelineLayout;
    VkRenderPass& m_renderPass;
    std::vector<aux::ShaderDescription> m_shaderList;
    PipelineCI& m_auxCI;
public:
    explicit Pipeline(aux::PipelineLayout& pipelineLayout, VkRenderPass& renderPass, PipelineCI &pipelineCI);
    ~Pipeline();
    void bindToGraphic(VkCommandBuffer& cmdBuf);
    void setShaderStages(std::vector<aux::ShaderDescription> &shaders);
    VkPipeline get() { return m_pipeline; }
    VkPipeline& getR() { return m_pipeline; }
    static void setCache(VkPipelineCache *cache) {
        Pipeline::m_pPipelineCache = cache;
    }

    static VkPipelineCache *getCache() {
        Assert(Pipeline::m_pPipelineCache != nullptr, "initialize first!");
        return Pipeline::m_pPipelineCache;
    }
private:
};

}
