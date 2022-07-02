#pragma once
namespace aux {
#define V2CAT(x, y, z) x##y##z
#define V2STAGE(x) V2CAT(VK_PIPELINE_STAGE_, x, _BIT)
#define V2ACCESS(x) V2CAT(VK_ACCESS_, x, _BIT)
};

#include "..\util\log.h"
#include "..\vk-all.h"
#include "device.h"
#include "describe.h"
#include "descriptor.h"
#include "descriptorPool.h"
#include "descriptorSet.h"
#include "descriptorSetLayout.h"
#include "memory.h"
#include "image.h"
#include "renderPass.h"
#include "framebuffer.h"
#include "swapChain.h"
#include "pipelineLayout.h"
#include "shaderStage.h"
#include "pipeline.h"
#include "computePipeline.h"
#include "attachmentDescription.h"
#include "subpassDescription.h"
#include "imBarrier.h"
#include "commandPool.h"
#include "commandBuffer.h"
#include "semaphore.h"
#include "semaphoreMgr.h"
#include "specialization.h"
#include "queue.h"
#include "fence.h"
#include "fenceMgr.h"
#include "buffer.h"
#include "vertexInput.h"
