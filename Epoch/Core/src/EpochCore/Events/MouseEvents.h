#pragma once
#include "Event.h"
#include "EpochCore/Input/KeyCodes.h"

namespace Epoch
{
	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float x, float y) : myMouseX(x), myMouseY(y) {}

		inline float GetX() const { return myMouseX; }
		inline float GetY() const { return myMouseY; }

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategory::Input | EventCategory::Mouse)

	private:
		float myMouseX, myMouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float xOffset, float yOffset) : myXOffset(xOffset), myYOffset(yOffset) {}

		inline float GetXOffset() const { return myXOffset; }
		inline float GetYOffset() const { return myYOffset; }

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategory::Input | EventCategory::Mouse)

	private:
		float myXOffset, myYOffset;
	};

	class MouseButtonEvent : public Event
	{
	public:
		inline MouseButton GetMouseButton() const { return myButton; }

		EVENT_CLASS_CATEGORY(EventCategory::Input | EventCategory::Mouse | EventCategory::MouseButton)

	protected:
		MouseButtonEvent(MouseButton aButton) : myButton(aButton) {}

		MouseButton myButton;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(MouseButton aButton) : MouseButtonEvent(aButton) {}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(MouseButton aButton) : MouseButtonEvent(aButton) {}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};
}
