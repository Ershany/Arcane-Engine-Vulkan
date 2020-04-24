#pragma once

namespace Arcane
{
	class Shader
	{
	public:
		Shader(const std::string &vertBinaryPath, const std::string &fragBinaryPath);
		~Shader();

		void Enable() const;
		void Disable() const;
	private:
		VkShaderModule CreateShaderModule(const std::string &shaderCode);
	};
}
