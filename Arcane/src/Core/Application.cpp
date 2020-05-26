#include "arcpch.h"
#include "Application.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Core/Window.h"
#include "Core/FileUtils.h"
#include "Core/Layer.h"
#include "Core/Logger.h"
#include "Graphics/Renderer/VulkanAPI.h"

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

	Application& Application::GetInstance()
	{
		static Application app;
		return app;
	}

	void Application::Run()
	{
		m_Vulkan->InitVulkan();

		Loop();
	}

	void Application::OnEvent(Event &e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(std::bind(&Application::OnWindowResize, this, std::placeholders::_1));

		for (auto iter = m_LayerStack.end(); iter != m_LayerStack.begin(); --iter)
		{
			(*iter)->OnEvent(e);
			if (e.Handled)
				break;
		}

		//ARC_LOG_INFO("{0}", e);
	}

	void Application::PushLayer(Layer *layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer *overlay)
	{
		m_LayerStack.PushOverlay(overlay);
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
			for (auto layer : m_LayerStack)
			{
				layer->OnUpdate();
			}

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

	bool Application::OnWindowResize(WindowResizeEvent &e)
	{
		if (e.GetWidth() > 0 && e.GetHeight() > 0)
		{
			m_Vulkan->NotifyWindowResized();
		}

		return false;
	}
}
