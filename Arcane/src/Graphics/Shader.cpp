#include "arcpch.h"
#include "Shader.h"

#include "Core/FileUtils.h"

namespace Arcane
{
	Shader::Shader(const VkDevice device, const std::string &vertBinaryPath, const std::string &fragBinaryPath) : m_Device(device), m_VertexBinaryPath(vertBinaryPath), m_FragBinaryPath(fragBinaryPath)
	{
		Init();
	}

	Shader::~Shader()
	{
		// Can be freed after the PSO is created, but then you can't create new pipeline state objects using said shader at runtime
		vkDestroyShaderModule(m_Device, m_VertexShaderModule, nullptr);
		vkDestroyShaderModule(m_Device, m_FragmentShaderModule, nullptr);
	}

	void Shader::Init()
	{
		std::string vertShaderCode = FileUtils::ReadFile(m_VertexBinaryPath);
		std::string fragShaderCode = FileUtils::ReadFile(m_FragBinaryPath);

		VkShaderModule vertModule = CreateShaderModule(vertShaderCode);
		VkShaderModule fragModule = CreateShaderModule(fragShaderCode);

		m_VertexShaderModule = vertModule;
		m_FragmentShaderModule = fragModule;

		VkPipelineShaderStageCreateInfo vertCreateInfo = {};
		vertCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertCreateInfo.module = m_VertexShaderModule;
		vertCreateInfo.pName = "main";
		// Possible optimization if you use pSpecializationInfo to set constants at pipeline creation time instead of at render time (only works for constants)

		VkPipelineShaderStageCreateInfo fragCreateInfo = {};
		fragCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragCreateInfo.module = m_FragmentShaderModule;
		fragCreateInfo.pName = "main";
		// Possible optimization if you use pSpecializationInfo to set constants at pipeline creation time instead of at render time (only works for constants)

		m_ShaderStages.reserve(2);
		m_ShaderStages.push_back(vertCreateInfo);
		m_ShaderStages.push_back(fragCreateInfo);
	}

	VkShaderModule Shader::CreateShaderModule(const std::string &shaderBinary)
	{
		std::vector<char> shaderBinaryBuffer(shaderBinary.begin(), shaderBinary.end());

		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shaderBinaryBuffer.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderBinaryBuffer.data());

		VkShaderModule shaderModule = VK_NULL_HANDLE;
		VkResult result = vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule);
		ARC_ASSERT(result == VK_SUCCESS, "Failed to create shader module");

		return shaderModule;
	}
}
