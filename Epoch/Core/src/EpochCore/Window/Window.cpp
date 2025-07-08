#include "epch.h"
#include "Window.h"
#include "GLFWWindow.h"

namespace Epoch
{
	Window::Window() = default;
	Window::~Window() = default;

	std::unique_ptr<Window> Window::Create(const WindowProperties& aProps)
	{
		return std::make_unique<GLFWWindow>(aProps);
	}
}
