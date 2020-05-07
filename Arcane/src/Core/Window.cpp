#include "arcpch.h"
#include "Window.h"

namespace Arcane
{
	Window::Window(const std::string &title, int width, int height) : m_Title(title), m_Width(width), m_Height(height)
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

		ARC_LOG_INFO("Creating window ({0}, {1})", m_Width, m_Height);
		m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
	}

	void Window::Update()
	{
		glfwPollEvents();
	}

	VkResult Window::CreateVulkanWindowSurface(VkInstance &instance, VkAllocationCallbacks *allocator, VkSurfaceKHR *surface) const
	{
		return glfwCreateWindowSurface(instance, m_Window, nullptr, surface);
	}

	void Window::AppendTitle(std::string & value)
	{
		glfwSetWindowTitle(m_Window, (m_Title + " " + value).c_str());
	}

	const char** Window::GetExtensions(uint32_t *outExtensionCount) const
	{
		return glfwGetRequiredInstanceExtensions(outExtensionCount);
	}
}
