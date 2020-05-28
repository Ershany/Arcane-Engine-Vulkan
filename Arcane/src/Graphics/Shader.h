#pragma once

namespace Arcane
{
	class VulkanAPI;

	class Shader
	{
	public:
		Shader(const VulkanAPI *const vulkan, const std::string &vertBinaryPath, const std::string &fragBinaryPath);
		~Shader();

		inline const std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStages() { return m_ShaderStages; }
	private:
		void Init();

		VkShaderModule CreateShaderModule(const std::string &shaderCode);
	private:
		const VulkanAPI *const m_Vulkan;

		const std::string m_VertexBinaryPath, m_FragBinaryPath; // TODO: Should probably be removed from release builds
		VkShaderModule m_VertexShaderModule, m_FragmentShaderModule;
		std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
	};
}
