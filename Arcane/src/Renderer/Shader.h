#pragma once

namespace Arcane
{
	class Shader
	{
	public:
		Shader(const VkDevice *device, const std::string &vertBinaryPath, const std::string &fragBinaryPath);
		~Shader();

		void Enable() const;
		void Disable() const;

		inline const std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStages() { return m_ShaderStages; }
	private:
		void Init();

		VkShaderModule CreateShaderModule(const std::string &shaderCode);
	private:
		const VkDevice *m_Device;

		const std::string m_VertexBinaryPath, m_FragBinaryPath; // TODO: Should probably be removed from release builds
		VkShaderModule m_VertexShaderModule, m_FragmentShaderModule;
		std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
	};
}
