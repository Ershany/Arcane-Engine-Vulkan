#pragma once

namespace Arcane
{
	class VulkanAPI;

	struct TextureSettings
	{
		
	};

	class Texture
	{
	public:
		Texture(const VulkanAPI *const vulkan, const TextureSettings &settings = TextureSettings());
		~Texture();

		void GenerateTexture(uint32_t width, uint32_t height, const void *data = nullptr);

		inline int GetWidth() const { return m_Width; }
		inline int GetHeight() const { return m_Height; }
	private:
		const VulkanAPI *const m_Vulkan;

		const TextureSettings m_TextureSettings;
		uint32_t m_Width, m_Height;

		VkImage m_TextureImage;
		VkDeviceMemory m_TextureImageMemory;
		VkImageView m_TextureImageView;
	};
}
