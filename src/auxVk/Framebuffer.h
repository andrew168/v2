#pragma once
#include "auxVk.h"
#include "renderPass.h"

namespace aux
{
class RenderPass;


class Framebuffer {
    aux::RenderPass* m_pAuxRenderPass;
    VkFramebuffer m_framebuffer;

public:
    Framebuffer(aux::Image& image);
    VkFramebuffer get() { return m_framebuffer; }
};
}

