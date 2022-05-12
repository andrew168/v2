#pragma once
#include "auxVk.h"

namespace aux
{
class Framebuffer {
    aux::RenderPass* m_pAuxRenderPass;
    VkFramebuffer m_framebuffer;

public:
    Framebuffer(aux::Image& image);
    VkFramebuffer get() { return m_framebuffer; }
};
}

