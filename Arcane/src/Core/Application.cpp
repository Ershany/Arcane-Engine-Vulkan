#include "arcpch.h"
#include "Application.h"

#include "Core/Window.h"
#include "Core/FileUtils.h"
#include "Core/Logger.h"
#include "Renderer/VulkanAPI.h"

namespace Arcane
{
	Application::Application()
	{
		m_Window = new Window(std::string("Arcane Engine"), 1280, 720);
		m_Window->Init(this);

		m_Vulkan = new VulkanAPI(m_Window);
	}

	Application::~Application()
	{
		Cleanup();
	}

	void Application::Run()
	{
		m_Vulkan->InitVulkan();

		Loop();
	}

	void Application::OnEvent(Event & e)
	{
		ARC_LOG_INFO("{0}", e);
	}

	void Application::Cleanup()
	{
		delete m_Vulkan;
		delete m_Window;
	}

	void Application::Loop()
	{
		float fps = 0;
		m_Timer.Reset();

		while (!m_Window->ShouldClose())
		{
			m_Window->Update();
			Render();
			++fps;

			if (m_Timer.Elapsed() >= 1.0)
			{
				std::string profileString = std::string("- ") + std::to_string(fps) + std::string("fps - ") + std::to_string(1000.0f / fps) + std::string("ms");
				m_Window->AppendTitle(profileString);
				fps = 0.0;
				m_Timer.Rewind(1.0);
			}
		}
	}

	void Application::Render()
	{
		m_Vulkan->Render();
	}
}
