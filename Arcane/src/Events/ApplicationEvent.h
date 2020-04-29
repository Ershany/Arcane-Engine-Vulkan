#pragma once

#include "Event.h"

namespace Arcane
{

	class ApplicationUpdateEvent : public Event
	{
	public:
		ApplicationUpdateEvent() = default;

		EVENT_TYPE(EventType::ApplicationUpdate);
		EVENT_FLAGS_TYPE(EventFlags::EventFlagsApplication);

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "ApplicationUpdateEvent";
			return ss.str();
		}
	};

	class ApplicationTickEvent : public Event
	{
	public:
		ApplicationTickEvent() = default;

		EVENT_TYPE(EventType::ApplicationTick);
		EVENT_FLAGS_TYPE(EventFlags::EventFlagsApplication);

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "ApplicationTickEvent";
			return ss.str();
		}
	};

	class ApplicationRenderEvent : public Event
	{
	public:
		ApplicationRenderEvent() = default;

		EVENT_TYPE(EventType::ApplicationRender);
		EVENT_FLAGS_TYPE(EventFlags::EventFlagsApplication);

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "ApplicationRenderEvent";
			return ss.str();
		}
	};

	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(uint32_t width, uint32_t height) : m_Width(width), m_Height(height) {}

		EVENT_TYPE(EventType::WindowResize);
		EVENT_FLAGS_TYPE(EventFlags::EventFlagsApplication);

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent - Width:" << m_Width << ", Height:" << m_Height;
			return ss.str();
		}

		inline uint32_t GetWidth() { return m_Width; }
		inline uint32_t GetHeight() { return m_Height; }
	private:
		uint32_t m_Width, m_Height;
	};

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() = default;

		EVENT_TYPE(EventType::WindowClose);
		EVENT_FLAGS_TYPE(EventFlags::EventFlagsApplication);

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowCloseEvent";
			return ss.str();
		}
	};

}
