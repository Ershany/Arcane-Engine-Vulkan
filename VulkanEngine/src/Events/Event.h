#pragma once

/*
	Events in Arcane are not being buffered, instead they are blocking
	That means every time an event gets dispatched it must be processed
	TODO: Buffer Events and process during the event part of the update stage
*/

enum class EventType
{
	None = 0,
	ApplicationUpdate, ApplicationRender, ApplicationTick,
	WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
	MouseButtonPressed, MouseButtonReleased, MouseScrolled, MouseMoved,
	KeyPressed, KeyReleased,
};

enum class EventFlags
{
	None = 0x00,
	EventFlagsApplication = 0x01,
	EventFlagsInput = 0x02,
	EventFlagsMouse = 0x04,
	EventFlagsMouseButton = 0x08,
	EventFlagsKeyboard = 0x10,
};

class EventDispatcher
{
public:

};

class Event
{
public:

};
