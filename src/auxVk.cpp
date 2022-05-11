#include "auxVk.h"

namespace aux 
{
    VkDevice* Device::m_device = nullptr;
    vks::VulkanDevice* Device::m_vksDevice = nullptr;

    Image::Image(VkFormat format, int32_t width, int32_t height) :
        m_format(format),
        m_width(width),
        m_height(height),
        m_deviceMemory(nullptr),
        m_view(nullptr),
        m_mipLevels(1),
        m_arrayLayers(1),
        m_isCubemap(false)
    {
        createImage();
        allocMemory();
        VK_CHECK_RESULT(vkBindImageMemory(*(Device::get()), m_image, m_deviceMemory, 0));
        createImageView();
        createSampler();
    }

    Image::Image(VkFormat format, int32_t cubeLength, VkImageCreateFlagBits flags) :
        m_format(format),
        m_width(cubeLength),
        m_height(cubeLength),
        m_mipLevels(static_cast<uint32_t>(floor(log2(cubeLength))) + 1),
        m_arrayLayers(6),
        m_deviceMemory(nullptr),
        m_view(nullptr),
        m_isCubemap(true)
    {
        Assert(flags != VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, "unsupported type!s");
        createCubemap();        
        allocMemory();
        VK_CHECK_RESULT(vkBindImageMemory(*(Device::get()), m_image, m_deviceMemory, 0));
        createImageView();
        createSampler();
    }

    void Image::createCubemap()
    {
        VkImageCreateInfo ci = getDefaultCI();
        ci.mipLevels = m_mipLevels;
        ci.arrayLayers = m_arrayLayers;
        ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        ci.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        VK_CHECK_RESULT(vkCreateImage(*(Device::get()), &ci, nullptr, &m_image));
    }

    void Image::createImage()
    {
        VkImageCreateInfo ci = getDefaultCI();
        VK_CHECK_RESULT(vkCreateImage(*(Device::get()), &ci, nullptr, &m_image));
    }

    VkImageCreateInfo Image::getDefaultCI() {
        static VkImageCreateInfo imageCI{};
        imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCI.imageType = VK_IMAGE_TYPE_2D;
        imageCI.format = m_format;
        imageCI.extent.width = m_width;
        imageCI.extent.height = m_height;
        imageCI.extent.depth = 1;
        imageCI.mipLevels = 1;
        imageCI.arrayLayers = 1;
        imageCI.samples = VK_SAMPLE_COUNT_1_BIT; // 1个sample 每pixel 
        imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        return imageCI;
    }

    void Image::allocMemory() {
        VkMemoryRequirements memReqs;
        vkGetImageMemoryRequirements(*Device::get(), m_image, &memReqs);
        VkMemoryAllocateInfo memAllocInfo{};
        memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memAllocInfo.allocationSize = memReqs.size;
        memAllocInfo.memoryTypeIndex = Device::getVksDevice()->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(*Device::get(), &memAllocInfo, nullptr, &m_deviceMemory));
    }
   
    void Image::createImageView()
    {
        VkImageViewCreateInfo viewCI{};
        viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCI.viewType = m_isCubemap ? VK_IMAGE_VIEW_TYPE_CUBE: VK_IMAGE_VIEW_TYPE_2D;
        viewCI.format = m_format;
        viewCI.subresourceRange = {};
        viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewCI.subresourceRange.levelCount = m_mipLevels;
        viewCI.subresourceRange.layerCount = m_arrayLayers;
        viewCI.image = m_image;
        VK_CHECK_RESULT(vkCreateImageView(*Device::get(), &viewCI, nullptr, &m_view));
    }

    void Image::createSampler() {
        VkSamplerCreateInfo samplerCI{};
        samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCI.magFilter = VK_FILTER_LINEAR;
        samplerCI.minFilter = VK_FILTER_LINEAR;
        samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCI.minLod = 0.0f;
        samplerCI.maxLod = static_cast<float>(m_mipLevels);
        samplerCI.maxAnisotropy = 1.0f;
        samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        VK_CHECK_RESULT(vkCreateSampler(*Device::get(), &samplerCI, nullptr, &m_sampler));
    }

    // ---
    RenderPass::RenderPass(aux::Image& image) :
        m_format(image.getFormat())
    {
        createAttachmentReference();
        createSubpass();
        createRenderPass();
    }

    void RenderPass::createAttachmentReference()
    {
        m_colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    }

    void RenderPass::createSubpass()
    {
        VkSubpassDescription subpassDescription{};
        m_subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        m_subpassDescription.colorAttachmentCount = 1;
        m_subpassDescription.pColorAttachments = &m_colorReference;
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


        VkAttachmentDescription attDesc{};
        // Color attachment
        attDesc.format = m_format;
        attDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        attDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


        // Create the actual renderpass
        VkRenderPassCreateInfo renderPassCI{};
        renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCI.attachmentCount = 1;
        renderPassCI.pAttachments = &attDesc;
        renderPassCI.subpassCount = 1;
        renderPassCI.pSubpasses = &m_subpassDescription;
        renderPassCI.dependencyCount = 2;
        renderPassCI.pDependencies = dependencies.data();

        VK_CHECK_RESULT(vkCreateRenderPass(*aux::Device::get(), &renderPassCI, nullptr, &m_renderPass));
    }
}
