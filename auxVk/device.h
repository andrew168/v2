#pragma once
#include "..\vk-all.h"
#include "VulkanDevice.h"

namespace aux
{
#define Assert(condition, msg) {if (condition) {printf(msg);}}

class Device {
    static VkDevice* m_pDevice;
    static VkQueue* m_pQueue;
    static VkPhysicalDevice* m_pPhysicalDevice;
    static vks::VulkanDevice* m_vksDevice;
public:
    static VkDevice* get() {
        if (m_pDevice == nullptr) {
            Assert(0, "setup first");
        }

        return m_pDevice;
    }

    static const VkPhysicalDevice& getPhysicalDeviceR() {
        if (m_pPhysicalDevice == nullptr) {
            Assert(0, "setup first");
        }

        return *m_pPhysicalDevice;
    }

    static const VkDevice& getR() {
        if (m_pDevice == nullptr) {
            Assert(0, "setup first");
        }

        return *m_pDevice;
    }

    static VkQueue& getQueue() {
        if (m_pQueue == nullptr) {
            Assert(0, "setup first");
        }

        return *m_pQueue;
    }

    static vks::VulkanDevice* getVksDevice() {
        if (m_vksDevice == nullptr) {
            Assert(0, "setup first");
        }
        return m_vksDevice;
    }

    static void set(VkPhysicalDevice* pPhysicalDevice, VkDevice* device, VkQueue* queue, vks::VulkanDevice* vksDevice)
    {
        m_pPhysicalDevice = pPhysicalDevice;
        m_pDevice = device;
        m_vksDevice = vksDevice;
        m_pQueue = queue;
    }

    static bool hasStorageImage(VkFormat format);
    static bool hasLinearTiling(VkFormat format);
};
}
