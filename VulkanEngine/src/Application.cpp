#include "pch.h"
#include "Application.h"

#include "Core/FileUtils.h"

Application::Application() : m_Window("Arcane Engine - Vulkan RT Support", 1280, 720), m_Instance(VK_NULL_HANDLE), m_PhysicalDevice(VK_NULL_HANDLE), m_Device(VK_NULL_HANDLE), m_Swapchain(VK_NULL_HANDLE), m_SwapchainImageFormat(VK_FORMAT_UNDEFINED),
							 m_SwapchainExtent(), m_Surface(VK_NULL_HANDLE), m_GraphicsQueue(VK_NULL_HANDLE), m_ComputeQueue(VK_NULL_HANDLE), m_CopyQueue(VK_NULL_HANDLE), m_PresentQueue(VK_NULL_HANDLE), m_DebugMessenger(VK_NULL_HANDLE)
{
}

Application::~Application()
{
	Cleanup();
}

void Application::Run()
{
	InitVulkan();

	Loop();
}

void Application::Loop()
{
	while (!m_Window.ShouldClose())
	{
		m_Window.Update();
	}
}

void Application::Cleanup()
{
	for (uint32_t i = 0; i < m_SwapchainImageViews.size(); i++)
		vkDestroyImageView(m_Device, m_SwapchainImageViews[i], nullptr);

	vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);

	vkDestroyDevice(m_Device, nullptr);

	if (m_EnableValidationLayers)
		DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);

	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);

	vkDestroyInstance(m_Instance, nullptr);
}

void Application::InitVulkan()
{
	CreateInstance();
	SetupValidationLayers();
	CreateSurface();
	SelectPhysicalDevice();
	CreateLogicalDeviceAndQueues();
	CreateSwapchain();
	CreateSwapchainImageViews();
	CreateGraphicsPipeline();
}

void Application::CreateInstance()
{
	if (m_EnableValidationLayers && !CheckValidationLayerSupport())
	{
		throw std::runtime_error("Validation Layers Requested, But Not Available");
	}

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
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan Instance");
	}
}

void Application::CreateSurface()
{
	VkResult result = m_Window.CreateVulkanWindowSurface(m_Instance, nullptr, &m_Surface);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan Window Surface");
	}
}

void Application::SelectPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
	if (deviceCount == 0)
		throw std::runtime_error("Failed to find GPUs with Vulkan support");

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

	if (m_PhysicalDevice == VK_NULL_HANDLE)
		throw std::runtime_error("Failed to find a suitable GPU that supports the extensions required");
}

void Application::CreateLogicalDeviceAndQueues()
{
	DeviceQueueIndices indices = FindDeviceQueueIndices(m_PhysicalDevice);
	std::array<VkDeviceQueueCreateInfo, 3> queueCreateInfo;
	std::array<uint32_t, 3> queueIndices = { indices.graphicsQueue.value(), indices.computeQueue.value(), indices.copyQueue.value() }; // Do not need present queue because it overlaps with one of these queues
	float queuePriority = 1.0f;

	for (uint32_t i = 0; i < queueCreateInfo.size(); i++)
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
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device");
	}

	vkGetDeviceQueue(m_Device, indices.graphicsQueue.value(), 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_Device, indices.computeQueue.value(), 0, &m_ComputeQueue);
	vkGetDeviceQueue(m_Device, indices.copyQueue.value(), 0, &m_CopyQueue);
	vkGetDeviceQueue(m_Device, indices.presentQueue.value(), 0, &m_PresentQueue); // Present queue will be one of the existing queues
}

void Application::CreateSwapchain()
{
	SwapchainSupportDetails swapchainDetails = QuerySwapchainSupport(m_PhysicalDevice);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapchainSurfaceFormat(swapchainDetails.formats);
	VkPresentModeKHR presentMode = ChooseSwapchainPresentMode(swapchainDetails.presentModes);
	m_SwapchainExtent = ChooseSwapchainExtent(swapchainDetails.capabilities);
	m_SwapchainImageFormat = surfaceFormat.format;

	uint32_t swapchainImageCount = 2u;
	switch (g_SwapchainPresentMode)
	{
	//case SwapchainPresentMode::VSYNC_OFF: swapchainImageCount = 1u; break; // TODO: Add support for turning vsync off
	case SwapchainPresentMode::VSYNC_TRIPLE_BUFFER: swapchainImageCount = 3u; break;
	case SwapchainPresentMode::VSYNC_DOUBLE_BUFFER: swapchainImageCount = 2u; break;
	}
	// TODO: Can log a warning if this value ends up being out of bounds
	swapchainImageCount = std::clamp(swapchainImageCount, swapchainDetails.capabilities.minImageCount, swapchainDetails.capabilities.maxImageCount);

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
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}
	createInfo.preTransform = swapchainDetails.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult result = vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_Swapchain);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create the swapchain");
	}

	uint32_t imageCount;
	vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, nullptr);
	m_SwapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, m_SwapchainImages.data());
}

