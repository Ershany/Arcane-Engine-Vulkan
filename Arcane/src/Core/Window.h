#pragma once

class Event;

namespace Arcane
{
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		Window(const std::string &title, int width, int height);
		~Window();

		void Update();
		VkResult CreateVulkanWindowSurface(VkInstance &instance, VkAllocationCallbacks *allocator, VkSurfaceKHR *surface) const;

		void AppendTitle(std::string &value);

		inline void SetEventCallback(const EventCallbackFn &callback) { m_Data.EventCallback = callback; };

		inline int GetWidth() const { return m_Data.Width; }
		inline int GetHeight() const { return m_Data.Height; }
		const char** GetExtensions(uint32_t *outExtensionCount) const;
		inline bool ShouldClose() const { return glfwWindowShouldClose(m_Window); }
	private:
		void Init();
	private:
		GLFWwindow *m_Window;

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;

			EventCallbackFn EventCallback;
		};
		WindowData m_Data;
	};
}
