#pragma once

namespace Arcane
{
	class VulkanAPI;

	class VertexBuffer
	{
	public:
		VertexBuffer(const VulkanAPI *const vulkan, const float *data, size_t amount);
		~VertexBuffer();

		void Bind(VkCommandBuffer &commandBuffer);

		inline uint32_t GetCount() { return m_Count; }
	private:
		void LoadData(const float *data, size_t amount);
	private:
		const VulkanAPI *const m_Vulkan;

		uint32_t m_Count;
		VkDeviceMemory m_VertexBufferMemory;
		VkBuffer m_VertexBuffer;
	};
}
