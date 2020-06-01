#pragma once

namespace Arcane
{
	class VulkanAPI;

	// TODO: Index buffer shouldn't use 32 bit indices when it is not needed
	class IndexBuffer
	{
	public:
		IndexBuffer(const VulkanAPI *const vulkan, const uint32_t *data, size_t amount);
		~IndexBuffer();

		void Bind(VkCommandBuffer &commandBuffer);

		inline uint32_t GetCount() { return m_Count; }
	private:
		void LoadData(const uint32_t *data, size_t amount);
	private:
		const VulkanAPI *const m_Vulkan;

		uint32_t m_Count;
		VkDeviceMemory m_IndexBufferMemory;
		VkBuffer m_IndexBuffer;
	};
}
