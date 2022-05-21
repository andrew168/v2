#pragma once
#include "..\vk.h"
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
        primitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST),
        cullMode(VK_CULL_MODE_NONE),
        depthWriteEnable(VK_FALSE), // 缺省不测试， 为快速
        depthTestEnable(VK_FALSE),
        rasterizationSamples(VK_SAMPLE_COUNT_1_BIT),
        blendEnable(VK_FALSE),
        srcColorBlendFactor(VK_BLEND_FACTOR_SRC_ALPHA),
        dstColorBlendFactor(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA),
        colorBlendOp(VK_BLEND_OP_ADD),
        srcAlphaBlendFactor(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA),
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
    PipelineCI& m_auxPipelineCI;
public:
    explicit Pipeline(aux::PipelineLayout& pipelineLayout, VkRenderPass& renderPass, PipelineCI &pipelineCI);
    ~Pipeline();
    void bindToGraphic(VkCommandBuffer& cmdBuf);
    void setShaderStages(std::vector<aux::ShaderDescription> &shaders);
    VkPipeline get() { return m_pipeline; }
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
