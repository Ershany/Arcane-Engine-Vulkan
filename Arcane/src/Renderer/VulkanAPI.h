#pragma once

namespace Arcane
{
	class Window;
	class Shader;

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
	};

	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class VulkanAPI
	{
	public:
		VulkanAPI(const Window *const window);
		~VulkanAPI();

		void Render();
		void InitVulkan();

		void CreateShader(const std::string &vertBinaryPath, const std::string &fragBinaryPath);
	private:
		void Cleanup();

		void CreateInstance();
		void CreateSurface();
		void SelectPhysicalDevice();
		void CreateLogicalDeviceAndQueues();
		void CreateSwapchain();
		void CreateSwapchainImageViews();
		void CreateRenderPass();
		void CreateGraphicsPipeline();
		void CreateFramebuffers();
		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateSyncObjects();

		int ScorePhysicalDeviceSuitability(const VkPhysicalDevice &device);
		bool CheckPhysicalDeviceExtensionSupport(const VkPhysicalDevice &device);
		DeviceQueueIndices FindDeviceQueueIndices(const VkPhysicalDevice &device);
		SwapchainSupportDetails QuerySwapchainSupport(const VkPhysicalDevice &device);
		VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
		VkPresentModeKHR ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
		VkExtent2D ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR &capabilities);
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

		VkSwapchainKHR m_Swapchain;
		std::vector<VkImage> m_SwapchainImages;
		std::vector<VkImageView> m_SwapchainImageViews;
		std::vector<VkFramebuffer> m_SwapchainFramebuffers;
		VkFormat m_SwapchainImageFormat;
		VkExtent2D m_SwapchainExtent;
		VkSurfaceKHR m_Surface;

		VkQueue m_GraphicsQueue;
		VkQueue m_ComputeQueue;
		VkQueue m_CopyQueue;
		VkQueue m_PresentQueue;

		VkCommandPool m_GraphicsCommandPool;
		std::vector<VkCommandBuffer> m_GraphicsCommandBuffers;

		const int MAX_FRAMES_IN_FLIGHT = 3;
		size_t m_CurrentFrame = 0;
		std::vector<VkSemaphore> m_ImageAvailableSemaphore, m_RenderFinishedSemaphore;
		std::vector<VkFence> m_InFlightFences, m_ImagesInFlight;

		// Temp Stuff - Should be abstracted in a pass system
		VkPipeline m_GraphicsPipeline;
		Shader *m_Shader;
		VkRenderPass m_RenderPass;
		VkPipelineLayout m_PipelineLayout;

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
