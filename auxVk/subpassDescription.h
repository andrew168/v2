#pragma once
#include "..\vk-all.h"
#include "image.h"

namespace aux
{
class SubpassDescription: public VkSubpassDescription
{
    aux::Image& m_image;
public:
    explicit SubpassDescription(aux::Image& image);
    ~SubpassDescription();
    VkSubpassDescription *get() { return dynamic_cast<VkSubpassDescription *>(this); }
private:
};

}
