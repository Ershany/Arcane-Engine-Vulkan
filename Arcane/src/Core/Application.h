#pragma once

#include "Window.h"
#include "Renderer/VulkanAPI.h"
#include "Defs.h"
#include "Events/Event.h"
#include "Core/Timer.h"

namespace Arcane
{
	class Application
	{
	public:
		Application();
		virtual  ~Application();

		void Run();

		void OnEvent(Event &e);
	private:
		void Cleanup();
		void Loop();
		void Render();
	private:
		Window *m_Window;
		Timer m_Timer;

		VulkanAPI *m_Vulkan;
	};
}
