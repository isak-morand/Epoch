#include "epch.h"
#include "Input.h"
#include <GLFW/glfw3.h>
#include "EpochCore/Window/Window.h"
#include "EpochCore/Assert.h"
#include "EpochCore/Events/KeyEvents.h"
#include "EpochCore/Events/MouseEvents.h"

namespace Epoch
{
	void Input::Initialize(Window* aWindow)
	{
		staticWindow = aWindow;
	}

	void Input::Update()
	{
		EPOCH_PROFILE_FUNC();

		ClearReleasedKeys();
		TransitionPressedKeys();
		TransitionPressedButtons();

		// Cleanup disconnected controller
		for (auto it = staticControllers.begin(); it != staticControllers.end(); )
		{
			int id = it->first;
			if (glfwJoystickPresent(id) != GLFW_TRUE)
			{
				it = staticControllers.erase(it);
			}
			else
			{
				it++;
			}
		}

		// Update controllers
		for (int id = GLFW_JOYSTICK_1; id < GLFW_JOYSTICK_LAST; id++)
		{
			if (glfwJoystickPresent(id) == GLFW_TRUE)
			{
				if (!IsControllerPresent(id))
				{
					Controller& controller = staticControllers[id];
					controller.DeadZones[0] = 0.1f;
					controller.DeadZones[1] = 0.1f;
					controller.DeadZones[2] = 0.1f;
					controller.DeadZones[3] = 0.1f;
				}

				Controller& controller = staticControllers[id];
				controller.Id = id;
				controller.Name = glfwGetJoystickName(id);

				int buttonCount;
				const unsigned char* buttons = glfwGetJoystickButtons(id, &buttonCount);
				for (int i = 0; i < buttonCount; i++)
				{
					GamepadButton button = (GamepadButton)i;

					if (buttons[i] == GLFW_PRESS &&
						!(controller.ButtonStates[button].State == KeyState::Pressed || controller.ButtonStates[button].State == KeyState::Held))
					{
						controller.ButtonStates[button].State = KeyState::Pressed;
					}
					else if (buttons[i] == GLFW_RELEASE &&
						(controller.ButtonStates[button].State == KeyState::Pressed || controller.ButtonStates[button].State == KeyState::Held))
					{
						controller.ButtonStates[button].State = KeyState::Released;
					}
				}

				int axisCount;
				const float* axes = glfwGetJoystickAxes(id, &axisCount);
				for (int i = 0; i < axisCount; i++)
				{
					controller.AxisStates[i] = abs(axes[i]) > controller.DeadZones[i] ? axes[i] : 0.0f;
				}

				int hatCount;
				const unsigned char* hats = glfwGetJoystickHats(id, &hatCount);
				for (int i = 0; i < hatCount; i++)
				{
					controller.HatStates[i] = hats[i];
				}
				
				//Left & right trigger press and release (has to be done manually)
				{
					auto updateButtonState = [&](GamepadAxis aAxis, GamepadButton aButton)
						{
							controller.AxisStates[(int)aAxis] = CU::Math::Remap01(controller.AxisStates[(int)aAxis], -1.0f, 1.0f);

							if (controller.AxisStates[(int)aAxis] > 0.0f &&
								controller.ButtonStates[aButton].State == KeyState::None)
							{
								controller.ButtonStates[aButton].State = KeyState::Pressed;
							}

							if (controller.AxisStates[(int)aAxis] <= 0.01f &&
								controller.ButtonStates[aButton].State == KeyState::Held)
							{
								controller.ButtonStates[aButton].State = KeyState::Released;
							}
						};

					updateButtonState(GamepadAxis::LeftTrigger, GamepadButton::LeftTrigger);
					updateButtonState(GamepadAxis::RightTrigger, GamepadButton::RightTrigger);
				}
			}
		}

		// Update mouse delta
		const CU::Vector2f mousePos = GetMousePositionInternal();
		staticMouseDelta = (staticLastMousePos - mousePos);
		staticLastMousePos = mousePos;
	}

