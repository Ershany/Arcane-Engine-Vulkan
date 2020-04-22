#pragma once

class Window
{
public:
	Window(const char *title, int width, int height);
	~Window();

	void Init();
	void Update();
	VkResult CreateVulkanWindowSurface(VkInstance &instance, VkAllocationCallbacks *allocator, VkSurfaceKHR *surface);

	inline int GetWidth() { return m_Width; }
	inline int GetHeight() { return m_Height; }
	const char** GetExtensions(uint32_t *outExtensionCount);
	inline bool ShouldClose() const { return glfwWindowShouldClose(m_Window); }
private:
	const char *m_Title;
	int m_Width, m_Height;
	GLFWwindow *m_Window;
};
