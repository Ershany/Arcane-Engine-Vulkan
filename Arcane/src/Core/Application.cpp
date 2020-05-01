#include "arcpch.h"
#include "Application.h"

#include "Core/FileUtils.h"
#include "Core/Logger.h"
#include "Events/ApplicationEvent.h"

namespace Arcane
{
	Application::Application()
		: m_Window("Arcane Engine - Vulkan RT Support", 1280, 720), m_Vulkan(&m_Window)
	{
	
	}

	Application::~Application()
	{
		Cleanup();
	}

	void Application::Run()
	{
		m_Vulkan.InitVulkan();

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
	
	}
}
