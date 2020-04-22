#pragma once

class Shader
{
public:
	Shader(const std::string &vertBinary, const std::string &fragBinary);
	~Shader();

	void Enable() const;
	void Disable() const;
private:
	VkShaderModule CreateShaderModule(const std::string &shaderCode);
};
