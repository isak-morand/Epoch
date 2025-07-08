#pragma once
#include <map>
#include <vector>
#include <string>
#include <CommonUtilities/Math/Vector/Vector2.hpp>
#include "KeyCodes.h"
#include "EpochCore/Events/Event.h"

namespace Epoch
{
	class Window;

	struct ControllerButtonData
	{
		GamepadButton Button;
		KeyState State = KeyState::None;
		KeyState LastState = KeyState::None;
	};

	struct Controller
	{
		int Id;
		std::string Name;
		std::map<GamepadButton, ControllerButtonData> ButtonStates;
		std::map<int, float> AxisStates;
		std::map<int, float> DeadZones;
		std::map<int, uint8_t> HatStates;
	};

	struct KeyData
	{
		KeyCode Key;
		KeyState State = KeyState::None;
		KeyState LastState = KeyState::None;
	};

	struct ButtonData
	{
		MouseButton Button;
		KeyState State = KeyState::None;
		KeyState LastState = KeyState::None;
	};

	class Input
	{
	public:
		static void Initialize(Window* aWindow);
		static void Update();

		static void OnEvent(Event& aEvent);

		static bool IsKeyPressed(KeyCode aKeyCode);
		static bool IsKeyHeld(KeyCode aKeyCode);
		static bool IsKeyReleased(KeyCode aKeyCode);

		static bool IsMouseButtonPressed(MouseButton aButton);
		static bool IsMouseButtonHeld(MouseButton aButton);
		static bool IsMouseButtonReleased(MouseButton aButton);

		static float GetMouseX();
		static float GetMouseY();
		static CU::Vector2f GetMousePosition();
		static CU::Vector2f GetMouseDelta() { return staticMouseDelta; }

		static CU::Vector2f GetMouseScroll() { return staticMouseScroll; }

		static void SetCursorMode(CursorMode aMode);
		static CursorMode GetCursorMode();

		// Controllers
		static bool IsControllerPresent(int aId);
		static std::vector<int> GetConnectedControllerIDs();
		static const Controller* GetController(int aId);
		static std::string_view GetControllerName(int aId);

		static bool IsControllerButtonPressed(int aControllerID, GamepadButton aButton);
		static bool IsControllerButtonHeld(int aControllerID, GamepadButton aButton);
		static bool IsControllerButtonReleased(int aControllerID, GamepadButton aButton);
		
		static float GetControllerAxis(int aControllerID, GamepadAxis aAxis);
		static uint8_t GetControllerHat(int aControllerID, int aHat);

		static float GetControllerDeadzone(int aControllerID, GamepadAxis aAxis);
		static void SetControllerDeadzone(int aControllerID, GamepadAxis aAxis, float aDeadzone);
		
		static const std::map<int, Controller>& GetControllers() { return staticControllers; }
		
	private:
		static CU::Vector2f GetMousePositionInternal();

		static void TransitionPressedKeys();
		static void TransitionPressedButtons();

		static void UpdateKeyState(KeyCode aKey, KeyState aNewState);
		static void UpdateButtonState(MouseButton aButton, KeyState aNewState);
		static void UpdateControllerButtonState(int aController, GamepadButton aButton, KeyState aNewState);

		static void ClearReleasedKeys();
		static void SetScrollValues(const CU::Vector2f& aScrollValues);

	private:
		static inline Window* staticWindow;

		static inline std::map<KeyCode, KeyData> staticKeyData;
		static inline std::map<MouseButton, ButtonData> staticMouseData;
		static inline std::map<int, Controller> staticControllers;

		static inline CU::Vector2f staticLastMousePos;
		static inline CU::Vector2f staticMouseDelta;
		static inline CU::Vector2f staticMouseScroll;
	};
}
