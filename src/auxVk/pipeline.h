#pragma once
#include "..\vk.h"
#include "device.h"
#include "pipelineLayout.h"
#include "renderPass.h"
#include "shaderDescription.h"

namespace aux
{
struct PipelineCI {
    VkPrimitiveTopology primitiveTopology;
    std::vector<aux::ShaderDescription> shaders;
    PipelineCI():
        primitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST),
        shaders{}
    {
    }
};

class Pipeline
{
    static VkPipelineCache* m_pPipelineCache;

    VkPipeline m_pipeline;
    aux::PipelineLayout& m_pipelineLayout;
    aux::RenderPass& m_renderPass;
    std::vector<aux::ShaderDescription> m_shaderList;
    PipelineCI& m_auxPipelineCI;
public:
    explicit Pipeline(aux::PipelineLayout& pipelineLayout, aux::RenderPass& renderPass, PipelineCI &pipelineCI);
    ~Pipeline();
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
