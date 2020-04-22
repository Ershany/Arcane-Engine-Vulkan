#include "pch.h"
#include "Shader.h"

#include "Core/FileUtils.h"

Shader::Shader(const std::string &vertBinary, const std::string &fragBinary)
{
	std::string vertShaderCode = FileUtils::ReadFile("res/Shaders/simple_vert.spv");
	std::string fragShaderCode = FileUtils::ReadFile("res/Shaders/simple_frag.spv");

	VkShaderModule vertModule = CreateShaderModule(vertShaderCode);
	VkShaderModule fragModule = CreateShaderModule(fragShaderCode);
}

VkShaderModule Shader::CreateShaderModule(const std::string &shaderCode)
{
	std::vector<char> vertBuffer(shaderCode.size()); // Is this required to be 32 bit aligned? We might be able to use the std::string instead of converting

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = vertBuffer.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(vertBuffer.data());

	//VkShaderModule shaderModule;
	//vkCreateShaderModule()

	return VkShaderModule();
}
