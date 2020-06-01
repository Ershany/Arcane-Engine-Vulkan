#pragma once

#include "Graphics/Vertex.h"

namespace Arcane
{
	class Window;
	class Shader;
	class Texture;
	class VertexBuffer;
	class IndexBuffer;
	struct TextureSettings;

	struct DeviceQueueIndices
	{
		std::optional<uint32_t> graphicsQueue;
		std::optional<uint32_t> computeQueue;
		std::optional<uint32_t> copyQueue;

		std::optional<uint32_t> presentQueue;

		bool IsSuitable()
		{
			return graphicsQueue.has_value() && computeQueue.has_value() && copyQueue.has_value() && presentQueue.has_value();
		}

		void Reset()
		{
			graphicsQueue.reset();
			computeQueue.reset();
			copyQueue.reset();
			presentQueue.reset();
		}
	};

	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	// Temporary (alignas makes sure the variable is N byte aligned, should mimic the struct packing in the shaders)
	struct StandardMaterialUBO
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 projection;
	};

	class VulkanAPI
	{
	public:
		VulkanAPI(const Window *const window);
		~VulkanAPI();

		void Render();
		void InitVulkan();
		void InitImGui();

		// Resource Creation Helpers
		void CreateBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkSharingMode sharingMode, VkBuffer *outBuffer, VkDeviceMemory *outBufferMemory) const;
		void CreateImage2D(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkSharingMode sharingMode,
							VkImage *outImage, VkDeviceMemory *outTextureMemory) const;
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer destBuffer, VkDeviceSize size) const;
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) const;
		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const;

		// Getters
		inline const VkDevice* GetDevice() const { return &m_Device; }

		// Setters
		inline void NotifyWindowResized() { m_FramebufferResized = true; }
	private:
		void Cleanup();
		void CleanupSwapchain();

		void CreateInstance();
		void CreateSurface();
		void SelectPhysicalDevice();
		void CreateLogicalDeviceAndQueues();
		void CreateSwapchain();
		void CreateSwapchainImageViews();
		void CreateDepthResources();
		void CreateRenderPass();
		void CreateDescriptorSetLayout();
		void CreateGraphicsPipeline();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateSyncObjects();
		void CreateTemporaryResources();
		void RecreateSwapchain();
		void CreateUniformBuffers();
		void UpdateUniformBuffer(uint32_t currSwapchainImageIndex);
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateTextureSamplers();

		VkCommandBuffer BeginSingleUseCommands(VkCommandPool pool) const;
		void EndSingleUseCommands(VkCommandBuffer commandBuffer, VkCommandPool pool, VkQueue queue) const;
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

		int ScorePhysicalDeviceSuitability(const VkPhysicalDevice &device);
		bool CheckPhysicalDeviceExtensionSupport(const VkPhysicalDevice &physicalDevice);
		DeviceQueueIndices FindDeviceQueueIndices(const VkPhysicalDevice &physicalDevice);
		SwapchainSupportDetails QuerySwapchainSupport(const VkPhysicalDevice &physicalDevice);
		VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
		VkPresentModeKHR ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
		VkExtent2D ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR &capabilities);
		VkFormat FindDepthFormat();
		bool HasStencilComponent(VkFormat format);
		VkFormat FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		bool CheckValidationLayerSupport();
		std::vector<const char*> GetRequiredExtensions();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
		void SetupValidationLayers();

		// Helper Functions for Vulkan Validation Layer Initialization
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
			void *userData);
		static VkResult CreateDebugUtilsMessengerEXT(
			VkInstance instance,
			const VkDebugUtilsMessengerCreateInfoEXT *createInfo,
			VkAllocationCallbacks *allocator,
			VkDebugUtilsMessengerEXT *debugMessenger);
		static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *allocator);
	private:
		const Window *const m_Window;
		VkInstance m_Instance;
		VkPhysicalDevice m_PhysicalDevice;
		VkDevice m_Device;
		DeviceQueueIndices m_DeviceQueueIndices;

		VkSwapchainKHR m_Swapchain;
		std::vector<VkImage> m_SwapchainImages;
		std::vector<VkImageView> m_SwapchainImageViews;
		std::vector<VkFramebuffer> m_SwapchainFramebuffers;
		VkFormat m_SwapchainImageFormat;
		VkExtent2D m_SwapchainExtent;
		VkSurfaceKHR m_Surface;
		VkImage m_DepthImage;
		VkDeviceMemory m_DepthImageMemory;
		VkImageView m_DepthImageView;

		VkQueue m_GraphicsQueue;
		VkQueue m_ComputeQueue;
		VkQueue m_CopyQueue;
		VkQueue m_PresentQueue;

		VkCommandPool m_GraphicsCommandPool;
		VkCommandPool m_CopyCommandPool;
		std::vector<VkCommandBuffer> m_GraphicsCommandBuffers;

		const int MAX_FRAMES_IN_FLIGHT = 3;
		size_t m_CurrentFrame = 0;
		std::vector<VkSemaphore> m_ImageAvailableSemaphore, m_RenderFinishedSemaphore;
		std::vector<VkFence> m_InFlightFences, m_ImagesInFlight;

		bool m_FramebufferResized = false;

		// Temp Stuff - Should be abstracted
		VkDescriptorPool m_DescriptorPool;
		std::vector<VkDescriptorSet> m_DescriptorSets;
		VkDescriptorSetLayout m_DescriptorSetLayout;
		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_GraphicsPipeline;
		Shader *m_Shader;
		VkRenderPass m_RenderPass;
		VertexBuffer *m_VertexBuffer;
		IndexBuffer *m_IndexBuffer;
		std::vector<VkBuffer> m_UniformBuffers;
		std::vector<VkDeviceMemory> m_UniformBuffersMemory;
		Texture *m_Texture;
		VkSampler m_GenericTextureSampler;
		const std::vector<float> vertices = {
			-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
			0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
			-0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

			-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
			0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			0.5f, 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
			-0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
		};
		const std::vector<uint32_t> indices =
		{
			0, 2, 1, 2, 0, 3,
			4, 6, 5, 6, 4, 7
		};

		const std::vector<const char*> m_RequiredExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		const std::vector<const char*> m_ValidationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};
		VkDebugUtilsMessengerEXT m_DebugMessenger;
#ifdef ARC_DEBUG
		bool const m_EnableValidationLayers = true;
#else
		bool const m_EnableValidationLayers = false;
#endif
	};
}
