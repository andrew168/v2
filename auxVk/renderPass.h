#pragma once
#include "..\vk-all.h"

namespace aux
{
class AttachmentDescription;
class Image;
class Framebuffer;

class RenderPass
{
    aux::Image* m_pImage;
    VkFormat m_format;
    VkRenderPass* m_pRenderPass;
    VkRenderPassBeginInfo* m_pRenderPassBeginInfo;
    VkCommandBuffer* m_rCurrentCB = nullptr;  // refereence， 只是引用，不是自己new的，不能delete
    bool isVK = false;
public:
    explicit RenderPass(aux::Image& image);
    explicit RenderPass(VkRenderPass& renderpass);
    ~RenderPass();
    VkRenderPassBeginInfo bi(uint32_t width,
        uint32_t height,
        uint32_t clearValueCount = 0,
        const VkClearValue* pClearValues = nullptr);
    
    void begin(VkCommandBuffer* pCmdBuf, aux::Framebuffer* auxFramebuffer, VkClearColorValue color = { 0.0f, 0.0f, 0.0f, 1.0f });
    void begin(VkCommandBuffer& cmdBuf, VkRenderPassBeginInfo& beginInfo);
    void end();
    VkRenderPass* get() { return m_pRenderPass; }
private:
    void createRenderPass();
};

}
