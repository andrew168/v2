#pragma once
#include "..\vk.h"
#include "image.h"
#include "Framebuffer.h"

namespace aux
{
class RenderPass
{
    aux::Image* m_pImage;
    VkFormat m_format;
    VkRenderPass m_renderPass;
    VkAttachmentReference m_colorReference;
    VkSubpassDescription m_subpassDescription{};
public:
    explicit RenderPass(aux::Image& image);
    ~RenderPass();
    void begin(VkCommandBuffer* pCmdBuf, aux::Framebuffer* auxFramebuffer);
    VkRenderPass get() { return m_renderPass; }
private:
    void createAttachmentReference();
    void createSubpass();
    void createRenderPass();
};

}
