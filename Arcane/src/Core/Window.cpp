#include "arcpch.h"
#include "Window.h"

#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

namespace Arcane
{
	static void GLFWErrorCallback(int error, const char *description)
	{
		ARC_LOG_ERROR("GLFW Error Code: {0} - {1}", error, description);
	}

	Window::Window(const std::string &title, int width, int height) : m_Data()
	{
		m_Data.Title = title;
		m_Data.Width = width; m_Data.Height = height;
		m_Data.VSync = false;

		Init();
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_Window);

		glfwTerminate();
	}

	void Window::Init()
	{
		int success = glfwInit();
		ARC_ASSERT(success, "Failed to initialize GLFW");
		glfwSetErrorCallback(GLFWErrorCallback);

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		ARC_LOG_INFO("Creating window ({0}, {1})", m_Data.Width, m_Data.Height);
		m_Window = glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(m_Window, &m_Data);


		// GLFW callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow *window, int width, int height)
		{
			WindowData *data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
			data->Width = width;
			data->Height = height;

			WindowResizeEvent event(width, height);
			data->EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow *window)
		{
			WindowData *data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));

			WindowCloseEvent event;
			data->EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow *window, int key, int scancode, int action, int mods)
		{
			WindowData *data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					data->EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data->EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, 1);
					data->EventCallback(event);
					break;
				}
				default:
				{
					ARC_LOG_WARN("Key Input Action: {0} - Not available", action);
					break;
				}
			}
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow *window, int button, int action, int mods)
		{
			WindowData *data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data->EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data->EventCallback(event);
					break;
				}
				default:
				{
					ARC_LOG_WARN("Key Input Action: {0} - Not available", action);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow *window, double xOffset, double yOffset)
		{
			WindowData *data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));

			MouseScrolledEvent event(xOffset, yOffset);
			data->EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow *window, double xPos, double yPos)
		{
			WindowData *data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));

			MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
			data->EventCallback(event);
		});
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
		glfwSetWindowTitle(m_Window, (m_Data.Title + " " + value).c_str());
	}

	const char** Window::GetExtensions(uint32_t *outExtensionCount) const
	{
		return glfwGetRequiredInstanceExtensions(outExtensionCount);
	}
}
