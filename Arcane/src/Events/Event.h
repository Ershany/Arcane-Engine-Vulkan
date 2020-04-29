#pragma once

/*
	Events in Arcane are not being buffered, instead they are blocking
	That means every time an event gets dispatched it must be processed
	TODO: Buffer Events and process during the event part of the update stage
*/

#include "arcpch.h"

namespace Arcane
{

	enum class EventType
	{
		None = 0,
		ApplicationUpdate, ApplicationRender, ApplicationTick,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		MouseButtonPressed, MouseButtonReleased, MouseScrolled, MouseMoved,
		KeyPressed, KeyReleased,
	};

	enum class EventFlags : uint8_t
	{
		None = 0x00,
		EventFlagsApplication = 0x01,
		EventFlagsInput = 0x02,
		EventFlagsMouse = 0x04,
		EventFlagsMouseButton = 0x08,
		EventFlagsKeyboard = 0x10,
	};

#define EVENT_TYPE(type) static EventType GetStaticType() { return type; }\
						 virtual EventType GetEventType() const override { return GetStaticType(); }

#define EVENT_FLAGS_TYPE(flags) virtual uint8_t GetEventFlags() const override { return static_cast<uint8_t>(flags); }

	class Event
	{
	public:
		virtual EventType GetEventType() const = 0;
		virtual uint8_t GetEventFlags() const = 0;
		virtual std::string ToString() const = 0;

		inline bool IsInCategory(EventFlags flags)
		{
			return GetEventFlags() & static_cast<uint8_t>(flags);
		}
	public:
		bool Handled = false; // This will allow certain layers to handle the event and stop the event from further propagating
	};

	/*
	class EventDispatcher
	{
		template<typename T>
		using EventFn = std::function<bool(T&)>;
	public:
		EventDispatcher(Event &event) : m_Event(event) {}

		template<typename T>
		bool Dispatch(EventFn<T> func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.m_Handled = func(*(T*)&m_Event);
				return true;
			}
			return false;
		}
	private:
		Event &m_Event;
	};
	*/

	inline std::ostream& operator<<(std::ostream &os, const Event &e)
	{
		return os << e.ToString();
	}

}
