#include "arcpch.h"
#include "VulkanAPI.h"

#include "Defs.h"
#include "Core/Window.h"
#include "Core/FileUtils.h"
#include "Renderer/Shader.h"

namespace Arcane
{
	VulkanAPI::VulkanAPI(const Window *const window)
		: m_Window(window), m_Instance(VK_NULL_HANDLE), m_PhysicalDevice(VK_NULL_HANDLE), m_Device(VK_NULL_HANDLE), m_Swapchain(VK_NULL_HANDLE), m_SwapchainImageFormat(VK_FORMAT_UNDEFINED),
		m_SwapchainExtent(), m_Surface(VK_NULL_HANDLE), m_GraphicsQueue(VK_NULL_HANDLE), m_ComputeQueue(VK_NULL_HANDLE), m_CopyQueue(VK_NULL_HANDLE), m_PresentQueue(VK_NULL_HANDLE), m_GraphicsCommandPool(VK_NULL_HANDLE),
		m_DebugMessenger(VK_NULL_HANDLE)
	{
	
	}

	VulkanAPI::~VulkanAPI()
	{
		Cleanup();
	}

	void VulkanAPI::Render()
	{
		vkWaitForFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(m_Device, m_Swapchain, UINT64_MAX, m_ImageAvailableSemaphore[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapchain();
			return;
		}
		ARC_ASSERT((result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR), "Vulkan: Failed to acquire swap chain image");

		// Check if a previous frame is using this image, only needed if MAX_FRAMES_IN_FLIGHT is higher than the # of swapchain images
		if (m_ImagesInFlight[imageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(m_Device, 1, &m_ImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}
		m_ImagesInFlight[imageIndex] = m_InFlightFences[m_CurrentFrame];

		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphore[m_CurrentFrame] };
		VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphore[m_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }; // We need to wait on the semaphore at the stage where we write to the colour attachment (after pixel shader)

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_GraphicsCommandBuffers[imageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame]);
		result = vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]);
		ARC_ASSERT(result == VK_SUCCESS, "Failed to submit Vulkan draw command buffer");

		VkSwapchainKHR swapChains[] = { m_Swapchain };

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;
		
