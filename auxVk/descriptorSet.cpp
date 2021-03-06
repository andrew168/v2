#include "..\util\assert.h"
#include "descriptorSet.h"
#include "describe.h"
#include "pipeline.h"

namespace aux
{
DescriptorSet::DescriptorSet(VkDescriptorSet& ds):
	m_pDescriptorSet(&ds),
	m_pDescriptorPool(nullptr),
	m_isVK(true)
{
}
DescriptorSet::DescriptorSet(VkDescriptorPool& pool, VkDescriptorSetLayout& dsl):
	m_pDescriptorPool(&pool),
	m_rDSL(&dsl)
{
	VkDescriptorSetAllocateInfo allocInfo = ai(*m_pDescriptorPool, m_rDSL, 1);
	m_pDescriptorSet = new VkDescriptorSet(); // ToDo: delete
	VK_CHECK_RESULT(vkAllocateDescriptorSets(Device::getR(), &allocInfo, m_pDescriptorSet));
}

void DescriptorSet::write(std::vector<Descriptor> descs)
{
	DEPRECIATEDBY("updateW");
	std::vector<VkWriteDescriptorSet> computeWriteDescriptorSets(descs.size());
	for (uint32_t i = 0; i < descs.size(); i++) 
	{
		auto& item = descs[i];
		Describe::any(computeWriteDescriptorSets[i], item.descriptorType, *m_pDescriptorSet, item.dstBinding, item.pImageInfo);
	}
	vkUpdateDescriptorSets(Device::getR(), static_cast<uint32_t>(computeWriteDescriptorSets.size()), computeWriteDescriptorSets.data(), 0, NULL);
}

DescriptorSet::DescriptorSet(DescriptorSetCI &ci)
{
	TODO("pool: move out"); // 应该把pool挪出去
	const VkDevice& device = Device::getR();

	VkDescriptorPoolSize poolSize = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 };

	VkDescriptorPoolCreateInfo descriptorPoolCI{};
	descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCI.poolSizeCount = 1;
	descriptorPoolCI.pPoolSizes = &poolSize;
	descriptorPoolCI.maxSets = 2;
	m_pDescriptorPool = new VkDescriptorPool();
	VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCI, nullptr, m_pDescriptorPool));

	m_pDescriptorSet = new VkDescriptorSet();
	DescriptorSet::allocate(*m_pDescriptorSet, *m_pDescriptorPool, ci.pSetLayouts);
	
	VkWriteDescriptorSet writeDescriptorSet{};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.dstSet = *m_pDescriptorSet;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.pImageInfo = ci.pImageInfo ;
	vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
}

void DescriptorSet::bindToGraphics(VkCommandBuffer &cmdBuf,
		VkPipelineLayout layout,
		uint32_t dynamicOffsetCount,
		const uint32_t* pDynamicOffsets) 
{
	vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, 
		layout, 0, 1, get(), dynamicOffsetCount, pDynamicOffsets);
}

void DescriptorSet::bind(VkCommandBuffer& cmdBuf,
	PipelineBase& pipeline,
	uint32_t dynamicOffsetCount,
	const uint32_t* pDynamicOffsets)
{
	VkPipelineBindPoint bp = pipeline.isGraphics()? VK_PIPELINE_BIND_POINT_GRAPHICS :
		VK_PIPELINE_BIND_POINT_COMPUTE;
	vkCmdBindDescriptorSets(cmdBuf, bp, *(pipeline.getLayout()),
		0, 1, get(), dynamicOffsetCount, pDynamicOffsets);
}

void DescriptorSet::bindToGraphics(const std::vector<VkDescriptorSet> &sets, 
	VkCommandBuffer& cmdBuf,
	VkPipelineLayout &layout,
	uint32_t dynamicOffsetCount,
	const uint32_t* pDynamicOffsets)
{
	vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
		layout, 0, static_cast<uint32_t>(sets.size()), sets.data(), dynamicOffsetCount, pDynamicOffsets);
}

// 直接allocate，免除ai，使用更方便
void DescriptorSet::allocate(VkDescriptorSet& dSet, 
	const VkDescriptorPool& pool, 
	const VkDescriptorSetLayout* pLayout) 
{
	DEPRECIATED(); //ToDo:被DS类的 ctor取代
	VkDescriptorSetAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	ai.descriptorPool = pool;
	ai.pSetLayouts = pLayout;
	ai.descriptorSetCount = 1;
	VK_CHECK_RESULT(vkAllocateDescriptorSets(Device::getR(), &ai, &dSet));
}

/*updateW: W-for WriteDescriptorSet
*/
void DescriptorSet::updateW(std::vector<VkWriteDescriptorSet> sets)
{
	DEPRECIATEDBY("non-static!");
	vkUpdateDescriptorSets(Device::getR(),
		static_cast<uint32_t>(sets.size()), sets.data(), 0, NULL);
}

void DescriptorSet::updateC(std::vector<VkCopyDescriptorSet> sets)
{
	DEPRECIATEDBY("non-static!");
	vkUpdateDescriptorSets(Device::getR(), 0, NULL,
		static_cast<uint32_t>(sets.size()), sets.data());
}

void DescriptorSet::updateW(std::vector<Descriptor> descs)
{
	std::vector<VkWriteDescriptorSet> wds(descs.size());
	for (uint32_t i = 0; i < descs.size(); i++)
	{
		auto& item = descs[i];
		Describe::any(wds[i], item.descriptorType, *m_pDescriptorSet, item.dstBinding, item.pImageInfo);
	}
	vkUpdateDescriptorSets(Device::getR(), static_cast<uint32_t>(wds.size()), wds.data(), 0, NULL);
}

DescriptorSet::~DescriptorSet()
{
	if (m_isVK) {
		// 从Vk实体转化来的， 不能delete
		return;
	}
	
	vkDestroyDescriptorPool(Device::getR(), *m_pDescriptorPool, nullptr);
	delete m_pDescriptorPool;	m_pDescriptorPool = nullptr;
	
	if (m_pDescriptorSet != nullptr) {
		delete m_pDescriptorSet;	m_pDescriptorSet = nullptr;
	}
}
}
