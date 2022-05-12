#pragma once
#include "..\vk.h"
#include "image.h"

namespace aux
{
class AttachmentDescription
{
    aux::Image& m_image;
    VkFormat m_format;
    VkAttachmentDescription m_AttachmentDescription;
public:
    explicit AttachmentDescription(aux::Image& image);
    ~AttachmentDescription();
    VkAttachmentDescription *get() { return &m_AttachmentDescription; }
private:
};

}
