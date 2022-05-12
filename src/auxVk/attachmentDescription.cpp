#include "auxVk.h"
#include "attachmentDescription.h"

namespace aux
{
AttachmentDescription::AttachmentDescription(aux::Image& image) :
    m_pImage(&image),
    m_format(image.getFormat()),
    m_AttachmentDescription{}
{
    // Color attachment
    m_AttachmentDescription.format = m_format;
    m_AttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    m_AttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    m_AttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    m_AttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    m_AttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    m_AttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_AttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

AttachmentDescription::~AttachmentDescription()
{
}
}
