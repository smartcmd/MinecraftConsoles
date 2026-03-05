#pragma once

#ifdef _WINDOWS64

#include <windows.h>

class KeyboardMouseInput
{
public:
	static const int MAX_KEYS = 256;

	static const int MOUSE_LEFT = 0;
	static const int MOUSE_RIGHT = 1;
	static const int MOUSE_MIDDLE = 2;
	static const int MAX_MOUSE_BUTTONS = 3;

	static const int KEY_FORWARD = 'W';
	static const int KEY_BACKWARD = 'S';
	static const int KEY_LEFT = 'A';
	static const int KEY_RIGHT = 'D';
	static const int KEY_JUMP = VK_SPACE;
	static const int KEY_SNEAK = VK_LSHIFT;
	static const int KEY_SPRINT = VK_CONTROL;
	static const int KEY_INVENTORY = 'E';
	static const int KEY_DROP = 'Q';
	static const int KEY_CRAFTING = 'C';
	static const int KEY_CRAFTING_ALT = 'R';
	static const int KEY_CONFIRM = VK_RETURN;
	static const int KEY_CANCEL = VK_ESCAPE;
	static const int KEY_PAUSE = VK_ESCAPE;
	static const int KEY_THIRD_PERSON = VK_F5;
	static const int KEY_DEBUG_INFO = VK_F3;
	static const int KEY_DEBUG_MENU = VK_F4;

	void Init();
	void Tick();
	void ClearAllState();

	void OnKeyDown(int vkCode);
	void OnKeyUp(int vkCode);
	void OnMouseButtonDown(int button);
	void OnMouseButtonUp(int button);
	void OnMouseMove(int x, int y);
	void OnMouseWheel(int delta);
	void OnRawMouseDelta(int dx, int dy);

	bool IsKeyDown(int vkCode) const;
	bool IsKeyPressed(int vkCode) const;
	bool IsKeyReleased(int vkCode) const;

	bool IsMouseButtonDown(int button) const;
	bool IsMouseButtonPressed(int button) const;
	bool IsMouseButtonReleased(int button) const;

	int GetMouseX() const { return m_mouseX; }
	int GetMouseY() const { return m_mouseY; }

	int GetMouseDeltaX() const { return m_mouseDeltaX; }
	int GetMouseDeltaY() const { return m_mouseDeltaY; }

	int GetMouseWheel();
	int PeekMouseWheel() const { return m_mouseWheelAccum; }
	void ConsumeMouseWheel() { if (m_mouseWheelAccum != 0) m_mouseWheelConsumed = true; m_mouseWheelAccum = 0; }
	bool WasMouseWheelConsumed() const { return m_mouseWheelConsumed; }

	// Per-frame delta consumption for low-latency mouse look.
	// Reads and clears the raw accumulators (not the per-tick snapshot).
	void ConsumeMouseDelta(float &dx, float &dy);

	void SetMouseGrabbed(bool grabbed);
	bool IsMouseGrabbed() const { return m_mouseGrabbed; }

	void SetCursorHiddenForUI(bool hidden);
	bool IsCursorHiddenForUI() const { return m_cursorHiddenForUI; }

	void SetWindowFocused(bool focused);
	bool IsWindowFocused() const { return m_windowFocused; }

	bool HasAnyInput() const { return m_hasInput; }

	void SetKBMActive(bool active) { m_kbmActive = active; }
	bool IsKBMActive() const { return m_kbmActive; }

	void SetScreenCursorHidden(bool hidden) { m_screenWantsCursorHidden = hidden; }
	bool IsScreenCursorHidden() const { return m_screenWantsCursorHidden; }

	float GetMoveX() const;
	float GetMoveY() const;

	float GetLookX(float sensitivity) const;
	float GetLookY(float sensitivity) const;

private:
	bool m_keyDown[MAX_KEYS];
	bool m_keyDownPrev[MAX_KEYS];

	bool m_keyPressedAccum[MAX_KEYS];
	bool m_keyReleasedAccum[MAX_KEYS];
	bool m_keyPressed[MAX_KEYS];
	bool m_keyReleased[MAX_KEYS];

	bool m_mouseButtonDown[MAX_MOUSE_BUTTONS];
	bool m_mouseButtonDownPrev[MAX_MOUSE_BUTTONS];

	bool m_mouseBtnPressedAccum[MAX_MOUSE_BUTTONS];
	bool m_mouseBtnReleasedAccum[MAX_MOUSE_BUTTONS];
	bool m_mouseBtnPressed[MAX_MOUSE_BUTTONS];
	bool m_mouseBtnReleased[MAX_MOUSE_BUTTONS];

	int m_mouseX;
	int m_mouseY;

	int m_mouseDeltaX;
	int m_mouseDeltaY;
	int m_mouseDeltaAccumX;
	int m_mouseDeltaAccumY;

	int m_mouseWheelAccum;
	bool m_mouseWheelConsumed;

	bool m_mouseGrabbed;

	bool m_cursorHiddenForUI;

	bool m_windowFocused;

	bool m_hasInput;

	bool m_kbmActive;

	bool m_screenWantsCursorHidden;
};

extern KeyboardMouseInput g_KBMInput;

#endif // _WINDOWS64
