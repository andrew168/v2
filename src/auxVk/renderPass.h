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
public:
    explicit RenderPass(aux::Image& image);
    ~RenderPass();
    void begin(VkCommandBuffer* pCmdBuf, aux::Framebuffer* auxFramebuffer);
    VkRenderPass get() { return m_renderPass; }
private:
    void createRenderPass();
};

}
