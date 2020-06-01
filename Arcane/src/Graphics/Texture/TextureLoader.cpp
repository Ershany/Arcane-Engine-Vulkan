#include "arcpch.h"
#include "TextureLoader.h"

#include "Graphics/Renderer/VulkanAPI.h"
#include "Graphics/Texture/Texture.h"

#include <stb_image.h>

namespace Arcane
{
	VulkanAPI* TextureLoader::s_Vulkan = nullptr;
	std::unordered_map<std::string, Texture*> TextureLoader::s_TextureCache;

	void TextureLoader::Initialize(VulkanAPI *vulkan)
	{
		s_Vulkan = vulkan;
		Texture::InitializeStaticData(vulkan);
	}

	Texture* TextureLoader::LoadTexture(const std::string &path, TextureSettings *settings)
	{
		ARC_ASSERT(s_Vulkan, "Texture: Can't load texture when TextureLoader is not initialized");

		auto iter = s_TextureCache.find(path);
		if (iter != s_TextureCache.end())
		{
			return iter->second;
		}

		int texWidth, texHeight, texChannels;
		stbi_uc *pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		ARC_ASSERT(pixels, "Texture: Failed to load image {0}", path);

		Texture *texture = nullptr;
		if (settings)
		{
			texture = new Texture(s_Vulkan, *settings);
		}
		else
		{
			texture = new Texture(s_Vulkan);
		}

		texture->GenerateTexture((uint32_t)texWidth, (uint32_t)texHeight, pixels);
		stbi_image_free(pixels);

		s_TextureCache.insert(std::pair<std::string, Texture*>(path, texture));
		return texture;
	}
}
