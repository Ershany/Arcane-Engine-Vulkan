#include "arcpch.h"
#include "LayerStack.h"

#include "Core/Layer.h"

namespace Arcane
{
	LayerStack::~LayerStack()
	{
		for (auto layer : m_Layers)
		{
			layer->OnDetach();
			delete layer;
		}
	}

	void LayerStack::PushLayer(Layer *layer)
	{
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
		layer->OnAttach();
		m_LayerInsertIndex++;
	}

	void LayerStack::PushOverlay(Layer *overlay)
	{
		m_Layers.emplace_back(overlay);
		overlay->OnAttach();
	}

	void LayerStack::PopLayer(Layer *layer)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex, layer);
		if (it != m_Layers.begin() + m_LayerInsertIndex)
		{
			layer->OnDetach();
			m_Layers.erase(it);
			m_LayerInsertIndex--;
		}
		else
		{
			ARC_LOG_WARN("Failed to pop layer: {0}, from the layer stack", layer->GetName());
		}
	}

	void LayerStack::PopOverlay(Layer *overlay)
	{
		auto it = std::find(m_Layers.begin() + m_LayerInsertIndex, m_Layers.end(), overlay);
		if (it != m_Layers.end())
		{
			overlay->OnDetach();
			m_Layers.erase(it);
		}
		else
		{
			ARC_LOG_WARN("Failed to pop overlay: {0}, from the layer stack", overlay->GetName());
		}
	}
}
