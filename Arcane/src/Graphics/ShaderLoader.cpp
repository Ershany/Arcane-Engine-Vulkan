#include "arcpch.h"
#include "ShaderLoader.h"

#include "Graphics/Renderer/VulkanAPI.h"
#include "Graphics/Shader.h"

namespace Arcane
{
	VulkanAPI* ShaderLoader::s_Vulkan = nullptr;
	std::unordered_map<size_t, Shader*> ShaderLoader::s_ShaderCache;
	std::hash<std::string> ShaderLoader::s_Hasher;

	void ShaderLoader::Initialize(VulkanAPI *vulkan)
	{
		s_Vulkan = vulkan;
	}

	Shader* ShaderLoader::LoadShader(const std::string &vertPath, const std::string &fragPath)
	{
		ARC_ASSERT(s_Vulkan, "Shader: Can't load shader when ShaderLoader is not initialized");

		size_t hash = s_Hasher(vertPath + fragPath);
		auto iter = s_ShaderCache.find(hash);
		if (iter != s_ShaderCache.end())
		{
			return iter->second;
		}

		Shader *shader = new Shader(s_Vulkan, vertPath, fragPath);

		s_ShaderCache.insert(std::pair<size_t, Shader*>(hash, shader));
		return shader;
	}
}
