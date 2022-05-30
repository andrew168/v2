#include "auxVk.h"
#include "attachmentDescription.h"

namespace aux
{
RenderPass::RenderPass(VkRenderPass &renderpass) :
    m_pRenderPass(&renderpass),
    m_pImage(nullptr),
    m_format(VK_FORMAT_UNDEFINED),
    m_pRenderPassBeginInfo(nullptr),
    isVK(true)
{
}

RenderPass::RenderPass(aux::Image& image) :
    m_pImage(&image),
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

    aux::AttachmentDescription auxAttDesc(*m_pImage);
    aux::SubpassDescription auxSubpassDescription(*m_pImage);

    // Create the actual renderpass
    VkRenderPassCreateInfo renderPassCI{};
    renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCI.attachmentCount = 1;
    renderPassCI.pAttachments = auxAttDesc.get();
    renderPassCI.subpassCount = 1;
    renderPassCI.pSubpasses = auxSubpassDescription.get();
    renderPassCI.dependencyCount = 2;
    renderPassCI.pDependencies = dependencies.data();
    m_pRenderPass = new VkRenderPass();
    VK_CHECK_RESULT(vkCreateRenderPass(Device::getR(), &renderPassCI, nullptr, m_pRenderPass));
}

void RenderPass::begin(VkCommandBuffer& cmdBuf,
    VkRenderPassBeginInfo &beginInfo)
{
    m_rCurrentCB = &cmdBuf;
    vkCmdBeginRenderPass(cmdBuf, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}


void RenderPass::fillBI(VkRenderPassBeginInfo& bi, 
    uint32_t width,
    uint32_t height,
    uint32_t clearValueCount,
    const VkClearValue* pClearValues)
{
    bi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    bi.renderPass = *m_pRenderPass;
    bi.renderArea.extent.width = width;
    bi.renderArea.extent.height = height;
    bi.clearValueCount = clearValueCount;
    bi.pClearValues = pClearValues;
}

void RenderPass::begin(VkCommandBuffer *pCmdBuf, 
    aux::Framebuffer *auxFramebuffer, 
    VkClearColorValue color)
{
    m_rCurrentCB = pCmdBuf;
    VkClearValue clearValues[1];
    clearValues[0].color = { color };    
    if (m_pRenderPassBeginInfo == nullptr) {
        m_pRenderPassBeginInfo = new VkRenderPassBeginInfo{};
        m_pRenderPassBeginInfo->sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        m_pRenderPassBeginInfo->renderPass = *m_pRenderPass;
        m_pRenderPassBeginInfo->renderArea.extent.width = m_pImage->getWidth();
        m_pRenderPassBeginInfo->renderArea.extent.height = m_pImage->getHeight();
        m_pRenderPassBeginInfo->framebuffer = *(auxFramebuffer->get());
    }

    m_pRenderPassBeginInfo->clearValueCount = 1;
    m_pRenderPassBeginInfo->pClearValues = clearValues;

    vkCmdBeginRenderPass(*pCmdBuf, m_pRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::end()
{
    if (!m_rCurrentCB) {
        Assert(0, "Must Begin before end!");
    }

    vkCmdEndRenderPass(*m_rCurrentCB);
    m_rCurrentCB = nullptr;
}

RenderPass::~RenderPass()
{
    if (!isVK) {
        delete m_pRenderPassBeginInfo;
        m_pRenderPassBeginInfo = nullptr;
        
        vkDestroyRenderPass(Device::getR(), *m_pRenderPass, nullptr);
        delete m_pRenderPass;
    }

    m_pRenderPass = nullptr;
}
}
