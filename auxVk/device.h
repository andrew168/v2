#pragma once
#include "..\vk-all.h"
#include "VulkanExampleBase.h"
#include "VulkanTexture.hpp"
#include "VulkanglTFModel.h"
#include "VulkanUtils.hpp"

namespace aux
{
#define Assert(condition, msg) {if (condition) {printf(msg);}}

class Device {
    static VkDevice* m_pDevice;
    static VkQueue* m_pQueue;
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

    static void set(VkDevice* device, VkQueue* queue, vks::VulkanDevice* vksDevice)
    {
        m_pDevice = device;
        m_vksDevice = vksDevice;
        m_pQueue = queue;
    }
};
}
