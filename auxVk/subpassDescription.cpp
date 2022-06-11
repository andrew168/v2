#include "auxVk.h"
#include "SubpassDescription.h"

namespace aux
{
SubpassDescription::SubpassDescription(aux::Image& image) :
    VkSubpassDescription(),
    m_image(image)
{
    VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

    this->pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    this->colorAttachmentCount = 1;
    this->pColorAttachments = &colorReference;
}

SubpassDescription::~SubpassDescription()
{
}
}
