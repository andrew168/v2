#pragma once
#include "..\vk-all.h"
#include "ktx.h"
#include "..\base\VulkanTexture.h"

namespace aux
{

struct ImageCI : public VkImageCreateInfo
{
    void* m_pData;  // 图像的数据，从文件读出来的
    uint32_t m_dataSize;

    bool isCubemap;
    VkComponentMapping* pComponentMapping = nullptr;
    ImageCI(VkFormat _format = VK_FORMAT_R32G32B32A32_SFLOAT,
        int32_t _width = 1,
        int32_t _height = 1,
        int32_t _mipLevels = 1,
        int32_t _arrayLayers = 1,
        void *pData = nullptr,
        uint32_t dataSize = 0) :
        m_pData(pData),
        m_dataSize(dataSize),
        VkImageCreateInfo()
    {
        sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageType = VK_IMAGE_TYPE_2D;
        format = _format;
        extent.width = _width;
        extent.height = _height;
        extent.depth = 1;
        mipLevels = _mipLevels;
        arrayLayers = _arrayLayers;
        samples = VK_SAMPLE_COUNT_1_BIT; // 1个sample 每pixel 
        tiling = VK_IMAGE_TILING_OPTIMAL;
        sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        isCubemap = false;
    }
};

class Image {
    uint32_t m_width;
    uint32_t m_height;
    VkFormat m_format;
    uint32_t m_mipLevels;
    uint32_t m_arrayLayers;
    bool m_isCubemap;
    VkImage m_image;
    VkDevice m_device;
    VkDeviceMemory m_deviceMemory;
    VkImageView m_view;
    VkSampler m_sampler;
    VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkDescriptorImageInfo m_descriptor;
public:
    // 顺序： ctor, dtor, static, .... get/set
    Image();
    Image(ImageCI& ci);
    Image(std::string filename,
        VkFormat format,
        VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
        VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    void loadFromFile(
        std::string filename,
        VkFormat format,
        VkQueue copyQueue,
        VkImageUsageFlags imageUsageFlags,
        VkImageLayout imageLayout);

    void init(
        ImageCI&        ci, 
        VkImageLayout   imageLayout, 
        ktxTexture*     ktxTexture = nullptr);
    ktxResult loadKTXFile(
        std::string     filename, 
        ktxTexture**    target);
    void copyData(ImageCI& ci,
        ktxTexture&     ktxTexture,
        VkImageLayout   imageLayout,
        std::vector<VkBufferImageCopy>* pRegions = nullptr);

    void changeLayout(VkCommandBuffer& cmdbuffer,
        VkImageLayout newImageLayout,
        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    
    static void copyOneMip2Cube(
        VkCommandBuffer& cmdBuf,
        aux::Image& auxImage,
        VkExtent3D& region,
        aux::Image& cube,
        uint32_t arrayLayers,
        uint32_t mipLevels);
    static void calRegions(std::vector<VkBufferImageCopy>& bufferCopyRegions, ktxTexture* ktxTexture);

    VkDescriptorImageInfo* getDescriptor() { return &m_descriptor; }
    VkDescriptorImageInfo& getDescriptorR() { return m_descriptor; }
    VkFormat getFormat() { return m_format; }
    VkImage getImage() { Assert(0, "ToDo: TBD");  return m_image; }
    VkImage& getImageR() { return m_image; }
    VkDeviceMemory getDeviceMemory() { return m_deviceMemory; }
    VkImageView getView() { return m_view; }
    VkImageView* getViewP() { return &m_view; }
    uint32_t getMipLevels() { return m_mipLevels; }
    uint32_t getArrayLayers() { return m_arrayLayers; }
    VkSampler getSampler() { return m_sampler; }
    uint32_t getWidth() { return m_width; }
    uint32_t getHeight() { return m_height; }
    void toVKS(vks::Texture& vks);
private:
    void allocMemory(ImageCI& ci);
    void createImageView(ImageCI& ci);
    void createSampler(ImageCI& ci);
    void copyToStage(
        ImageCI& ci, 
        ktxTexture& ktxTexture);
    void stageToImage(
        ImageCI& ci, 
        ktxTexture& ktxTexture, 
        VkBuffer& stagingBuffer);

    void loadToStage(
        ktxTexture** ppKtxTexture,
        std::string filename,
        VkFormat format,
        VkQueue copyQueue,
        VkImageUsageFlags imageUsageFlags,
        VkImageLayout imageLayout);
    void fromStageToImage(
        VkFormat format,
        VkImageUsageFlags imageUsageFlags,
        VkImageLayout imageLayout,
        ktxTexture* ktxTexture, 
        ImageCI& ci);
    void Image::init9(VkFormat format,
        ImageCI& ci);

    void createImage(ImageCI& ci);
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

};
}
