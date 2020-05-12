#pragma once

#include "Defs.h"
#include "Core/Timer.h"
#include "Events/Event.h"
#include "Core/LayerStack.h"

namespace Arcane
{
	class Window;
	class VulkanAPI;
	class Layer;

	class Application
	{
	public:
		Application();
		virtual  ~Application();

		void Run();

		void OnEvent(Event &e);

		void PushLayer(Layer *layer);
		void PushOverlay(Layer *overlay);
	private:
		void Cleanup();
		void Loop();
		void Render();
	private:
		Window *m_Window;
		Timer m_Timer;
		VulkanAPI *m_Vulkan;
		LayerStack m_LayerStack;
	};
}
