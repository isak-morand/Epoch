#pragma once
#include "Window.h"

struct GLFWwindow;

namespace Epoch
{
	class GLFWWindow : public Window
	{
	public:
		GLFWWindow() = delete;
		GLFWWindow(const WindowProperties& aProps);
		~GLFWWindow() override;

		void PollEvents() override;

		void* GetGLFWWindow() const override { return (void*)myWindow; }
		void* GetNativeWindow() const override;

		void HideWindow() const override;
		void ShowWindow() const override;

		void SetCursorMode(CursorMode aMode) override;
		CursorMode GetCursorMode() override;
		void GetCursorPosition(double& outX, double& outY) const override;

	private:
		GLFWwindow* myWindow;
	};
}
