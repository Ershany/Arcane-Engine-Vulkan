#pragma once

namespace Arcane
{
	class VulkanAPI;
	class Shader;

	class ShaderLoader
	{
	public:
		static void Initialize(VulkanAPI *vulkan);

		static Shader* LoadShader(const std::string &vertPath, const std::string &fragPath);
	private:
		static VulkanAPI *s_Vulkan;

		static std::unordered_map<size_t, Shader*> s_ShaderCache;
		static std::hash<std::string> s_Hasher;
	};
}
