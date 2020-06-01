#include "arcpch.h"
#include "IndexBuffer.h"

#include "Graphics/Renderer/VulkanAPI.h"

namespace Arcane
{
	IndexBuffer::IndexBuffer(const VulkanAPI *const vulkan, const uint32_t *data, size_t amount) : m_Vulkan(vulkan), m_Count(static_cast<uint32_t>(amount))
	{
		LoadData(data, amount);
	}

	IndexBuffer::~IndexBuffer()
	{
		vkDestroyBuffer(*m_Vulkan->GetDevice(), m_IndexBuffer, nullptr);
		vkFreeMemory(*m_Vulkan->GetDevice(), m_IndexBufferMemory, nullptr);
	}

	void IndexBuffer::Bind(VkCommandBuffer &commandBuffer)
	{
		vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
	}

	void IndexBuffer::LoadData(const uint32_t *data, size_t amount)
	{
		ARC_ASSERT(data, "IndexBuffer: Failed to initialize because no data was provided");

		VkDeviceSize bufferSize = sizeof(data[0]) * amount;
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		// HOST_COHERENT_BIT guarantees the driver completes the memory transfer operation for the VkMapMemory operation before the next VkQueueSubmit call
		m_Vulkan->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_SHARING_MODE_CONCURRENT, &stagingBuffer, &stagingBufferMemory);

		void *pointerToMem;
		vkMapMemory(*m_Vulkan->GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &pointerToMem);
		memcpy(pointerToMem, data, static_cast<size_t>(bufferSize));
		vkUnmapMemory(*m_Vulkan->GetDevice(), stagingBufferMemory);

		m_Vulkan->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SHARING_MODE_CONCURRENT, &m_IndexBuffer, &m_IndexBufferMemory);

		m_Vulkan->CopyBuffer(stagingBuffer, m_IndexBuffer, bufferSize);
		vkDestroyBuffer(*m_Vulkan->GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(*m_Vulkan->GetDevice(), stagingBufferMemory, nullptr);
	}
}
