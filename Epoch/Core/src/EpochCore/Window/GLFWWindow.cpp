#include "epch.h"
#include "GLFWWindow.h"

#include <GLFW/glfw3.h>
#ifdef PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>
#endif

#include "EpochCore/Events/WindowEvents.h"
#include "EpochCore/Log.h"
#include "EpochCore/Assert.h"
#include <EpochCore/Events/MouseEvents.h>
#include <EpochCore/Events/KeyEvents.h>

namespace Epoch
{
	static void GLFWErrorCallback(int error, const char* description)
	{
		LOG_ERROR("GLFW Error ({}): {}", error, description);
	}

	GLFWWindow::GLFWWindow(const WindowProperties& aProps)
	{
		myData.Width = aProps.Width;
		myData.Height = aProps.Height;
		myData.Title = aProps.Title;

		{
			EPOCH_ENSURE(glfwInit(), "Could not initialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
		}

		{
			LOG_INFO("Creating Window: {} ({}, {})", myData.Title, myData.Width, myData.Height);
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
			glfwWindowHint(GLFW_VISIBLE, aProps.StartHidden ? GLFW_FALSE : GLFW_TRUE);
			myWindow = glfwCreateWindow((int)myData.Width, (int)myData.Height, myData.Title.c_str(), nullptr, nullptr);
		}

		glfwSetWindowUserPointer(myWindow, &myData);

		if (glfwRawMouseMotionSupported())
		{
			glfwSetInputMode(myWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}
		else
		{
			LOG_WARNING("Raw mouse motion not supported.");
		}

		//Callbacks
		{
			glfwSetWindowCloseCallback(myWindow, [](GLFWwindow* window)
			{
				auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

				WindowCloseEvent event;
				data.EventCallback(event);
			});

			glfwSetWindowSizeCallback(myWindow, [](GLFWwindow* window, int width, int height)
			{
				auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

				WindowResizeEvent event((uint32_t)width, (uint32_t)height);
				data.Width = width;
				data.Height = height;
				data.EventCallback(event);
			});

			glfwSetWindowIconifyCallback(myWindow, [](GLFWwindow* window, int iconified)
			{
				auto& data = *((WindowData*)glfwGetWindowUserPointer(window));
				WindowMinimizeEvent event((bool)iconified);
				data.EventCallback(event);
			});

			//glfwSetWindowFocusCallback(myWindow, [](GLFWwindow* window, int focused)
			//{
			//
			//});

			//glfwSetDropCallback(myWindow, [](GLFWwindow* window, int path_count, const char* paths[])
			//{
			//	auto& data = *((WindowData*)glfwGetWindowUserPointer(window));
			//
			//	std::vector<std::string> droppedPaths(path_count);
			//	for (size_t i = 0; i < path_count; i++)
			//	{
			//		droppedPaths[i] = std::string(paths[i]);
			//	}
			//
			//	EditorFileDroppedEvent event(droppedPaths);
			//	data.eventCallback(event);
			//});

			glfwSetKeyCallback(myWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

				switch (action)
				{
					case GLFW_PRESS:
					{
						KeyPressedEvent event((KeyCode)key);
						data.EventCallback(event);
						break;
					}
					case GLFW_RELEASE:
					{
						KeyReleasedEvent event((KeyCode)key);
						data.EventCallback(event);
						break;
					}
				}
			});

			glfwSetCharCallback(myWindow, [](GLFWwindow* window, uint32_t codepoint)
			{
				auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

				KeyTypedEvent event((KeyCode)codepoint);
				data.EventCallback(event);
			});

			glfwSetMouseButtonCallback(myWindow, [](GLFWwindow* window, int button, int action, int mods)
			{
				auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

				switch (action)
				{
					case GLFW_PRESS:
					{
						MouseButtonPressedEvent event((MouseButton)button);
						data.EventCallback(event);
						break;
					}
					case GLFW_RELEASE:
					{
						MouseButtonReleasedEvent event((MouseButton)button);
						data.EventCallback(event);
						break;
					}
				}
			});

			glfwSetScrollCallback(myWindow, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

				MouseScrolledEvent event((float)xOffset, (float)yOffset);
				data.EventCallback(event);
			});

			glfwSetCursorPosCallback(myWindow, [](GLFWwindow* window, double x, double y)
			{
				auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

				MouseMovedEvent event((float)x, (float)y);
				data.EventCallback(event);
			});

			//glfwSetCursorEnterCallback(myWindow, [](GLFWwindow* window, int entered)
			//{
			//
			//});
		}

		// Update window size to actual size
		{
			int width, height;
			glfwGetWindowSize(myWindow, &width, &height);
			myData.Width = width;
			myData.Height = height;
		}
	}

	GLFWWindow::~GLFWWindow()
	{
		glfwDestroyWindow(myWindow);
		glfwTerminate();
	}

	void GLFWWindow::PollEvents()
	{
		glfwPollEvents();
	}

	void* GLFWWindow::GetNativeWindow() const
	{
#ifdef PLATFORM_WINDOWS
		return (void*)glfwGetWin32Window(myWindow);
#else
		return nullptr;
#endif
	}

	void GLFWWindow::HideWindow() const
	{
		glfwHideWindow(myWindow);
	}

	void GLFWWindow::ShowWindow() const
	{
		glfwShowWindow(myWindow);
	}

	void GLFWWindow::SetCursorMode(CursorMode aMode)
	{
		if (aMode == CursorMode::Locked)
		{
			glfwSetInputMode(myWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetInputMode(myWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}
		else if (aMode == CursorMode::Hidden)
		{
			glfwSetInputMode(myWindow, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
			glfwSetInputMode(myWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}
		else // CursorMode::Normal
		{
			glfwSetInputMode(myWindow, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
			glfwSetInputMode(myWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}

	CursorMode GLFWWindow::GetCursorMode()
	{
		return (CursorMode)(glfwGetInputMode(myWindow, GLFW_CURSOR) - GLFW_CURSOR_NORMAL);
	}

	void GLFWWindow::GetCursorPosition(double& outX, double& outY) const
	{
		glfwGetCursorPos(myWindow, &outX, &outY);
	}
}
