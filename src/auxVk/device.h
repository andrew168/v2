#pragma once
#include "..\vk.h"

namespace aux
{
#define Assert(condition, msg) {if (condition) {printf(msg);}}

class Device {
    static VkDevice* m_device;
    static vks::VulkanDevice* m_vksDevice;
public:
    static VkDevice* get() {
        if (m_device == nullptr) {
            Assert(0, "setup first");
        }

        return m_device;
    }

    static vks::VulkanDevice* getVksDevice() {
        if (m_vksDevice == nullptr) {
            Assert(0, "setup first");
        }
        return m_vksDevice;
    }

    static void set(VkDevice* device)
    {
        m_device = device;
    }

    static void setVksDevice(vks::VulkanDevice* device)
    {
        m_vksDevice = device;
    }
};
}
