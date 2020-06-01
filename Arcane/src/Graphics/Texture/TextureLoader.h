#pragma once

namespace Arcane
{
	class VulkanAPI;
	class Texture;
	struct TextureSettings;

	class TextureLoader
	{
	public:
		static void Initialize(VulkanAPI *vulkan);

		static Texture* LoadTexture(const std::string &path, TextureSettings *settings);
	private:
		static VulkanAPI *s_Vulkan;

		static std::unordered_map<std::string, Texture*> s_TextureCache;
	};
}
