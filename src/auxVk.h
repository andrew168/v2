#pragma once
#include "vk2.h"

namespace aux
{
#define Assert(condition, msg) {if (condition) {printf(msg);}}

    class Device {
        static VkDevice* m_device;
        static vks::VulkanDevice* m_vksDevice;
    public:
        static VkDevice *get() {
            if (m_device == nullptr) {
                assert(0, "setup first");
            }

            return m_device;
        }

        static vks::VulkanDevice* getVksDevice() {
            if (m_vksDevice == nullptr) {
                assert(0, "setup first");
            }
            return m_vksDevice;
        }

        static void set(VkDevice *device)
        {
            m_device = device;
        }

        static void setVksDevice(vks::VulkanDevice *device)
        {
            m_vksDevice = device;
        }
    };

    class Image {
        uint32_t m_width;
        uint32_t m_height;
        VkFormat m_format;
        VkImage m_image;
        VkDevice m_device;
        VkDeviceMemory m_deviceMemory;
        VkImageView m_view;
    public:
        Image(VkFormat format, int32_t width, int32_t height);
        VkImage getImage() { return m_image; }
        VkDeviceMemory getDeviceMemory() { return m_deviceMemory; }
        VkImageView getView() { return m_view; }
    private:
        void createImage();
        void allocMemory();
        void createImageView();
    };


}
