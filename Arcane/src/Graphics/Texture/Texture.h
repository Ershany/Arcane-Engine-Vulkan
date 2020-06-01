#pragma once

namespace Arcane
{
	class VulkanAPI;
	class TextureLoader;

	struct TextureSettings
	{
		VkFormat TextureFormat = VK_FORMAT_UNDEFINED; // This is where SRGB is also specified for albedo textures

		VkFilter TextureMinificationFilterMode = VK_FILTER_LINEAR; // Filtering mode when multiple texels maps to one pixel (texture is far from the camera)
		VkFilter TextureMagnificationFilterMode = VK_FILTER_LINEAR; // Filtering mode when multiple pixels map to one texel (texture is close to the camera)
		float TextureAnistropyLevel = 16.0f;

		VkBorderColor BorderColour = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		VkSamplerAddressMode TextureWrapU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		VkSamplerAddressMode TextureWrapV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		VkSamplerAddressMode TextureWrapW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		bool HasMips = true;
		float MipBias = 0.0f; // Positive = Blurrier texture, Negative = Sharper texture but aliasing is common

		bool SamplerCompatible(const TextureSettings &other) const
		{
			bool compat = this->TextureMinificationFilterMode == other.TextureMinificationFilterMode &&
				this->TextureMagnificationFilterMode == other.TextureMagnificationFilterMode &&
				this->TextureAnistropyLevel == other.TextureAnistropyLevel &&
				this->BorderColour == other.BorderColour &&
				this->TextureWrapU == other.TextureWrapU &&
				this->TextureWrapV == other.TextureWrapV &&
				this->TextureWrapW == other.TextureWrapW &&
				this->HasMips == other.HasMips &&
				this->MipBias == other.MipBias;

			return compat;
		}
	};

	class Texture
	{
		friend TextureLoader;
	public:
		Texture(const VulkanAPI *const vulkan, const TextureSettings &settings = TextureSettings());
		~Texture();

		void GenerateTexture(uint32_t width, uint32_t height, const void *data = nullptr);

		inline VkSampler GetTextureSampler() { return *m_TextureSampler; }
		inline int GetWidth() const { return m_Width; }
		inline int GetHeight() const { return m_Height; }
		inline VkImageView GetImageView() { return m_TextureImageView; }
	private:
		static void CreateSampler(const VulkanAPI *const vulkan, const TextureSettings &settings, VkSampler *outSampler);
		static void InitializeStaticData(const VulkanAPI *const vulkan);
	private:
		static VkSampler s_GenericSampler;
		static std::unordered_map<VkFormat, uint64_t> s_FormatByteSizes;

		const VulkanAPI *const m_Vulkan;
		TextureSettings m_TextureSettings;
		uint32_t m_Width, m_Height;
		VkSampler *m_TextureSampler;

		VkImage m_TextureImage;
		VkDeviceMemory m_TextureImageMemory;
		VkImageView m_TextureImageView;
	};
}
