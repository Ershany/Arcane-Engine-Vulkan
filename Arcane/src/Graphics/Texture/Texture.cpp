#include "arcpch.h"
#include "Texture.h"

#include "Graphics/Renderer/VulkanAPI.h"

Arcane::Texture::Texture(const VulkanAPI *const vulkan, const TextureSettings & settings)
	: m_Vulkan(vulkan), m_TextureSettings(settings)
{

}

Arcane::Texture::~Texture()
{

}

void Arcane::Texture::GenerateTexture(int width, int height, const void * data)
{
	m_Width = width;
	m_Height = height;
}
