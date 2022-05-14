#pragma once
#include "..\vk.h"
#include "image.h"
#include "Framebuffer.h"

namespace aux
{
class AttachmentDescription;

class RenderPass
{
    aux::Image& m_image;
    VkFormat m_format;
    VkRenderPass m_renderPass;
    VkRenderPassBeginInfo* m_pRenderPassBeginInfo;
public:
    explicit RenderPass(aux::Image& image);
    ~RenderPass();
    void begin(VkCommandBuffer* pCmdBuf, aux::Framebuffer* auxFramebuffer, VkClearColorValue color = { 0.0f, 0.0f, 0.0f, 1.0f });
    VkRenderPass* get() { return &m_renderPass; }
private:
    void createRenderPass();
};

}
