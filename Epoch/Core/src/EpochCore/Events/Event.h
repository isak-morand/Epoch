#pragma once
#include <functional>

namespace Epoch
{
	enum class EventType
	{
		None,
		WindowClose, WindowMinimize, WindowResize,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum class EventCategory
	{
		None			= 0,
		Application		= 1 << 0,
		Input			= 1 << 1,
		Keyboard		= 1 << 2,
		Mouse			= 1 << 3,
		MouseButton		= 1 << 4,
		Physics			= 1 << 5
	};

	inline EventCategory operator|(EventCategory a, EventCategory b)
	{
		return static_cast<EventCategory>(static_cast<int>(a) | static_cast<int>(b));
	}

#define EVENT_CLASS_TYPE(aType) static EventType GetStaticType() { return EventType::aType; }\
								EventType GetEventType() const override { return GetStaticType(); }

#define EVENT_CLASS_CATEGORY(aCategory) int GetCategoryFlags() const override { return static_cast<int>(aCategory); }

	class Event
	{
	public:
		Event() = default;
		virtual ~Event() = default;

		bool IsHandled() const { return myIsHandled; }

		virtual EventType GetEventType() const = 0;
		virtual int GetCategoryFlags() const = 0;

		bool IsInCategory(EventCategory aCategory) const
		{
			return GetCategoryFlags() & static_cast<int>(aCategory);
		}

	private:
		bool myIsHandled = false;

		friend class EventDispatcher;
	};

	class EventDispatcher
	{
		template<typename T>
		using EventFn = std::function<bool(T&)>;
	public:
		EventDispatcher(Event& aEvent) : myEvent(aEvent) {}

		template<typename T>
		bool Dispatch(EventFn<T> aFunc)
		{
			if (myEvent.GetEventType() != T::GetStaticType() || myEvent.myIsHandled)
			{
				return false;
			}

			myEvent.myIsHandled = aFunc(*(T*)&myEvent);
			return true;
		}
	private:
		Event& myEvent;
	};
}
