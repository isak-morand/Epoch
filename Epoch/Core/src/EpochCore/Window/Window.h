#pragma once
#include <string>
#include <functional>
#include <memory>
#include "EpochCore/Input/KeyCodes.h"

namespace Epoch
{
	class Event;

	struct WindowProperties
	{
		std::string Title = "Epoch Application";
		unsigned Width = 1280;
		unsigned Height = 720;
		bool StartHidden = false;
	};

	class Window
	{
		using EventCallbackFn = std::function<void(Event&)>;
	public:
		virtual ~Window();

		static std::unique_ptr<Window> Create(const WindowProperties& aProps = WindowProperties());

		void SetEventCallback(const EventCallbackFn& aCallback) { myData.EventCallback = aCallback; }

		uint32_t GetWidth() const { return myData.Width; }
		uint32_t GetHeight() const { return myData.Height; }
		const std::string& GetTitle() const { return myData.Title; }

		virtual void PollEvents() = 0;

		virtual void* GetNativeWindow() const = 0;
		virtual void* GetGLFWWindow() const = 0;

		virtual void HideWindow() const = 0;
		virtual void ShowWindow() const = 0;

		virtual void SetCursorMode(CursorMode aMode) = 0;
		virtual CursorMode GetCursorMode() = 0;
		virtual void GetCursorPosition(double& outX, double& outY) const = 0;

	protected:
		Window();

	protected:
		struct WindowData
		{
			std::string Title = "";
			uint32_t Width = 0;
			uint32_t Height = 0;

			EventCallbackFn EventCallback;
		} myData;
	};
}
