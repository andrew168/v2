#include "auxVk.h"
#include "attachmentDescription.h"

namespace aux
{
AttachmentDescription::AttachmentDescription(aux::Image& image) :
    VkAttachmentDescription(),
    m_image(image),
    m_format(image.getFormat())
{
    // Color attachment
    this->format = m_format;
    this->samples = VK_SAMPLE_COUNT_1_BIT;
    this->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    this->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    this->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    this->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    this->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    this->finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // attDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}



AttachmentDescription::~AttachmentDescription()
{
}
}
