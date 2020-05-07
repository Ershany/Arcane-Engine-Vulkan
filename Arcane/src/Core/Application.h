#pragma once

#include "Defs.h"
#include "Core/Timer.h"
#include "Events/Event.h"

namespace Arcane
{
	class Window;
	class VulkanAPI;

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
