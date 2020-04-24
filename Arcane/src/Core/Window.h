#pragma once

namespace Arcane
{
	class Window
	{
	public:
		Window(const char *title, int width, int height);
		~Window();

		void Init();
		void Update();
		VkResult CreateVulkanWindowSurface(VkInstance &instance, VkAllocationCallbacks *allocator, VkSurfaceKHR *surface) const;

		inline int GetWidth() const { return m_Width; }
		inline int GetHeight() const { return m_Height; }
		const char** GetExtensions(uint32_t *outExtensionCount) const;
		inline bool ShouldClose() const { return glfwWindowShouldClose(m_Window); }
	private:
		const char *m_Title;
		int m_Width, m_Height;
		GLFWwindow *m_Window;
	};
}
