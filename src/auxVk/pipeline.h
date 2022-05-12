#pragma once
#include "..\vk.h"
#include "device.h"
#include "pipelineLayout.h"
#include "renderPass.h"

namespace aux
{
class Pipeline
{
    static VkPipelineCache* m_pPipelineCache;

    VkPipeline m_pipeline;
    aux::PipelineLayout* m_pPipelineLayout;
    aux::RenderPass* m_pRenderPass;

public:
    explicit Pipeline(aux::PipelineLayout* pPipelineLayout, aux::RenderPass* pRenderPass);
    ~Pipeline();
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
