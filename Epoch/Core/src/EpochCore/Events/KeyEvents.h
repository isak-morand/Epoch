#pragma once
#include "Event.h"
#include "EpochCore/Input/KeyCodes.h"

namespace Epoch
{
	class KeyEvent : public Event
	{
	public:
		inline KeyCode GetKeyCode() const { return myKeyCode; }

		EVENT_CLASS_CATEGORY(EventCategory::Input | EventCategory::Keyboard)

	protected:
		KeyEvent(KeyCode aKeycode) : myKeyCode(aKeycode) {}

		KeyCode myKeyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(KeyCode aKeycode) : KeyEvent(aKeycode) {}

		EVENT_CLASS_TYPE(KeyPressed)
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(KeyCode aKeycode) : KeyEvent(aKeycode) {}

		EVENT_CLASS_TYPE(KeyReleased)
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(KeyCode aKeycode) : KeyEvent(aKeycode) {}

		EVENT_CLASS_TYPE(KeyTyped)
	};
}