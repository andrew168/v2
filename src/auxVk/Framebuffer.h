#pragma once
#include "auxVk.h"
#include "renderPass.h"

namespace aux
{
class RenderPass;


class Framebuffer {
    VkFramebuffer m_framebuffer;

public:
    Framebuffer(aux::Image& image, aux::RenderPass& auxRenderPass);
    ~Framebuffer();
    VkFramebuffer* get() { return &m_framebuffer; }
};
}