void Application::CreateSwapchainImageViews()
{
	m_SwapchainImageViews.resize(m_SwapchainImages.size());

	for (uint32_t i = 0; i < m_SwapchainImageViews.size(); i++)
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
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image views for the swap chain images");
		}
	}
}

void Application::CreateGraphicsPipeline()
{
	auto vertShaderCode = FileUtils::ReadFile("res/Shaders/simple_vert.spv");
	auto fragShaderCode = FileUtils::ReadFile("res/Shaders/simple_frag.spv");
}

int Application::ScorePhysicalDeviceSuitability(const VkPhysicalDevice &device)
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

bool Application::CheckPhysicalDeviceExtensionSupport(const VkPhysicalDevice & device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(m_RequiredExtensions.begin(), m_RequiredExtensions.end());
	for (uint32_t i = 0; i < availableExtensions.size(); i++)
	{
		requiredExtensions.erase(availableExtensions[i].extensionName);
	}

	return requiredExtensions.empty();
}

DeviceQueueIndices Application::FindDeviceQueueIndices(const VkPhysicalDevice &device)
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

SwapchainSupportDetails Application::QuerySwapchainSupport(const VkPhysicalDevice &device)
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

VkSurfaceFormatKHR Application::ChooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (uint32_t i = 0; i < availableFormats.size(); i++)
	{
		if (availableFormats[i].format == VK_FORMAT_B8G8R8_SRGB && availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormats[i];
	}

	// TODO: Log that an incorrect format was chosen for the swapchain
	// TODO: Instead, rank the available formats and choose the best one and log info on the type that was chosen
	// TODO: Still log a warning if the types we want were not available
	return availableFormats[0];
}

VkPresentModeKHR Application::ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	VkPresentModeKHR desiredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	switch (g_SwapchainPresentMode)
	{
	//case SwapchainPresentMode::VSYNC_OFF: break; // TODO: Add support for turning vsync off
	case SwapchainPresentMode::VSYNC_DOUBLE_BUFFER: desiredPresentMode = VK_PRESENT_MODE_FIFO_KHR; break;
	case SwapchainPresentMode::VSYNC_TRIPLE_BUFFER: desiredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR; break;
	}

	// TODO: Change the desired mode based on settings
	for (uint32_t i = 0; i < availablePresentModes.size(); i++)
	{
		if (availablePresentModes[i] == desiredPresentMode)
		{
			return availablePresentModes[i];
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR; // This mode is guaranteed to be available
}

VkExtent2D Application::ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
	// Create the swapchain extent by using the window size
	VkExtent2D extent = { static_cast<uint32_t>(m_Window.GetWidth()), static_cast<uint32_t>(m_Window.GetHeight()) };
	extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
	return extent;
}

std::vector<const char*> Application::GetRequiredExtensions()
{
	uint32_t supportedExtensionCount = 0;
	const char **supportedExtensions = m_Window.GetExtensions(&supportedExtensionCount);
	std::vector<const char*> extensions(supportedExtensions, supportedExtensions + supportedExtensionCount);

	if (m_EnableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

bool Application::CheckValidationLayerSupport()
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

void Application::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = Application::DebugCallback;
	createInfo.pUserData = nullptr;
}

void Application::SetupValidationLayers()
{
	if (!m_EnableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo(createInfo);

	VkResult result = CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger);
	if (result != VK_SUCCESS)
	{
		std::runtime_error("Failed to Setup Debug Messenger for Validation Layers");
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL Application::DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
	void *userData)
{
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		std::cerr << "Validation Layer: " << callbackData->pMessage << std::endl;
	}

	return VK_FALSE;
}

VkResult Application::CreateDebugUtilsMessengerEXT(
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

void Application::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks * allocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, allocator);
	}
}
