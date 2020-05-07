#include "arcpch.h"
#include "Application.h"

#include "Core/FileUtils.h"
#include "Core/Logger.h"
#include "Events/ApplicationEvent.h"

namespace Arcane
{
	Application::Application()
		: m_Window(std::string("Arcane Engine"), 1280, 720), m_Vulkan(&m_Window)
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
		float fps = 0;
		m_Timer.Reset();

		while (!m_Window.ShouldClose())
		{
			m_Window.Update();
			Render();
			++fps;

			if (m_Timer.Elapsed() >= 1.0)
			{
				std::string profileString = std::string("- ") + std::to_string(fps) + std::string("fps - ") + std::to_string(1000.0f / fps) + std::string("ms");
				m_Window.AppendTitle(profileString);
				fps = 0.0;
				m_Timer.Rewind(1.0);
			}
		}

		vkDeviceWaitIdle(m_Vulkan.GetDevice()); // If window closes finish GPU work before deleting resources that are in-flight
	}

	void Application::Render()
	{
		m_Vulkan.Render();
	}

	void Application::Cleanup()
	{
	
	}
}
