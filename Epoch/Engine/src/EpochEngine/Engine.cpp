#include "Engine.h"
#include "EpochCore/Input/Input.h"
#include "EpochCore/Profiler.h"
#include "EpochCore/FileSystem.h"
#include <EpochRendering/IRenderer.h>
#include <EpochRendering/IImGuiRenderer.h>
#include <EpochProjects/Project.h>

namespace Epoch
{
	Engine::Engine(EngineSpecification aSpecification)
	{
		if (myInstance)
		{
			LOG_WARNING("Multiple instances of the engine isn't supported.");
			return;
		}

		myInstance = this;

		mySpecification = aSpecification;
	}

	Engine::~Engine() = default;

	void Engine::Run()
	{
		myIsRunning = true;
		bool firstFrame = true;

		Initialize();

		while (myIsRunning)
		{
			EPOCH_PROFILE_SCOPE("Frame");

			Input::Update();
			ProcessEvents();

			if (!myWindowMinimized)
			{
				myRendererInterface->BeginFrame();

				{
					EPOCH_PROFILE_SCOPE("Engine::Run: Layer stack update");
					for (const auto& layer : myLayerStack)
					{
						layer->OnUpdate();
					}
				}

				if (myUpdateCallback)
				{
					EPOCH_PROFILE_SCOPE("Engine::Run: Update Callback");
					myUpdateCallback();
				}

				if (mySpecification.ImGuiEnabled)
				{
					EPOCH_PROFILE_SCOPE("Engine::Run: Layer stack render ImGui");
					for (const auto& layer : myLayerStack)
					{
						layer->OnRenderImGui();
					}
				}

				myRendererInterface->EndFrame();
			}

			float time = myTimer.Elapsed();
			myDeltaTime = time - myLastTime;
			myLastTime = time;

			//TODO: Rework, hack to make avoid white first frame
			if (firstFrame)
			{
				firstFrame = false;
				myWindow->ShowWindow();
			}

			EPOCH_PROFILE_MARK_FRAME;
		}

		Shutdown();
	}

	void Engine::OnEvent(Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);

		if (aEvent.IsInCategory(EventCategory::Application))
		{
			dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) { return OnWindowClose(e); });
			dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) { return OnWindowResize(e); });
			dispatcher.Dispatch<WindowMinimizeEvent>([this](WindowMinimizeEvent& e) { return OnWindowMinimize(e); });
		}
		else if (aEvent.IsInCategory(EventCategory::Input))
		{
			Input::OnEvent(aEvent);
		}

		for (auto it = myLayerStack.rbegin(); it != myLayerStack.rend(); ++it)
		{
			(*it)->OnEvent(aEvent);
			if (aEvent.IsHandled()) break;
		}
	}

	void Engine::Initialize()
	{
		myWindow = Window::Create(mySpecification.WindowProperties);
		myWindow->SetEventCallback([this](Event& aEvent) { OnEvent(aEvent); });
		myWindow->HideWindow();

		myRendererInterface = Rendering::IRenderer::Create();
		if (!myRendererInterface->Initialize(myWindow.get()))
		{
			EPOCH_ASSERT(false, "Failed to initialize the renderer!");
		}

		if (mySpecification.ImGuiEnabled)
		{
			myImGuiRendererInterface = Rendering::IImGuiRenderer::Create();
			if (!myImGuiRendererInterface->Initialize(myWindow.get()))
			{
				EPOCH_ASSERT(false, "Failed to initialize the ImGui renderer!");
			}
		}

		Input::Initialize(myWindow.get());

		Core::FileSystem::Initialize();

		if (myInitCallback)
		{
			myInitCallback();
		}
	}

	void Engine::Shutdown()
	{
		if (myShutdownCallback)
		{
			myShutdownCallback();
		}

		Core::FileSystem::Shutdown();
	}

	void Engine::ProcessEvents()
	{
		EPOCH_PROFILE_FUNC();

		myWindow->PollEvents();

		std::queue<std::function<void()>> eventQueue;
		{
			std::scoped_lock<std::mutex> lock(myEventQueueMutex);
			std::swap(eventQueue, myEventQueue);
		}
		while (eventQueue.size() > 0)
		{
			auto& func = eventQueue.front();
			func();
			eventQueue.pop();
		}
	}

	bool Engine::OnWindowClose(const WindowCloseEvent& aEvent)
	{
		myIsRunning = false;
		return false;
	}

	bool Engine::OnWindowResize(const WindowResizeEvent& aEvent)
	{
		const uint32_t width = aEvent.GetWidth(), height = aEvent.GetHeight();
		if (width > 0 && height > 0)
		{
			QueueEvent([renderer = myRendererInterface.get(), width, height]() { renderer->OnResize(width, height); });
		}

		return false;
	}

	bool Engine::OnWindowMinimize(const WindowMinimizeEvent& aEvent)
	{
		myWindowMinimized = aEvent.IsMinimized();
		return false;
	}

}
