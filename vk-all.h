#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <chrono>
#include <map>
#include "algorithm"

#if defined(__ANDROID__)
#define TINYGLTF_ANDROID_LOAD_FROM_ASSETS
#endif

#include <vulkan/vulkan.h>

#if defined(V2_PBR)

#if !defined(VK_FLAGS_NONE)
// Custom define for better code readability
#define VK_FLAGS_NONE 0
// Default fence timeout in nanoseconds
#define DEFAULT_FENCE_TIMEOUT 100000000000
#endif

#include "VulkanUtils.hpp"
namespace vks
{

struct Buffer : public ::Buffer
{

};
}

#endif
