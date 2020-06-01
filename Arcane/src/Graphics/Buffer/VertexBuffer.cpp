#include "arcpch.h"
#include "VertexBuffer.h"

#include "Graphics/Renderer/VulkanAPI.h"

namespace Arcane
{
	VertexBuffer::VertexBuffer(const VulkanAPI *const vulkan, const float *data, size_t amount) : m_Vulkan(vulkan), m_Count(static_cast<uint32_t>(amount))
	{
		LoadData(data, amount);
	}

	VertexBuffer::~VertexBuffer()
	{
		vkDestroyBuffer(*m_Vulkan->GetDevice(), m_VertexBuffer, nullptr);
		vkFreeMemory(*m_Vulkan->GetDevice(), m_VertexBufferMemory, nullptr);
	}

	void VertexBuffer::Bind(VkCommandBuffer & commandBuffer)
	{
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_VertexBuffer, offsets);
	}

	void VertexBuffer::LoadData(const float *data, size_t amount)
	{
		ARC_ASSERT(data, "VertexBuffer: Failed to initialize because no data was provided");

		VkDeviceSize bufferSize = sizeof(data[0]) * amount;
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		// HOST_COHERENT_BIT guarantees the driver completes the memory transfer operation for the VkMapMemory operation before the next VkQueueSubmit call
		m_Vulkan->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_SHARING_MODE_CONCURRENT, &stagingBuffer, &stagingBufferMemory);

		void *pointerToMem;
		vkMapMemory(*m_Vulkan->GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &pointerToMem);
		memcpy(pointerToMem, data, static_cast<size_t>(bufferSize));
		vkUnmapMemory(*m_Vulkan->GetDevice(), stagingBufferMemory);

		m_Vulkan->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SHARING_MODE_CONCURRENT, &m_VertexBuffer, &m_VertexBufferMemory);

		m_Vulkan->CopyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);
		vkDestroyBuffer(*m_Vulkan->GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(*m_Vulkan->GetDevice(), stagingBufferMemory, nullptr);
	}
}
