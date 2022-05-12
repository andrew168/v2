#pragma once
#include "..\vk.h"
#include "image.h"

namespace aux
{
class RenderPass
{
    VkFormat m_format;
    VkRenderPass m_renderPass;
    VkAttachmentReference m_colorReference;
    VkSubpassDescription m_subpassDescription{};
public:
    explicit RenderPass(aux::Image& image);
    VkRenderPass get() { return m_renderPass; }
private:
    void createAttachmentReference();
    void createSubpass();
    void createRenderPass();
};

}
