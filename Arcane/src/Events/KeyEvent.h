#pragma once

#include "Event.h"

namespace Arcane
{

	class KeyEvent : public Event
	{
	protected:
		KeyEvent(int keyCode) : m_KeyCode(keyCode) {}
	public:
		EVENT_FLAGS_TYPE(static_cast<uint8_t>(EventFlags::EventFlagsInput) | static_cast<uint8_t>(EventFlags::EventFlagsKeyboard));

		inline int GetKeyCode() const { return m_KeyCode; }
	protected:
		int m_KeyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(int keyCode, int repeatCount) : KeyEvent(keyCode), m_RepeatCount(repeatCount) {}

		EVENT_TYPE(EventType::KeyPressed);

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent -  KeyCode:" << GetKeyCode() << ", RepeatedCount:" << m_RepeatCount;
			return ss.str();
		}

		inline int GetRepeatCount() const { return m_RepeatCount; }
	private:
		int m_RepeatCount;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(int keyCode) : KeyEvent(keyCode) {}

		EVENT_TYPE(EventType::KeyReleased);

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent - KeyCode:" << GetKeyCode();
			return ss.str();
		}
	};

}
