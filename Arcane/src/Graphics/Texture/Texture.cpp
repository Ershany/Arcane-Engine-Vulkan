#include "arcpch.h"
#include "Texture.h"

#include "Graphics/Renderer/VulkanAPI.h"

Arcane::Texture::Texture(const VulkanAPI *const vulkan, const TextureSettings & settings)
	: m_Vulkan(vulkan), m_TextureSettings(settings), m_Width(0), m_Height(0)
{

}

Arcane::Texture::~Texture()
{
	vkDestroyImage(*m_Vulkan->GetDevice(), m_TextureImage, nullptr);
	vkFreeMemory(*m_Vulkan->GetDevice(), m_TextureImageMemory, nullptr);
	vkDestroyImageView(*m_Vulkan->GetDevice(), m_TextureImageView, nullptr);
}

void Arcane::Texture::GenerateTexture(uint32_t width, uint32_t height, const void *data)
{
	m_Width = width;
	m_Height = height;

	VkDeviceSize imageSize = static_cast<uint64_t>(m_Width) * static_cast<uint64_t>(m_Height) * 4;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	m_Vulkan->CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_SHARING_MODE_EXCLUSIVE, &stagingBuffer, &stagingMemory);

	void *pointerToMem = nullptr;
	vkMapMemory(*m_Vulkan->GetDevice(), stagingMemory, 0, imageSize, 0, &pointerToMem);
	memcpy(pointerToMem, data, static_cast<size_t>(imageSize));
	vkUnmapMemory(*m_Vulkan->GetDevice(), stagingMemory);

	m_Vulkan->CreateImage2D(m_Width, m_Height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SHARING_MODE_EXCLUSIVE, &m_TextureImage, &m_TextureImageMemory);

	// TODO: These should all be recorded by a single command buffer, instead of each function synchronously submitting its own command buffer
	// Make a function SetupCommandBuffer() & FlushSetupCommandBuffer() or something and record all of these actions into one command buffer
	m_Vulkan->TransitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL); // CreateImage2D sets the layout to VK_IMAGE_LAYOUT_UNDEFINED
	m_Vulkan->CopyBufferToImage(stagingBuffer, m_TextureImage, static_cast<uint32_t>(m_Width), static_cast<uint32_t>(m_Height));
	m_Vulkan->TransitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(*m_Vulkan->GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(*m_Vulkan->GetDevice(), stagingMemory, nullptr);

	m_TextureImageView = m_Vulkan->CreateImageView(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}
