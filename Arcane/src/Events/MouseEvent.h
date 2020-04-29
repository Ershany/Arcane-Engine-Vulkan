#pragma once

#include "Event.h"

namespace Arcane
{

	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float x, float y) : m_MouseX(x), m_MouseY(y) {}

		EVENT_TYPE(EventType::MouseMoved);
		EVENT_FLAGS_TYPE(static_cast<uint8_t>(EventFlags::EventFlagsInput) | static_cast<uint8_t>(EventFlags::EventFlagsMouse));

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent - MouseX:" << m_MouseX << ", MouseY:" << m_MouseY;
			return ss.str();
		}
	private:
		float m_MouseX, m_MouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float xOffset, float yOffset) : m_XOffset(xOffset), m_YOffset(yOffset) {}

		EVENT_TYPE(EventType::MouseScrolled);
		EVENT_FLAGS_TYPE(static_cast<uint8_t>(EventFlags::EventFlagsInput) | static_cast<uint8_t>(EventFlags::EventFlagsMouse));

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent - XOffset:" << m_XOffset << ", YOffset:" << m_YOffset;
			return ss.str();
		}
	private:
		float m_XOffset, m_YOffset;
	};

	class MouseButtonEvent : public Event
	{
	public:
		inline int GetMouseButton() const { return m_Button; }

		EVENT_FLAGS_TYPE(static_cast<uint8_t>(EventFlags::EventFlagsInput) | static_cast<uint8_t>(EventFlags::EventFlagsMouse) | static_cast<uint8_t>(EventFlags::EventFlagsMouseButton));
	protected:
		MouseButtonEvent(int mouseButton) : m_Button(mouseButton) {}

		int m_Button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(int mouseButton) : MouseButtonEvent(mouseButton) {}

		EVENT_TYPE(EventType::MouseButtonPressed);

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent - Button:" << m_Button;
			return ss.str();
		}
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(int mouseButton) : MouseButtonEvent(mouseButton) {}

		EVENT_TYPE(EventType::MouseButtonReleased);

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent - Button:" << m_Button;
			return ss.str();
		}
	};

}
