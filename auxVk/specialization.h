#pragma once
#include "..\vk-all.h"

namespace aux
{
class Specialization
{
public:
    inline static VkSpecializationInfo info(
		uint32_t mapEntryCount, 
		const VkSpecializationMapEntry* mapEntries, 
		size_t dataSize, 
		const void* data)
	{
		VkSpecializationInfo specializationInfo{};
		specializationInfo.mapEntryCount = mapEntryCount;
		specializationInfo.pMapEntries = mapEntries;
		specializationInfo.dataSize = dataSize;
		specializationInfo.pData = data;
		return specializationInfo;
	}

	/** @brief Initialize a specialization constant info structure to pass to a shader stage */
	inline static VkSpecializationInfo info(
		const std::vector<VkSpecializationMapEntry>& mapEntries, 
		size_t dataSize, 
		const void* data)
	{
		VkSpecializationInfo specializationInfo{};
		specializationInfo.mapEntryCount = static_cast<uint32_t>(mapEntries.size());
		specializationInfo.pMapEntries = mapEntries.data();
		specializationInfo.dataSize = dataSize;
		specializationInfo.pData = data;
		return specializationInfo;
	}

	inline static VkSpecializationMapEntry mapEntry(
		uint32_t constantID,
		uint32_t offset,
		size_t size)
	{
		VkSpecializationMapEntry specializationMapEntry{};
		specializationMapEntry.constantID = constantID;
		specializationMapEntry.offset = offset;
		specializationMapEntry.size = size;
		return specializationMapEntry;
	}

};

}