	void Input::OnEvent(Event& aEvent)
	{
		EventDispatcher dispatcher(aEvent);

		if (aEvent.IsInCategory(EventCategory::Keyboard))
		{
			dispatcher.Dispatch<KeyPressedEvent>([](KeyPressedEvent& e) { UpdateKeyState(e.GetKeyCode(), KeyState::Pressed); return false; });
			dispatcher.Dispatch<KeyReleasedEvent>([](KeyReleasedEvent& e) { UpdateKeyState(e.GetKeyCode(), KeyState::Released); return false; });
		}
		else if (aEvent.IsInCategory(EventCategory::MouseButton))
		{
			dispatcher.Dispatch<MouseButtonPressedEvent>([](MouseButtonPressedEvent& e) { UpdateButtonState(e.GetMouseButton(), KeyState::Pressed); return false; });
			dispatcher.Dispatch<MouseButtonReleasedEvent>([](MouseButtonReleasedEvent& e) { UpdateButtonState(e.GetMouseButton(), KeyState::Released); return false; });
		}
		else
		{
			dispatcher.Dispatch<MouseScrolledEvent>([](MouseScrolledEvent& e) { SetScrollValues({ e.GetXOffset(), e.GetYOffset() }); return false; });
		}
	}

	bool Input::IsKeyPressed(KeyCode aKeyCode)
	{
		return staticKeyData.find(aKeyCode) != staticKeyData.end() && staticKeyData[aKeyCode].State == KeyState::Pressed;
	}

	bool Input::IsKeyHeld(KeyCode aKeyCode)
	{
		return staticKeyData.find(aKeyCode) != staticKeyData.end() &&
			(staticKeyData[aKeyCode].State == KeyState::Held || staticKeyData[aKeyCode].State == KeyState::Pressed);
	}

	bool Input::IsKeyReleased(KeyCode aKeyCode)
	{
		return staticKeyData.find(aKeyCode) != staticKeyData.end() && staticKeyData[aKeyCode].State == KeyState::Released;
	}

	bool Input::IsMouseButtonPressed(MouseButton aButton)
	{
		return staticMouseData.find(aButton) != staticMouseData.end() && staticMouseData[aButton].State == KeyState::Pressed;
	}

	bool Input::IsMouseButtonHeld(MouseButton aButton)
	{
		return staticMouseData.find(aButton) != staticMouseData.end() &&
			(staticMouseData[aButton].State == KeyState::Held || staticMouseData[aButton].State == KeyState::Pressed);
	}

	bool Input::IsMouseButtonReleased(MouseButton aButton)
	{
		return staticMouseData.find(aButton) != staticMouseData.end() && staticMouseData[aButton].State == KeyState::Released;
	}

	float Input::GetMouseX()
	{
		return GetMousePosition().x;
	}

	float Input::GetMouseY()
	{
		return GetMousePosition().y;
	}

	CU::Vector2f Input::GetMousePosition()
	{
		return staticLastMousePos;
	}

	CU::Vector2f Input::GetMousePositionInternal()
	{
		EPOCH_ASSERT(staticWindow, "Input system not initialized with window!");

		double x, y;
		staticWindow->GetCursorPosition(x, y);
		return { (float)x, (float)y };
	}

	void Input::SetCursorMode(CursorMode aMode)
	{
		EPOCH_ASSERT(staticWindow, "Input system not initialized with window!");

		staticWindow->SetCursorMode(aMode);
	}

	CursorMode Input::GetCursorMode()
	{
		EPOCH_ASSERT(staticWindow, "Input system not initialized with window!");

		return staticWindow->GetCursorMode();
	}

	bool Input::IsControllerPresent(int aId)
	{
		return staticControllers.find(aId) != staticControllers.end();
	}

	std::vector<int> Input::GetConnectedControllerIDs()
	{
		std::vector<int> ids;
		ids.reserve(staticControllers.size());
		for (auto [id, controller] : staticControllers)
		{
			ids.emplace_back(id);
		}

		return ids;
	}

	const Controller* Input::GetController(int aId)
	{
		if (!Input::IsControllerPresent(aId))
		{
			return nullptr;
		}

		return &staticControllers.at(aId);
	}

	std::string_view Input::GetControllerName(int aId)
	{
		if (!Input::IsControllerPresent(aId))
		{
			return {};
		}

		return staticControllers.at(aId).Name;
	}

	bool Input::IsControllerButtonPressed(int aControllerID, GamepadButton aButton)
	{
		if (!Input::IsControllerPresent(aControllerID))
		{
			return false;
		}

		auto& contoller = staticControllers.at(aControllerID);
		return contoller.ButtonStates.find(aButton) != contoller.ButtonStates.end() && contoller.ButtonStates[aButton].State == KeyState::Pressed;
	}

	bool Input::IsControllerButtonHeld(int aControllerID, GamepadButton aButton)
	{
		if (!Input::IsControllerPresent(aControllerID))
		{
			return false;
		}

		auto& contoller = staticControllers.at(aControllerID);
		
		if (auto it = contoller.ButtonStates.find(aButton); it != contoller.ButtonStates.end())
		{
			if (it->second.State == KeyState::Held || it->second.State == KeyState::Pressed)
			{
				return true;
			}
		}
		return false;
		
		//return contoller.buttonStates.find(aButton) != contoller.buttonStates.end() && contoller.buttonStates[aButton].state == KeyState::Held;
	}

