#include "arcpch.h"
#include "Texture.h"

#include "Graphics/Renderer/VulkanAPI.h"

namespace Arcane
{
	VkSampler Texture::s_GenericSampler = nullptr;
	std::unordered_map<VkFormat, uint64_t> Texture::s_FormatByteSizes;

	Texture::Texture(const VulkanAPI *const vulkan, const TextureSettings &settings)
		: m_Vulkan(vulkan), m_TextureSettings(), m_Width(0), m_Height(0), m_TextureSampler(nullptr), m_TextureImage(VK_NULL_HANDLE), m_TextureImageMemory(VK_NULL_HANDLE), m_TextureImageView(VK_NULL_HANDLE)
	{
		// Compare the settings passed in to the default settings and this will tell us what texture sampler we should be using when sampling the texture
		if (m_TextureSettings.SamplerCompatible(settings))
		{
			m_TextureSampler = &s_GenericSampler;
		}
		else
		{
			CreateSampler(m_Vulkan, settings, m_TextureSampler);
		}
		m_TextureSettings = settings;
	}

	Texture::~Texture()
	{
		vkDestroyImage(*m_Vulkan->GetDevice(), m_TextureImage, nullptr);
		vkFreeMemory(*m_Vulkan->GetDevice(), m_TextureImageMemory, nullptr);
		vkDestroyImageView(*m_Vulkan->GetDevice(), m_TextureImageView, nullptr);
	}

	void Texture::GenerateTexture(uint32_t width, uint32_t height, const void *data)
	{
		ARC_ASSERT(m_TextureSettings.TextureFormat != VK_FORMAT_UNDEFINED, "Texture: Cannot create a texture without specifying the format");

		m_Width = width;
		m_Height = height;

		uint64_t pixelSize = 4;
		auto iter = s_FormatByteSizes.find(m_TextureSettings.TextureFormat);
		if (iter != s_FormatByteSizes.end())
		{
			pixelSize = iter->second;
		}
		else
		{
			ARC_ASSERT(false, "Texture: Failed to find pixel size for the texture format - {0} - Defaulting to {1} bytes per pixel", m_TextureSettings.TextureFormat, pixelSize);
		}

		VkDeviceSize imageSize = static_cast<uint64_t>(m_Width) * static_cast<uint64_t>(m_Height) * pixelSize;
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		m_Vulkan->CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_SHARING_MODE_EXCLUSIVE, &stagingBuffer, &stagingMemory);

		void *pointerToMem = nullptr;
		vkMapMemory(*m_Vulkan->GetDevice(), stagingMemory, 0, imageSize, 0, &pointerToMem);
		memcpy(pointerToMem, data, static_cast<size_t>(imageSize));
		vkUnmapMemory(*m_Vulkan->GetDevice(), stagingMemory);

		m_Vulkan->CreateImage2D(m_Width, m_Height, m_TextureSettings.TextureFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SHARING_MODE_EXCLUSIVE, &m_TextureImage, &m_TextureImageMemory);

		// TODO: These should all be recorded by a single command buffer, instead of each function synchronously submitting its own command buffer
		// Make a function SetupCommandBuffer() & FlushSetupCommandBuffer() or something and record all of these actions into one command buffer
		m_Vulkan->TransitionImageLayout(m_TextureImage, m_TextureSettings.TextureFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL); // CreateImage2D sets the layout to VK_IMAGE_LAYOUT_UNDEFINED
		m_Vulkan->CopyBufferToImage(stagingBuffer, m_TextureImage, static_cast<uint32_t>(m_Width), static_cast<uint32_t>(m_Height));
		m_Vulkan->TransitionImageLayout(m_TextureImage, m_TextureSettings.TextureFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(*m_Vulkan->GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(*m_Vulkan->GetDevice(), stagingMemory, nullptr);

		m_TextureImageView = m_Vulkan->CreateImageView(m_TextureImage, m_TextureSettings.TextureFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	void Texture::CreateSampler(const VulkanAPI *const vulkan, const TextureSettings &settings, VkSampler *outSampler)
	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.pNext = nullptr;
		samplerInfo.magFilter = settings.TextureMagnificationFilterMode;
		samplerInfo.minFilter = settings.TextureMinificationFilterMode;
		samplerInfo.addressModeU = settings.TextureWrapU;
		samplerInfo.addressModeV = settings.TextureWrapV;
		samplerInfo.addressModeW = settings.TextureWrapW;
		samplerInfo.anisotropyEnable = settings.TextureAnistropyLevel > 1.0f ? VK_TRUE : VK_FALSE;
		samplerInfo.maxAnisotropy = settings.TextureAnistropyLevel;
		samplerInfo.borderColor = settings.BorderColour;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE; // Can be true for PCF on shadow maps
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = settings.MipBias;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		VkResult result = vkCreateSampler(*vulkan->GetDevice(), &samplerInfo, nullptr, outSampler);
		ARC_ASSERT(result == VK_SUCCESS, "Vulkan: Failed to create texture sampler");
	}

	void Texture::InitializeStaticData(const VulkanAPI *const vulkan)
	{
		// Create mapping of formats and their corresponding sizes in bytes
		s_FormatByteSizes.emplace(std::pair<VkFormat, uint64_t>(VK_FORMAT_R8G8B8A8_SRGB, 4));
		s_FormatByteSizes.emplace(std::pair<VkFormat, uint64_t>(VK_FORMAT_R8G8B8A8_SINT, 4));
		s_FormatByteSizes.emplace(std::pair<VkFormat, uint64_t>(VK_FORMAT_R8G8B8A8_UINT, 4));
		s_FormatByteSizes.emplace(std::pair<VkFormat, uint64_t>(VK_FORMAT_R8G8B8A8_SNORM, 4));
		s_FormatByteSizes.emplace(std::pair<VkFormat, uint64_t>(VK_FORMAT_R8G8B8A8_UNORM, 4));

		// Create default sampler for generic texture settings
		TextureSettings defaultTextureSettings;
		CreateSampler(vulkan, defaultTextureSettings, &s_GenericSampler);
	}
}
