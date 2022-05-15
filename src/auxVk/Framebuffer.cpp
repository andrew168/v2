#include "auxVk.h"
#include "Framebuffer.h"

namespace aux
{
Framebuffer::Framebuffer(aux::Image& image, aux::RenderPass& auxRenderPass)
{
    VkFramebufferCreateInfo framebufferCI{};
    framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCI.renderPass = *(auxRenderPass.get());
    framebufferCI.attachmentCount = 1;
    framebufferCI.pAttachments = image.getViewP();
    framebufferCI.width = image.getWidth();
    framebufferCI.height = image.getHeight();
    framebufferCI.layers = image.getArrayLayers();

    VK_CHECK_RESULT(vkCreateFramebuffer(*Device::get(), &framebufferCI, nullptr, &m_framebuffer));
};


Framebuffer::~Framebuffer()
{
    vkDestroyFramebuffer(*(aux::Device::get()), m_framebuffer, nullptr);
}
}
