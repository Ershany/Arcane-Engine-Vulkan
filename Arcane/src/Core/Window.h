#pragma once

namespace Arcane
{
	class Event;
	class Application;

	class Window
	{
	public:
		Window(const std::string &title, int width, int height);
		~Window();

		void Init(Application *application);
		void Update();
		VkResult CreateVulkanWindowSurface(VkInstance &instance, VkAllocationCallbacks *allocator, VkSurfaceKHR *surface) const;

		void AppendTitle(std::string &value);

		inline int GetWidth() const { return m_Data.Width; }
		inline int GetHeight() const { return m_Data.Height; }
		const char** GetExtensions(uint32_t *outExtensionCount) const;
		inline bool ShouldClose() const { return glfwWindowShouldClose(m_Window); }
	private:
		GLFWwindow *m_Window;

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;

			Application *App;
		};
		WindowData m_Data;
	};
}
