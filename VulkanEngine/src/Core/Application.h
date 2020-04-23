#pragma once

#include "Window.h"
#include "Renderer/VulkanAPI.h"

class Application
{
public:
	Application();
	~Application();

	void Run();
private:
	void Loop();
	void Cleanup();
private:
	Window m_Window;

	VulkanAPI m_Vulkan;
};
