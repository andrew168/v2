﻿#include "auxVk.h"
#include "attachmentDescription.h"

namespace aux
{
RenderPass::RenderPass(aux::Image& image) :
    m_image(image),
    m_format(image.getFormat()),
    m_pRenderPassBeginInfo(nullptr)
{
    createRenderPass();
}

void RenderPass::createRenderPass()
{
    // Use subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies;
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    aux::AttachmentDescription auxAttDesc(m_image);
    aux::SubpassDescription auxSubpassDescription(m_image);

    // Create the actual renderpass
    VkRenderPassCreateInfo renderPassCI{};
    renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCI.attachmentCount = 1;
    renderPassCI.pAttachments = auxAttDesc.get();
    renderPassCI.subpassCount = 1;
    renderPassCI.pSubpasses = auxSubpassDescription.get();
    renderPassCI.dependencyCount = 2;
    renderPassCI.pDependencies = dependencies.data();

    VK_CHECK_RESULT(vkCreateRenderPass(*aux::Device::get(), &renderPassCI, nullptr, &m_renderPass));
}

void RenderPass::begin(VkCommandBuffer *pCmdBuf, aux::Framebuffer *auxFramebuffer, VkClearColorValue color)
{
    VkClearValue clearValues[1];
    clearValues[0].color = { color };    
    if (m_pRenderPassBeginInfo == nullptr) {
        m_pRenderPassBeginInfo = new VkRenderPassBeginInfo{};
        m_pRenderPassBeginInfo->sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        m_pRenderPassBeginInfo->renderPass = m_renderPass;
        m_pRenderPassBeginInfo->renderArea.extent.width = m_image.getWidth();
        m_pRenderPassBeginInfo->renderArea.extent.height = m_image.getHeight();
        m_pRenderPassBeginInfo->framebuffer = *(auxFramebuffer->get());
    }

    m_pRenderPassBeginInfo->clearValueCount = 1;
    m_pRenderPassBeginInfo->pClearValues = clearValues;

    vkCmdBeginRenderPass(*pCmdBuf, m_pRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

RenderPass::~RenderPass()
{
    delete m_pRenderPassBeginInfo;
    m_pRenderPassBeginInfo = nullptr;
    vkDestroyRenderPass(*(aux::Device::get()), m_renderPass, nullptr);
}
}
