#include "auxVk.h"
#include "shaderStage.h"

namespace aux
{
VkPipelineShaderStageCreateInfo ShaderStage::loadShader(VkDevice device, std::string filename, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo shaderStage{};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = stage;
	shaderStage.pName = "main";
	std::ifstream ifs;
	
	ifs.open(filename, std::ios::binary | std::ios::in | std::ios::ate);

	if (!ifs.is_open())
	{
		ifs.open("./../data/shaders/" + filename, std::ios::binary | std::ios::in | std::ios::ate);
	}

	if (ifs.is_open()) {
		size_t size = ifs.tellg();
		ifs.seekg(0, std::ios::beg);
		char* shaderCode = new char[size];
		ifs.read(shaderCode, size);
		ifs.close();
		assert(size > 0);
		VkShaderModuleCreateInfo moduleCreateInfo{};
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.codeSize = size;
		moduleCreateInfo.pCode = (uint32_t*)shaderCode;
		vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderStage.module);
		delete[] shaderCode;
	}
	else {
		std::cerr << "Error: Could not open shader file \"" << filename << "\"" << std::endl;
		shaderStage.module = VK_NULL_HANDLE;
	}

	assert(shaderStage.module != VK_NULL_HANDLE);
	return shaderStage;
}

}
