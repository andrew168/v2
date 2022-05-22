#pragma once
#include "..\vk.h"

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
    ~RenderPass();
    void begin(VkCommandBuffer* pCmdBuf, aux::Framebuffer* auxFramebuffer, VkClearColorValue color = { 0.0f, 0.0f, 0.0f, 1.0f });
    void end();
    VkRenderPass* get() { return m_pRenderPass; }
private:
    void createRenderPass();
};

}