		result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized)
		{
			m_FramebufferResized = false;
			RecreateSwapchain();
		}
		else
		{
			ARC_ASSERT(result == VK_SUCCESS, "Vulkan: Failed to present the swapchain image");
		}

		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanAPI::InitVulkan()
	{
		CreateInstance();
		SetupValidationLayers();
		CreateSurface();
		SelectPhysicalDevice();
		CreateLogicalDeviceAndQueues();
		CreateSwapchain();
		CreateSwapchainImageViews();
		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateFramebuffers();
		CreateCommandPool();
		CreateVertexBuffer();
		CreateCommandBuffers();
		CreateSyncObjects();
	}

	void VulkanAPI::CreateShader(const std::string & vertBinaryPath, const std::string & fragBinaryPath)
	{
		
	}

	void VulkanAPI::CreateBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkSharingMode sharingMode, VkBuffer *outBuffer, VkDeviceMemory *outBufferMemory)
	{
		DeviceQueueIndices queueIndices = FindDeviceQueueIndices(m_PhysicalDevice);
		std::array<uint32_t, 2> allowedQueues{ queueIndices.graphicsQueue.value(), queueIndices.copyQueue.value() };

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = nullptr;
		bufferInfo.size = bufferSize;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = sharingMode;
		bufferInfo.queueFamilyIndexCount = static_cast<uint32_t>(allowedQueues.size());
		bufferInfo.pQueueFamilyIndices = allowedQueues.data();

		VkResult result = vkCreateBuffer(m_Device, &bufferInfo, nullptr, outBuffer);
		ARC_ASSERT(result == VK_SUCCESS, "Vulkan: Failed to create buffer");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_Device, *outBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		result = vkAllocateMemory(m_Device, &allocInfo, nullptr, outBufferMemory);
		ARC_ASSERT(result == VK_SUCCESS, "Vulkan: Failed to allocate buffer memory");

		vkBindBufferMemory(m_Device, *outBuffer, *outBufferMemory, 0); // Associates the allocated memory with the buffer
	}

	void VulkanAPI::CopyBuffer(VkBuffer srcBuffer, VkBuffer destBuffer, VkDeviceSize size)
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = m_CopyCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		VkBufferCopy copyRegion = {};
		copyRegion.size = size;
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, destBuffer, 1, &copyRegion);
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		vkQueueSubmit(m_CopyQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_CopyQueue);

		vkFreeCommandBuffers(m_Device, m_CopyCommandPool, 1, &commandBuffer);
	}

	void VulkanAPI::Cleanup()
	{
		vkDeviceWaitIdle(m_Device);

		CleanupSwapchain();

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(m_Device, m_ImageAvailableSemaphore[i], nullptr);
			vkDestroySemaphore(m_Device, m_RenderFinishedSemaphore[i], nullptr);
			vkDestroyFence(m_Device, m_InFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(m_Device, m_GraphicsCommandPool, nullptr);

		delete m_Shader;

		vkDestroyBuffer(m_Device, m_VertexBuffer, nullptr);
		vkFreeMemory(m_Device, m_VertexBufferMemory, nullptr);

		vkDestroyDevice(m_Device, nullptr);

		if (m_EnableValidationLayers)
			DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);

		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);

		vkDestroyInstance(m_Instance, nullptr);
	}

	void VulkanAPI::CleanupSwapchain()
	{
		vkDeviceWaitIdle(m_Device);

		vkFreeCommandBuffers(m_Device, m_GraphicsCommandPool, static_cast<uint32_t>(m_GraphicsCommandBuffers.size()), m_GraphicsCommandBuffers.data());

		vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
		vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);

		for (size_t i = 0; i < m_SwapchainFramebuffers.size(); i++)
			vkDestroyFramebuffer(m_Device, m_SwapchainFramebuffers[i], nullptr);

		for (size_t i = 0; i < m_SwapchainImageViews.size(); i++)
			vkDestroyImageView(m_Device, m_SwapchainImageViews[i], nullptr);

		vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
	}

	void VulkanAPI::CreateInstance()
	{
		auto extensions = GetRequiredExtensions();

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Demo";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Arcane Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo; // Needs to be out of scope so it isn't destroyed until vkCreateInstance gets called
		if (m_EnableValidationLayers)
		{
			ARC_ASSERT(CheckValidationLayerSupport(), "Validation Layers requested but not available");

			createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
			createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

			PopulateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = &debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
		ARC_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan Instance");
	}

	void VulkanAPI::CreateSurface()
	{
		VkResult result = m_Window->CreateVulkanWindowSurface(m_Instance, nullptr, &m_Surface);
		ARC_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan Window Surface");
	}

	void VulkanAPI::SelectPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
		ARC_ASSERT(deviceCount > 0, "Failed to find GPUs with Vulkan Support");

		std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, physicalDevices.data());

		VkPhysicalDevice *bestDevice = &physicalDevices[0];
		int bestDeviceScore = ScorePhysicalDeviceSuitability(physicalDevices[0]);
		for (uint32_t i = 1; i < deviceCount; i++)
		{
			int currentDeviceScore = ScorePhysicalDeviceSuitability(physicalDevices[i]);
			if (currentDeviceScore > bestDeviceScore)
			{
				bestDevice = &physicalDevices[i];
				bestDeviceScore = currentDeviceScore;
			}
		}
		if (bestDeviceScore > -1)
			m_PhysicalDevice = *bestDevice;

		ARC_ASSERT(m_PhysicalDevice != VK_NULL_HANDLE, "Failed to find a suitable GPU that supports the extensions required");
	}

	void VulkanAPI::CreateLogicalDeviceAndQueues()
	{
		DeviceQueueIndices indices = FindDeviceQueueIndices(m_PhysicalDevice);
		std::array<VkDeviceQueueCreateInfo, 3> queueCreateInfo;
		std::array<uint32_t, 3> queueIndices = { indices.graphicsQueue.value(), indices.computeQueue.value(), indices.copyQueue.value() }; // Do not need present queue because it overlaps with one of these queues
		float queuePriority = 1.0f;

		for (size_t i = 0; i < queueCreateInfo.size(); i++)
		{
			queueCreateInfo[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo[i].pNext = nullptr;
			queueCreateInfo[i].flags = 0;
			queueCreateInfo[i].queueFamilyIndex = queueIndices[i];
			queueCreateInfo[i].queueCount = 1;
			queueCreateInfo[i].pQueuePriorities = &queuePriority;
		}
		VkPhysicalDeviceFeatures deviceFeatures = {};
		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfo.data();
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfo.size());
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_RequiredExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = m_RequiredExtensions.data();
		if (m_EnableValidationLayers)
		{
			deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames = m_ValidationLayers.data();
		}
		else
		{
			deviceCreateInfo.enabledLayerCount = 0;
		}

		VkResult result = vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device);
		ARC_ASSERT(result == VK_SUCCESS, "Failed to create logical device");

		vkGetDeviceQueue(m_Device, indices.graphicsQueue.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, indices.computeQueue.value(), 0, &m_ComputeQueue);
		vkGetDeviceQueue(m_Device, indices.copyQueue.value(), 0, &m_CopyQueue);
		vkGetDeviceQueue(m_Device, indices.presentQueue.value(), 0, &m_PresentQueue); // Present queue will be one of the existing queues
	}

	void VulkanAPI::CreateSwapchain()
	{
		SwapchainSupportDetails swapchainDetails = QuerySwapchainSupport(m_PhysicalDevice);

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapchainSurfaceFormat(swapchainDetails.formats);
		VkPresentModeKHR presentMode = ChooseSwapchainPresentMode(swapchainDetails.presentModes);
		m_SwapchainExtent = ChooseSwapchainExtent(swapchainDetails.capabilities);
		m_SwapchainImageFormat = surfaceFormat.format;

		uint32_t swapchainImageCount = 2u;
		switch (g_SwapchainPresentMode)
		{
		case SwapchainPresentMode::TRIPLE_BUFFER: swapchainImageCount = 3u; break;
		case SwapchainPresentMode::DOUBLE_BUFFER: swapchainImageCount = 2u; break;
		}
		swapchainImageCount = std::clamp(swapchainImageCount, swapchainDetails.capabilities.minImageCount, swapchainDetails.capabilities.maxImageCount);
		ARC_LOG_INFO("Swapchain buffer count: {0}", swapchainImageCount);

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Surface;
		createInfo.minImageCount = swapchainImageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = m_SwapchainExtent;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // VK_IMAGE_USAGE_TRANSFER_DST_BIT - TODO: Should be this when we render offscreen first and transfer over with post processing

		DeviceQueueIndices indices = FindDeviceQueueIndices(m_PhysicalDevice);
		uint32_t queueIndices[] = { indices.graphicsQueue.value(), indices.presentQueue.value() };

		// The graphics queue and presentation need access to the same images, and if they are in different queues we need to set the image to be concurrent with the two queues
		if (indices.graphicsQueue != indices.presentQueue)
		{
			// TODO: Could manually handle the sharing but for now with current knowledge, this will do
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}
		createInfo.preTransform = swapchainDetails.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VkResult result = vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_Swapchain);
		ARC_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan's swapchain");

		uint32_t imageCount;
		vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, nullptr);
		m_SwapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, m_SwapchainImages.data());
	}

	void VulkanAPI::CreateSwapchainImageViews()
	{
		m_SwapchainImageViews.resize(m_SwapchainImages.size());

		for (size_t i = 0; i < m_SwapchainImageViews.size(); i++)
		{
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_SwapchainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_SwapchainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			VkResult result = vkCreateImageView(m_Device, &createInfo, nullptr, &m_SwapchainImageViews[i]);
			ARC_ASSERT(result == VK_SUCCESS, "Failed to create image views for the swap chain images");
		}
	}

	void VulkanAPI::CreateRenderPass()
	{
		VkAttachmentDescription colourAttachment = {};
		colourAttachment.format = m_SwapchainImageFormat;
		colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Images to be presented in the swapchain

		VkAttachmentReference colourAttachmentRef = {};
		colourAttachmentRef.attachment = 0;
		colourAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Vulkan can support compute subpass so graphics queue needs to be specified
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colourAttachmentRef;
		//subpass.pInputAttachments // Attachments that are read from a shader
		//subpass.pResolveAttachments // Attachments used for multisampling colour attachments
		//subpass.pDepthStencilAttachments // Attachment for a depth and stencil attachment
		//subpass.pPreserveAttachments // Attachments that are not used by this subpass, but for which the data must be preserved

		// TODO: Vulkan has auto defined sub passes before and after your subpass. Since the render pass sets up data in a certain way before and after
		// If you don't specify, Vulkan spec says that they must be provided implicitly
		// However on some Android drivers there is a bug where they will not, and you need to create subpass dependencies for these two implict stages
		// 1. SrcSubpass=VK_SUBPASS_EXTERNAL & DstSubpass=indexToYourFirstSubpass
		// 2. SrcSubpass=indexToYourLastSubpass & DstSubpass=VK_SUBPASS_EXTERNAL
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // We need to wait for the swapchain to finish reading before we can access it
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // We need to wait on this stage and access
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassCreateInfo = {};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &colourAttachment;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpass;
		renderPassCreateInfo.dependencyCount = 1;
		renderPassCreateInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(m_Device, &renderPassCreateInfo, nullptr, &m_RenderPass);
		ARC_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan RenderPass");
	}

	void VulkanAPI::CreateGraphicsPipeline()
	{
		m_Shader = new Shader(&m_Device, "res/Shaders/simple_vert.spv", "res/Shaders/simple_frag.spv");
		auto bindingDescription = Vertex::GetBindingDescription();
		auto attributeDescription = Vertex::GetAttributeDescription();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
		inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		// Viewport can be dynamic but you must create a VkDynamicState and fill it and submit that. Then at render time you must specify
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_SwapchainExtent.width);
		viewport.height = static_cast<float>(m_SwapchainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = m_SwapchainExtent;

		VkPipelineViewportStateCreateInfo viewportCreateInfo = {};
		viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportCreateInfo.viewportCount = 1;
		viewportCreateInfo.pViewports = &viewport;
		viewportCreateInfo.scissorCount = 1;
		viewportCreateInfo.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo = {};
		rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationCreateInfo.depthClampEnable = VK_FALSE; // TODO: Might be useful for shadowmaps?
		rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; // TODO: This is where we can do wireframe
		rasterizationCreateInfo.lineWidth = 1.0f;
		rasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizationCreateInfo.depthBiasEnable = VK_FALSE;
		rasterizationCreateInfo.depthBiasConstantFactor = 0.0f;
		rasterizationCreateInfo.depthBiasClamp = 0.0f;
		rasterizationCreateInfo.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = {}; // Enabling MSAA requires enabling a GPU feature
		multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
		multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleCreateInfo.minSampleShading = 1.0f;
		multisampleCreateInfo.pSampleMask = nullptr;
		multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
		multisampleCreateInfo.alphaToOneEnable = VK_FALSE;

		VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};

		VkPipelineColorBlendAttachmentState colourBlendState = {};
		colourBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colourBlendState.blendEnable = VK_FALSE;
		colourBlendState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colourBlendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colourBlendState.colorBlendOp = VK_BLEND_OP_ADD;
		colourBlendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colourBlendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colourBlendState.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {};
		colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendCreateInfo.logicOpEnable = VK_FALSE;
		colorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
		colorBlendCreateInfo.attachmentCount = 1;
		colorBlendCreateInfo.pAttachments = &colourBlendState;
		colorBlendCreateInfo.blendConstants[0] = 0.0f;
		colorBlendCreateInfo.blendConstants[1] = 0.0f;
		colorBlendCreateInfo.blendConstants[2] = 0.0f;
		colorBlendCreateInfo.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo layoutCreateInfo = {};
		layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutCreateInfo.setLayoutCount = 0;
		layoutCreateInfo.pSetLayouts = nullptr;
		layoutCreateInfo.pushConstantRangeCount = 0;
		layoutCreateInfo.pPushConstantRanges = nullptr;

		VkResult result = vkCreatePipelineLayout(m_Device, &layoutCreateInfo, nullptr, &m_PipelineLayout);
		ARC_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan Pipeline Layout");

		VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.pNext = nullptr;
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(m_Shader->GetShaderStages().size());
		pipelineCreateInfo.pStages = m_Shader->GetShaderStages().data();
		pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
		pipelineCreateInfo.pViewportState = &viewportCreateInfo;
		pipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
		pipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
		pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
		pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
		pipelineCreateInfo.pDynamicState = nullptr;
		pipelineCreateInfo.layout = m_PipelineLayout;
		pipelineCreateInfo.renderPass = m_RenderPass;
		pipelineCreateInfo.subpass = 0; // index of the subpass
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Used to create a pipeline from an existing pipeline
		pipelineCreateInfo.basePipelineIndex = -1; // Used to create a pipeline from an existing pipeline

		result = vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_GraphicsPipeline);
		ARC_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan Graphics Pipeline");
	}

	void VulkanAPI::CreateFramebuffers()
	{
		m_SwapchainFramebuffers.resize(m_SwapchainImageViews.size());

		for (size_t i = 0; i < m_SwapchainFramebuffers.size(); i++)
		{
			VkImageView attachments[] =
			{
				m_SwapchainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = m_SwapchainExtent.width;
			framebufferInfo.height = m_SwapchainExtent.height;
			framebufferInfo.layers = 1;

			VkResult result = vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_SwapchainFramebuffers[i]);
			ARC_ASSERT(result == VK_SUCCESS, "Failed to create render pass with swapchain image views");
		}
	}

	void VulkanAPI::CreateCommandPool()
	{
		DeviceQueueIndices queueIndices = FindDeviceQueueIndices(m_PhysicalDevice);

		VkCommandPoolCreateInfo graphicsCommandPoolInfo = {};
		graphicsCommandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		graphicsCommandPoolInfo.pNext = nullptr;
		graphicsCommandPoolInfo.queueFamilyIndex = queueIndices.graphicsQueue.value();
		graphicsCommandPoolInfo.flags = 0;

		VkResult result = vkCreateCommandPool(m_Device, &graphicsCommandPoolInfo, nullptr, &m_GraphicsCommandPool);
		ARC_ASSERT(result == VK_SUCCESS, "Vulkan: Failed to create graphics command pool");

		VkCommandPoolCreateInfo copyCommandPoolInfo = {};
		copyCommandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		copyCommandPoolInfo.pNext = nullptr;
		copyCommandPoolInfo.queueFamilyIndex = queueIndices.copyQueue.value();
		copyCommandPoolInfo.flags = 0;

		result = vkCreateCommandPool(m_Device, &copyCommandPoolInfo, nullptr, &m_CopyCommandPool);
		ARC_ASSERT(result == VK_SUCCESS, "Vulkan: Failed to create copy command pool");
	}

	void VulkanAPI::CreateCommandBuffers()
	{
		m_GraphicsCommandBuffers.resize(m_SwapchainFramebuffers.size());

		// Allocate commands buffers from the command pool
		{
			VkCommandBufferAllocateInfo allocateCreateInfo = {};
			allocateCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocateCreateInfo.pNext = nullptr;
			allocateCreateInfo.commandPool = m_GraphicsCommandPool;
			allocateCreateInfo.commandBufferCount = static_cast<uint32_t>(m_GraphicsCommandBuffers.size());
			allocateCreateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Secondary level can be reused in primary buffers. Good for re-use

			VkResult result = vkAllocateCommandBuffers(m_Device, &allocateCreateInfo, m_GraphicsCommandBuffers.data());
			ARC_ASSERT(result == VK_SUCCESS, "Failed to allocate Vulkan command buffers");
		}

		// Record commands into the command buffers
		// We need a command buffer, one for each framebuffer we are rendering to (which will match our swapchain buffer count)
		for (size_t i = 0; i < m_GraphicsCommandBuffers.size(); i++)
		{
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.pNext = nullptr;
			beginInfo.flags = 0;
			beginInfo.pInheritanceInfo = nullptr;

			VkResult result = vkBeginCommandBuffer(m_GraphicsCommandBuffers[i], &beginInfo);
			ARC_ASSERT(result == VK_SUCCESS, "Failed to begin Vulkan command buffer recording");

			VkRenderPassBeginInfo renderPassBegin = {};
			renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBegin.pNext = nullptr;
			renderPassBegin.renderPass = m_RenderPass;
			renderPassBegin.framebuffer = m_SwapchainFramebuffers[i];
			renderPassBegin.renderArea.offset = { 0, 0 };
			renderPassBegin.renderArea.extent = m_SwapchainExtent;
			VkClearValue clearColour = { 0.0f, 0.0f, 0.0f, 1.0f };
			renderPassBegin.clearValueCount = 1;
			renderPassBegin.pClearValues = &clearColour;

			VkBuffer vertexBuffers[] = { m_VertexBuffer };
			VkDeviceSize offsets[] = { 0 };

			vkCmdBeginRenderPass(m_GraphicsCommandBuffers[i], &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE); // Need to specify if you are using secondary command buffers here
			vkCmdBindPipeline(m_GraphicsCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);
			vkCmdBindVertexBuffers(m_GraphicsCommandBuffers[i], 0, 1, vertexBuffers, offsets);
			vkCmdDraw(m_GraphicsCommandBuffers[i], 3, 1, 0, 0);
			vkCmdEndRenderPass(m_GraphicsCommandBuffers[i]);
			
			result = vkEndCommandBuffer(m_GraphicsCommandBuffers[i]);
			ARC_ASSERT(result == VK_SUCCESS, "Failed to record Vulkan command buffer");
		}
	}

	void VulkanAPI::CreateSyncObjects()
	{
		m_ImageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
		m_RenderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		m_ImagesInFlight.resize(m_SwapchainImages.size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo sempaphoreInfo = {};
		sempaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		sempaphoreInfo.pNext = nullptr;
		sempaphoreInfo.flags = 0; 

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.pNext = nullptr;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start it in a signaled state avoids hanging on vkWaitForFences call at the start of the first frame

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkResult result = vkCreateSemaphore(m_Device, &sempaphoreInfo, nullptr, &m_ImageAvailableSemaphore[i]);
			ARC_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan semaphore");
			result = vkCreateSemaphore(m_Device, &sempaphoreInfo, nullptr, &m_RenderFinishedSemaphore[i]);
			ARC_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan semaphore");
			result = vkCreateFence(m_Device, &fenceInfo, nullptr, &m_InFlightFences[i]);
			ARC_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan fence");
		}
	}

	void VulkanAPI::RecreateSwapchain()
	{
		// Pause render thread if window is minimized
		while (m_Window->GetWidth() == 0 || m_Window->GetHeight() == 0)
		{
			glfwWaitEvents();
		}

		ARC_LOG_INFO("Vulkan: Recreating the Swapchain");

		CleanupSwapchain();

		CreateSwapchain();
		CreateSwapchainImageViews();
		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateFramebuffers();
		CreateCommandBuffers();
	}

	void VulkanAPI::CreateVertexBuffer()
	{
		const std::vector<Vertex> vertices =
		{
			{{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		// HOST_COHERENT_BIT guarantees the driver completes the memory transfer operation for the VkMapMemory operation before the next VkQueueSubmit call
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_SHARING_MODE_CONCURRENT, &stagingBuffer, &stagingBufferMemory);

		void *data;
		vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(m_Device, stagingBufferMemory);

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SHARING_MODE_CONCURRENT, &m_VertexBuffer, &m_VertexBufferMemory);
		
		CopyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);
		vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
		vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
	}

	uint32_t VulkanAPI::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		ARC_ASSERT(false, "Vulkan: Failed to find suitable memory type for allocation");
		return 0;
	}

	int VulkanAPI::ScorePhysicalDeviceSuitability(const VkPhysicalDevice &device)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		VkPhysicalDeviceMemoryProperties deviceMemoryProperties;

		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		vkGetPhysicalDeviceMemoryProperties(device, &deviceMemoryProperties);

		DeviceQueueIndices queueIndices = FindDeviceQueueIndices(device);
		SwapchainSupportDetails swapchainDetails = QuerySwapchainSupport(device);
		bool swapchainAdequate = !swapchainDetails.formats.empty() && !swapchainDetails.presentModes.empty();

		// These are things that are required to be supported
		if (!queueIndices.IsSuitable())
			return -1;
		if (!CheckPhysicalDeviceExtensionSupport(device))
			return -1;
		if (!swapchainAdequate)
			return -1;
		if (!deviceFeatures.geometryShader)
			return -1;
		if (!deviceFeatures.tessellationShader)
			return -1;

		int score = 0;
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 1000;
		}
		for (uint32_t i = 0; i < deviceMemoryProperties.memoryHeapCount; i++)
		{
			score += static_cast<int>(deviceMemoryProperties.memoryHeaps[i].size / std::pow(1024, 2)); // Megabytes of memory
		}
		score += deviceProperties.limits.maxImageDimension2D;

		return score;
	}

	bool VulkanAPI::CheckPhysicalDeviceExtensionSupport(const VkPhysicalDevice & device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(m_RequiredExtensions.begin(), m_RequiredExtensions.end());
		for (size_t i = 0; i < availableExtensions.size(); i++)
		{
			requiredExtensions.erase(availableExtensions[i].extensionName);
		}

		return requiredExtensions.empty();
	}

	DeviceQueueIndices VulkanAPI::FindDeviceQueueIndices(const VkPhysicalDevice &device)
	{
		DeviceQueueIndices queueIndices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		for (uint32_t i = 0; i < queueFamilies.size(); i++)
		{
			/*
				Check for presenting support and get the queue index (ideally the present queue would be the graphics queue since it would be more efficient)
				On almost all modern systems the graphics queue will be the present queue but the spec doesn't guarantee that
				https://stackoverflow.com/questions/51149001/can-graphics-and-present-queues-really-be-different
				So for now keep this additional conditional check for setting the present queue index and just override the present queue index if it is also a graphics queue
			*/
			VkBool32 presentQueue = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentQueue);
			if (presentQueue && (!queueIndices.presentQueue.has_value() || queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				queueIndices.presentQueue = i;
			}

			// Get the queue indices for our graphics, compute, and copy queues
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				queueIndices.graphicsQueue = i;
			}
			else if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				queueIndices.computeQueue = i;
			}
			else if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
			{
				queueIndices.copyQueue = i;
			}

			// If we find all of the suitable queues, we can early out
			if (queueIndices.IsSuitable())
			{
				break;
			}
		}

		return queueIndices;
	}

	SwapchainSupportDetails VulkanAPI::QuerySwapchainSupport(const VkPhysicalDevice &device)
	{
		SwapchainSupportDetails swapchainDetails;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &swapchainDetails.capabilities);

		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);
		if (formatCount != 0)
		{
			swapchainDetails.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, swapchainDetails.formats.data());
		}

		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);
		if (presentModeCount != 0)
		{
			swapchainDetails.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, swapchainDetails.presentModes.data());
		}

		return swapchainDetails;
	}

	VkSurfaceFormatKHR VulkanAPI::ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (size_t i = 0; i < availableFormats.size(); i++)
		{
			if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormats[i];
			}
		}

		// TODO: Instead rank the available formats and choose the best one and log warn on the type that was chosen
		ARC_LOG_WARN("Ideal Swapchain Format Unavailable");
		return availableFormats[0];
	}

	VkPresentModeKHR VulkanAPI::ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		VkPresentModeKHR desiredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		switch (g_SwapchainPresentMode)
		{
			//case SwapchainPresentMode::VSYNC_OFF: break; // TODO: Add support for turning vsync off
		case SwapchainPresentMode::DOUBLE_BUFFER: desiredPresentMode = VK_PRESENT_MODE_FIFO_KHR; break;
		case SwapchainPresentMode::TRIPLE_BUFFER: desiredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR; break;
		}

		for (size_t i = 0; i < availablePresentModes.size(); i++)
		{
			if (availablePresentModes[i] == desiredPresentMode)
			{
				return availablePresentModes[i];
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR; // This mode is guaranteed to be available
	}

	VkExtent2D VulkanAPI::ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR &capabilities)
	{
		// Create the swapchain extent by using the window size
		VkExtent2D extent = { static_cast<uint32_t>(m_Window->GetWidth()), static_cast<uint32_t>(m_Window->GetHeight()) };
		extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		return extent;
	}

	std::vector<const char*> VulkanAPI::GetRequiredExtensions()
	{
		uint32_t supportedExtensionCount = 0;
		const char **supportedExtensions = m_Window->GetExtensions(&supportedExtensionCount);
		std::vector<const char*> extensions(supportedExtensions, supportedExtensions + supportedExtensionCount);

		if (m_EnableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	bool VulkanAPI::CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char *layerName : m_ValidationLayers)
		{
			bool layerFound = false;

			for (const auto &layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}
		return true;
	}

	void VulkanAPI::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = VulkanAPI::DebugCallback;
		createInfo.pUserData = nullptr;
	}

	void VulkanAPI::SetupValidationLayers()
	{
		if (!m_EnableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugMessengerCreateInfo(createInfo);

		VkResult result = CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger);
		ARC_ASSERT(result == VK_SUCCESS, "Failed to Setup Debug Messenger for Validation Layers");
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanAPI::DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
		void *userData)
	{
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			switch (messageSeverity)
			{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: ARC_LOG_ERROR(callbackData->pMessage); break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: ARC_LOG_INFO(callbackData->pMessage); break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: ARC_LOG_WARN(callbackData->pMessage); break;
			default: ARC_LOG_ERROR(callbackData->pMessage); break;
			}
		}

		return VK_FALSE;
	}

	VkResult VulkanAPI::CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT *createInfo,
		VkAllocationCallbacks *allocator,
		VkDebugUtilsMessengerEXT *debugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, createInfo, allocator, debugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void VulkanAPI::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks * allocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, allocator);
		}
	}
}
