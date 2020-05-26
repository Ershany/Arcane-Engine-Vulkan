#pragma once

#include "Defs.h"
#include "Core/Timer.h"
#include "Core/Singleton.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Core/LayerStack.h"

namespace Arcane
{
	class Window;
	class VulkanAPI;
	class Layer;

	class Application : public Singleton
	{
	private:
		Application();
		virtual  ~Application();
	public:
		static Application& GetInstance();

		void Run();

		void OnEvent(Event &e);

		void PushLayer(Layer *layer);
		void PushOverlay(Layer *overlay);

		inline VulkanAPI* GetVulkanAPI() { return m_Vulkan; }
	private:
		void Cleanup();
		void Loop();
		void Render();

		bool OnWindowResize(WindowResizeEvent &e);
	private:
		Window *m_Window;
		Timer m_Timer;
		VulkanAPI *m_Vulkan;
		LayerStack m_LayerStack;
	};
}
