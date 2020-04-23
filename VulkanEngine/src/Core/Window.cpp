#include "arcpch.h"
#include "Window.h"

Window::Window(const char *title, int width, int height) : m_Title(title), m_Width(width), m_Height(height)
{
	Init();
}

Window::~Window() 
{
	glfwDestroyWindow(m_Window);

	glfwTerminate();
}

void Window::Init()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_Window = glfwCreateWindow(m_Width, m_Height, m_Title, nullptr, nullptr);
}

void Window::Update()
{
	glfwPollEvents();
}

VkResult Window::CreateVulkanWindowSurface(VkInstance &instance, VkAllocationCallbacks *allocator, VkSurfaceKHR *surface) const
{
	return glfwCreateWindowSurface(instance, m_Window, nullptr, surface);
}

const char** Window::GetExtensions(uint32_t *outExtensionCount) const
{
	return glfwGetRequiredInstanceExtensions(outExtensionCount);
}
