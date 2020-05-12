#include "arcpch.h"
#include "LayerStack.h"

#include "Core/Layer.h"

namespace Arcane
{
	LayerStack::LayerStack()
	{
		m_LayerInsert = m_Layers.begin();
	}

	LayerStack::~LayerStack()
	{
		for (auto layer : m_Layers)
			delete layer;
	}

	void LayerStack::PushLayer(Layer *layer)
	{
		m_LayerInsert = m_Layers.emplace(m_LayerInsert, layer);
	}

	void LayerStack::PushOverlay(Layer *overlay)
	{
		m_Layers.emplace_back(overlay);
	}

	void LayerStack::PopLayer(Layer *layer)
	{
		auto it = std::find(m_Layers.begin(), m_LayerInsert, layer);
		if (it != m_Layers.end())
		{
			m_Layers.erase(it);
			--m_LayerInsert;
		}
		else
		{
			ARC_LOG_WARN("Failed to pop layer: {0}, from the layer stack", layer->GetName());
		}
	}

	void LayerStack::PopOverlay(Layer *overlay)
	{
		auto it = std::find(m_LayerInsert, m_Layers.end(), overlay);
		if (it != m_Layers.end())
		{
			m_Layers.erase(it);
		}
		else
		{
			ARC_LOG_WARN("Failed to pop overlay: {0}, from the layer stack", overlay->GetName());
		}
	}
}
