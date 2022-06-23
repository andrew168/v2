#pragma once
#include "..\vk-all.h"

namespace aux
{
class Memory
{
public:
	inline static VkMemoryAllocateInfo ai()
	{
		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		return memAllocInfo;
	}
};

}
