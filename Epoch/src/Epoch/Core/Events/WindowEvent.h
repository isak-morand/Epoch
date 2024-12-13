#pragma once
#include "Event.h"

namespace Epoch
{
	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() {}

		EVENT_CLASS_TYPE(WindowClose)
	};

	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(unsigned int aWidth, unsigned int aHeight) : myWidth(aWidth), myHeight(aHeight) {}

		inline uint32_t GetWidth() const { return myWidth; }
		inline uint32_t GetHeight() const { return myHeight; }

		EVENT_CLASS_TYPE(WindowResize)

	private:
		unsigned int myWidth, myHeight;
	};

	class WindowMinimizeEvent : public Event
	{
	public:
		WindowMinimizeEvent(bool minimized) : myMinimized(minimized) {}

		bool IsMinimized() const { return myMinimized; }

		EVENT_CLASS_TYPE(WindowMinimize)

	private:
		bool myMinimized = false;
	};
}