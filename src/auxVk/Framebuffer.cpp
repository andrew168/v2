#include "auxVk.h"
#include "Framebuffer.h"

namespace aux
{
Framebuffer::Framebuffer(aux::Image& image)
{
    m_pAuxRenderPass = new aux::RenderPass(image);

    auto rp = new aux::RenderPass(image);
    VkFramebufferCreateInfo framebufferCI{};
    framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCI.renderPass = *(m_pAuxRenderPass->get());
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
