#pragma once
#include "..\vk-all.h"
#include "image.h"

namespace aux
{
class AttachmentDescription: public VkAttachmentDescription
{
    aux::Image& m_image;
    VkFormat m_format;
public:
    explicit AttachmentDescription(aux::Image& image);
    ~AttachmentDescription();
    VkAttachmentDescription *get() { return dynamic_cast<VkAttachmentDescription *>(this); }
private:
};

}