	bool Input::IsControllerButtonReleased(int aControllerID, GamepadButton aButton)
	{
		if (!Input::IsControllerPresent(aControllerID))
		{
			return false;
		}

		auto& contoller = staticControllers.at(aControllerID);
		return contoller.ButtonStates.find(aButton) != contoller.ButtonStates.end() && contoller.ButtonStates[aButton].State == KeyState::Released;
	}

	float Input::GetControllerAxis(int aControllerID, GamepadAxis aAxis)
	{
		if (!Input::IsControllerPresent(aControllerID))
		{
			return 0.0f;
		}

		int axis = (int)aAxis;
		const Controller& controller = staticControllers.at(aControllerID);
		if (controller.AxisStates.find(axis) == controller.AxisStates.end())
		{
			return 0.0f;
		}

		return controller.AxisStates.at(axis);
	}

	uint8_t Input::GetControllerHat(int aControllerID, int aHat)
	{
		if (!Input::IsControllerPresent(aControllerID))
		{
			return 0;
		}

		const Controller& controller = staticControllers.at(aControllerID);
		if (controller.HatStates.find(aHat) == controller.HatStates.end())
		{
			return 0;
		}

		return controller.HatStates.at(aHat);
	}

	float Input::GetControllerDeadzone(int aControllerID, GamepadAxis aAxis)
	{
		if (!Input::IsControllerPresent(aControllerID))
		{
			return 0.0f;
		}

		const Controller& controller = staticControllers.at(aControllerID);
		return controller.DeadZones.at((int)aAxis);
	}

	void Input::SetControllerDeadzone(int aControllerID, GamepadAxis aAxis, float aDeadzone)
	{
		if (!Input::IsControllerPresent(aControllerID))
		{
			return;
		}

		Controller& controller = staticControllers.at(aControllerID);
		controller.DeadZones[(int)aAxis] = aDeadzone;
	}

	void Input::TransitionPressedKeys()
	{
		for (const auto& [key, keyData] : staticKeyData)
		{
			if (keyData.State == KeyState::Pressed)
			{
				UpdateKeyState(key, KeyState::Held);
			}
		}
	}

	void Input::TransitionPressedButtons()
	{
		for (const auto& [button, buttonData] : staticMouseData)
		{
			if (buttonData.State == KeyState::Pressed)
			{
				UpdateButtonState(button, KeyState::Held);
			}
		}

		for (const auto& [id, controller] : staticControllers)
		{
			for (const auto& [button, buttonStates] : controller.ButtonStates)
			{
				if (buttonStates.State == KeyState::Pressed)
				{
					UpdateControllerButtonState(id, button, KeyState::Held);
				}
			}
		}
	}

	void Input::UpdateKeyState(KeyCode aKey, KeyState aNewState)
	{
		auto& keyData = staticKeyData[aKey];
		keyData.Key = aKey;
		keyData.LastState = keyData.State;
		keyData.State = aNewState;
	}

	void Input::UpdateButtonState(MouseButton aButton, KeyState aNewState)
	{
		auto& mouseData = staticMouseData[aButton];
		mouseData.Button = aButton;
		mouseData.LastState = mouseData.State;
		mouseData.State = aNewState;
	}

	void Input::UpdateControllerButtonState(int aController, GamepadButton aButton, KeyState aNewState)
	{
		auto& controllerButtonData = staticControllers.at(aController).ButtonStates.at(aButton);
		controllerButtonData.Button = aButton;
		controllerButtonData.LastState = controllerButtonData.State;
		controllerButtonData.State = aNewState;
	}

	void Input::ClearReleasedKeys()
	{
		for (const auto& [key, keyData] : staticKeyData)
		{
			if (keyData.State == KeyState::Released)
			{
				UpdateKeyState(key, KeyState::None);
			}
		}

		for (const auto& [button, buttonData] : staticMouseData)
		{
			if (buttonData.State == KeyState::Released)
			{
				UpdateButtonState(button, KeyState::None);
			}
		}

		for (const auto& [id, controller] : staticControllers)
		{
			for (const auto& [button, buttonStates] : controller.ButtonStates)
			{
				if (buttonStates.State == KeyState::Released)
				{
					UpdateControllerButtonState(id, button, KeyState::None);
				}
			}
		}
	}

	void Input::SetScrollValues(const CU::Vector2f& aScrollValues)
	{
		staticMouseScroll = aScrollValues;
	}
}
