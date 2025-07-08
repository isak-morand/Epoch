#pragma once
#include <memory>
#include <functional>
#include <queue>
#include <CommonUtilities/Timer.h>
#include <EpochCore/Events/Event.h>
#include <EpochCore/Events/WindowEvents.h>
#include <EpochCore/Window/Window.h>
#include "AppLayer/LayerStack.h"

namespace Epoch
{
	namespace Rendering
	{
		class IRenderer;
		class IImGuiRenderer;
	}

	struct EngineSpecification
	{
		WindowProperties WindowProperties;

		bool ImGuiEnabled = false;
	};

	class Engine
	{
	public:
		using UpdateCallback = std::function<void()>;
		using InitCallback = std::function<void()>;
		using ShutdownCallback = std::function<void()>;

	public:
		Engine(EngineSpecification aSpecification = EngineSpecification());
		~Engine();

		static Engine* GetInstance() { return myInstance; }

		void SetInitCallback(InitCallback cb) { myInitCallback = std::move(cb); }
		void SetUpdateCallback(UpdateCallback cb) { myUpdateCallback = std::move(cb); }
		void SetShutdownCallback(ShutdownCallback cb) { myShutdownCallback = std::move(cb); }

		LayerStack& GetLayerStack() { return myLayerStack; }

		void Run();

		void OnEvent(Event& aEvent);

		template<typename Func>
		void QueueEvent(Func&& aFunc)
		{
			std::scoped_lock<std::mutex> lock(myEventQueueMutex);
			myEventQueue.emplace(aFunc);
		}

		Rendering::IRenderer* GetRendererInterface() { return myRendererInterface.get(); }

		float GetDeltaTime() const { return myDeltaTime; }
		float GetTotalTime() const { return myTimer.Elapsed(); }

	private:
		void Initialize();
		void Shutdown();

		void ProcessEvents();

		bool OnWindowClose(const WindowCloseEvent& aEvent);
		bool OnWindowResize(const WindowResizeEvent& aEvent);
		bool OnWindowMinimize(const WindowMinimizeEvent& aEvent);

	private:
		static inline Engine* myInstance = nullptr;

		EngineSpecification mySpecification;

		bool myIsRunning = false;
		bool myWindowMinimized = false;

		InitCallback myInitCallback;
		UpdateCallback myUpdateCallback;
		ShutdownCallback myShutdownCallback;

		std::mutex myEventQueueMutex;
		std::queue<std::function<void()>> myEventQueue;

		CU::Timer myTimer;
		float myDeltaTime = 0.0f;
		float myLastTime = 0.0f;

		LayerStack myLayerStack;

		std::unique_ptr<Window> myWindow;

		std::unique_ptr<Rendering::IRenderer> myRendererInterface;
		std::unique_ptr<Rendering::IImGuiRenderer> myImGuiRendererInterface;
	};
}
