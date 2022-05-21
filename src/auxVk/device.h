#pragma once
#include "..\vk.h"

namespace aux
{
#define Assert(condition, msg) {if (condition) {printf(msg);}}

class Device {
    static VkDevice* m_pDevice;
    static vks::VulkanDevice* m_vksDevice;
public:
    static VkDevice* get() {
        if (m_pDevice == nullptr) {
            Assert(0, "setup first");
        }

        return m_pDevice;
    }

    static const VkDevice& getR() {
        if (m_pDevice == nullptr) {
            Assert(0, "setup first");
        }

        return *m_pDevice;
    }

    static vks::VulkanDevice* getVksDevice() {
        if (m_vksDevice == nullptr) {
            Assert(0, "setup first");
        }
        return m_vksDevice;
    }

    static void set(VkDevice* device)
    {
        m_pDevice = device;
    }

    static void setVksDevice(vks::VulkanDevice* device)
    {
        m_vksDevice = device;
    }
};
}
