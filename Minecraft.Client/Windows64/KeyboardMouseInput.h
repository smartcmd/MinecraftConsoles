#pragma once

#ifdef _WINDOWS64

#include <windows.h>

// HID usage page and usage for raw input registration
#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC ((USHORT)0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE ((USHORT)0x02)
#endif

class KeyboardMouseInput
{
public:
    //game controls
    const static int KEY_WALK_UP = 'W';
    const static int KEY_WALK_DOWN = 'S';
    const static int KEY_WALK_LEFT = 'A';
    const static int KEY_WALK_RIGHT = 'D';
    const static int KEY_SNEAK = VK_SHIFT;
    const static int KEY_INVEN = 'E';
    const static int KEY_DROP = 'Q';
    const static int KEY_CRAFT = 'C';
    const static int KEY_JUMP = VK_SPACE;
    const static int KEY_SPRINT = VK_CONTROL;
    const static int KEY_PAUSE = VK_ESCAPE;

    //menu controls
    const static int KEY_UP = VK_UP;
    const static int KEY_DOWN = VK_DOWN;
    const static int KEY_LEFT = VK_LEFT;
    const static int KEY_RIGHT = VK_RIGHT;
    const static int KEY_A = VK_RETURN;
    const static int KEY_B = VK_ESCAPE;
    const static int KEY_LEFT_SCROLL = 'Q';
    const static int KEY_RIGHT_SCROLL = 'E';

    //debug type shit
    const static int KEY_TOGGLE_VIEW = VK_F5;
    const static int KEY_DEBUG_OVERLAY = VK_F4;
    const static int KEY_DEBUG_CONSOLE_OVERLAY = VK_F3;
    const static int KEY_FULLSCREEN = VK_F11;
    const static int KEY_TOGGLE_CAPTURE = VK_MENU;
    const static int KEY_GAME_INFO_MENU = VK_TAB;

	KeyboardMouseInput();
	~KeyboardMouseInput();

	void Init(HWND hWnd);
	void Tick();
	void EndFrame();

	// Called from WndProc
	void OnKeyDown(WPARAM vk);
	void OnKeyUp(WPARAM vk);
	void OnRawMouseInput(LPARAM lParam);
	void OnMouseButton(int button, bool down);
	void OnMouseWheel(int delta);
	void ClearAllState();

	// Per-frame edge detection (for UI / per-frame logic like Alt toggle)
	bool IsKeyDown(int vk) const;
	bool IsKeyPressed(int vk) const;
	bool IsKeyReleased(int vk) const;
	bool IsMouseDown(int btn) const;
	bool IsMousePressed(int btn) const;
	bool IsMouseReleased(int btn) const;

	// Game-tick consume methods: accumulate across frames, clear on read.
	// Use these from code that runs at game tick rate (20Hz).
	bool ConsumeKeyPress(int vk);
	bool ConsumeMousePress(int btn);
	bool ConsumeMouseRelease(int btn);
	void ConsumeMouseDelta(float &dx, float &dy);
	int ConsumeScrollDelta();

	// Absolute cursor position (client-area coordinates, for GUI when not captured)
	void OnMouseMove(int x, int y);
	int GetMouseX() const;
	int GetMouseY() const;
	HWND GetHWnd() const;

	// Mouse capture for FPS look
	void SetCapture(bool capture);
	bool IsCaptured() const;

private:
	void CenterCursor();

	// Per-frame double-buffered state (for IsKeyPressed/Released per-frame edge detection)
	bool m_keyState[256];
	bool m_keyStatePrev[256];
	bool m_mouseButtons[3];
	bool m_mouseButtonsPrev[3];

	// Sticky press accumulators (persist until consumed by game tick)
	bool m_keyPressedAccum[256];
	bool m_mousePressedAccum[3];
	bool m_mouseReleasedAccum[3];

	// Mouse delta accumulators (persist until consumed by game tick)
	float m_mouseDeltaXAccum;
	float m_mouseDeltaYAccum;

	// Scroll accumulator (persists until consumed by game tick)
	int m_scrollDeltaAccum;

	bool m_captured;
	HWND m_hWnd;
	bool m_initialized;

	// Absolute cursor position in client coordinates
	int m_mouseX;
	int m_mouseY;
};

extern KeyboardMouseInput KMInput;

#endif // _WINDOWS64
