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

	// Temporary
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 colour;
		glm::vec2 uv;

		// Describes vertex buffers
		static VkVertexInputBindingDescription GetBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription = {};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // This is where you can use instance rendering

			return bindingDescription;
		}

		// Describes attributes, allows for different variable locations to be defined in different vertex buffers
		static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescription()
		{
			std::array<VkVertexInputAttributeDescription, 3> attributeDescription{};
			
			attributeDescription[0].binding = 0;
			attributeDescription[0].location = 0;
			attributeDescription[0].offset = offsetof(Vertex, pos);
			attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;

			attributeDescription[1].binding = 0;
			attributeDescription[1].location = 1;
			attributeDescription[1].offset = offsetof(Vertex, colour);
			attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;

			attributeDescription[2].binding = 0;
			attributeDescription[2].location = 2;
			attributeDescription[2].offset = offsetof(Vertex, uv);
			attributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;

			return attributeDescription;
		}
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

		// Helpers
		Shader* CreateShader(const std::string &vertBinaryPath, const std::string &fragBinaryPath);
		void CreateBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkSharingMode sharingMode, VkBuffer *outBuffer, VkDeviceMemory *outBufferMemory);
		void CreateImage2D(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkSharingMode sharingMode,
							VkImage *outImage, VkDeviceMemory *outTextureMemory);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer destBuffer, VkDeviceSize size);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

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
		void RecreateSwapchain();
		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void CreateUniformBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void UpdateUniformBuffer(uint32_t currSwapchainImageIndex);
		void CreateTextures();
		void CreateTextureImageViews();
		void CreateTextureSamplers();

		VkCommandBuffer BeginSingleUseCommands(VkCommandPool pool);
		void EndSingleUseCommands(VkCommandBuffer commandBuffer, VkCommandPool pool, VkQueue queue);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

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
		VkDeviceMemory m_VertexBufferMemory;
		VkBuffer m_VertexBuffer;
		VkDeviceMemory m_IndexBufferMemory;
		VkBuffer m_IndexBuffer;
		std::vector<VkBuffer> m_UniformBuffers;
		std::vector<VkDeviceMemory> m_UniformBuffersMemory;
		VkImage m_TextureImage;
		VkDeviceMemory m_TextureImageMemory;
		VkImageView m_TextureImageView;
		VkSampler m_GenericTextureSampler;
		const std::vector<Vertex> vertices = {
			{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // Bottom Left
			{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // Bottom Right
			{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // Top Right
			{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // Top left

			{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
			{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
			{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
			{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}
		};
		const std::vector<uint16_t> indices =
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