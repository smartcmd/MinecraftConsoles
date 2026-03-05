#include "stdafx.h"

#ifdef _WINDOWS64

#include "KeyboardMouseInput.h"
#include <cmath>

KeyboardMouseInput g_KBMInput;

extern HWND g_hWnd;

// Forward declaration
static void ClipCursorToWindow(HWND hWnd);

static bool IsModifierKeyDown(const bool* keyState, int vkCode)
{
	switch (vkCode)
	{
	case VK_SHIFT:
		return keyState[VK_LSHIFT] || keyState[VK_RSHIFT];
	case VK_CONTROL:
		return keyState[VK_LCONTROL] || keyState[VK_RCONTROL];
	case VK_MENU:
		return keyState[VK_LMENU] || keyState[VK_RMENU];
	default:
		return false;
	}
}

void KeyboardMouseInput::Init()
{
	memset(m_keyDown, 0, sizeof(m_keyDown));
	memset(m_keyDownPrev, 0, sizeof(m_keyDownPrev));
	memset(m_keyPressedAccum, 0, sizeof(m_keyPressedAccum));
	memset(m_keyReleasedAccum, 0, sizeof(m_keyReleasedAccum));
	memset(m_keyPressed, 0, sizeof(m_keyPressed));
	memset(m_keyReleased, 0, sizeof(m_keyReleased));
	memset(m_mouseButtonDown, 0, sizeof(m_mouseButtonDown));
	memset(m_mouseButtonDownPrev, 0, sizeof(m_mouseButtonDownPrev));
	memset(m_mouseBtnPressedAccum, 0, sizeof(m_mouseBtnPressedAccum));
	memset(m_mouseBtnReleasedAccum, 0, sizeof(m_mouseBtnReleasedAccum));
	memset(m_mouseBtnPressed, 0, sizeof(m_mouseBtnPressed));
	memset(m_mouseBtnReleased, 0, sizeof(m_mouseBtnReleased));
	m_mouseX = 0;
	m_mouseY = 0;
	m_mouseDeltaX = 0;
	m_mouseDeltaY = 0;
	m_mouseDeltaAccumX = 0;
	m_mouseDeltaAccumY = 0;
	m_mouseWheelAccum = 0;
	m_mouseWheelConsumed = false;
	m_mouseGrabbed = false;
	m_cursorHiddenForUI = false;
	m_windowFocused = true;
	m_hasInput = false;
	m_kbmActive = true;
	m_screenWantsCursorHidden = false;
	m_charBufferHead = 0;
	m_charBufferTail = 0;

	RAWINPUTDEVICE rid;
	rid.usUsagePage = 0x01; // HID_USAGE_PAGE_GENERIC
	rid.usUsage = 0x02;     // HID_USAGE_GENERIC_MOUSE
	rid.dwFlags = 0;
	rid.hwndTarget = g_hWnd;
	RegisterRawInputDevices(&rid, 1, sizeof(rid));
}

void KeyboardMouseInput::ClearAllState()
{
	memset(m_keyDown, 0, sizeof(m_keyDown));
	memset(m_keyDownPrev, 0, sizeof(m_keyDownPrev));
	memset(m_keyPressedAccum, 0, sizeof(m_keyPressedAccum));
	memset(m_keyReleasedAccum, 0, sizeof(m_keyReleasedAccum));
	memset(m_keyPressed, 0, sizeof(m_keyPressed));
	memset(m_keyReleased, 0, sizeof(m_keyReleased));
	memset(m_mouseButtonDown, 0, sizeof(m_mouseButtonDown));
	memset(m_mouseButtonDownPrev, 0, sizeof(m_mouseButtonDownPrev));
	memset(m_mouseBtnPressedAccum, 0, sizeof(m_mouseBtnPressedAccum));
	memset(m_mouseBtnReleasedAccum, 0, sizeof(m_mouseBtnReleasedAccum));
	memset(m_mouseBtnPressed, 0, sizeof(m_mouseBtnPressed));
	memset(m_mouseBtnReleased, 0, sizeof(m_mouseBtnReleased));
	m_mouseDeltaX = 0;
	m_mouseDeltaY = 0;
	m_mouseDeltaAccumX = 0;
	m_mouseDeltaAccumY = 0;
	m_mouseWheelAccum = 0;
	m_mouseWheelConsumed = false;
}

void KeyboardMouseInput::Tick()
{
	memcpy(m_keyDownPrev, m_keyDown, sizeof(m_keyDown));
	memcpy(m_mouseButtonDownPrev, m_mouseButtonDown, sizeof(m_mouseButtonDown));

	memcpy(m_keyPressed, m_keyPressedAccum, sizeof(m_keyPressedAccum));
	memcpy(m_keyReleased, m_keyReleasedAccum, sizeof(m_keyReleasedAccum));
	memset(m_keyPressedAccum, 0, sizeof(m_keyPressedAccum));
	memset(m_keyReleasedAccum, 0, sizeof(m_keyReleasedAccum));

	memcpy(m_mouseBtnPressed, m_mouseBtnPressedAccum, sizeof(m_mouseBtnPressedAccum));
	memcpy(m_mouseBtnReleased, m_mouseBtnReleasedAccum, sizeof(m_mouseBtnReleasedAccum));
	memset(m_mouseBtnPressedAccum, 0, sizeof(m_mouseBtnPressedAccum));
	memset(m_mouseBtnReleasedAccum, 0, sizeof(m_mouseBtnReleasedAccum));

	m_mouseDeltaX = m_mouseDeltaAccumX;
	m_mouseDeltaY = m_mouseDeltaAccumY;
	m_mouseDeltaAccumX = 0;
	m_mouseDeltaAccumY = 0;
	m_mouseWheelConsumed = false;

	m_hasInput = (m_mouseDeltaX != 0 || m_mouseDeltaY != 0 || m_mouseWheelAccum != 0);
	if (!m_hasInput)
	{
		for (int i = 0; i < MAX_KEYS; i++)
		{
			if (m_keyDown[i]) { m_hasInput = true; break; }
		}
	}
	if (!m_hasInput)
	{
		for (int i = 0; i < MAX_MOUSE_BUTTONS; i++)
		{
			if (m_mouseButtonDown[i]) { m_hasInput = true; break; }
		}
	}

	if ((m_mouseGrabbed || m_cursorHiddenForUI) && g_hWnd)
	{
		RECT rc;
		GetClientRect(g_hWnd, &rc);
		POINT center;
		center.x = (rc.right - rc.left) / 2;
		center.y = (rc.bottom - rc.top) / 2;
		ClientToScreen(g_hWnd, &center);
		SetCursorPos(center.x, center.y);
	}
}

void KeyboardMouseInput::OnKeyDown(int vkCode)
{
	if (vkCode >= 0 && vkCode < MAX_KEYS)
	{
		if (!m_keyDown[vkCode])
			m_keyPressedAccum[vkCode] = true;
		m_keyDown[vkCode] = true;
	}
}

void KeyboardMouseInput::OnKeyUp(int vkCode)
{
	if (vkCode >= 0 && vkCode < MAX_KEYS)
	{
		if (m_keyDown[vkCode])
			m_keyReleasedAccum[vkCode] = true;
		m_keyDown[vkCode] = false;
	}
}

void KeyboardMouseInput::OnMouseButtonDown(int button)
{
	if (button >= 0 && button < MAX_MOUSE_BUTTONS)
	{
		if (!m_mouseButtonDown[button])
			m_mouseBtnPressedAccum[button] = true;
		m_mouseButtonDown[button] = true;
	}
}

void KeyboardMouseInput::OnMouseButtonUp(int button)
{
	if (button >= 0 && button < MAX_MOUSE_BUTTONS)
	{
		if (m_mouseButtonDown[button])
			m_mouseBtnReleasedAccum[button] = true;
		m_mouseButtonDown[button] = false;
	}
}

void KeyboardMouseInput::OnMouseMove(int x, int y)
{
	m_mouseX = x;
	m_mouseY = y;
}

void KeyboardMouseInput::OnMouseWheel(int delta)
{
	// Normalize from raw Windows delta (multiples of WHEEL_DELTA=120) to discrete notch counts
	m_mouseWheelAccum += delta / WHEEL_DELTA;
}

int KeyboardMouseInput::GetMouseWheel()
{
	int val = m_mouseWheelAccum;
	if (val != 0)
		m_mouseWheelConsumed = true;
	m_mouseWheelAccum = 0;
	return val;
}

void KeyboardMouseInput::OnRawMouseDelta(int dx, int dy)
{
	m_mouseDeltaAccumX += dx;
	m_mouseDeltaAccumY += dy;
}

bool KeyboardMouseInput::IsKeyDown(int vkCode) const
{
	if (vkCode == VK_SHIFT || vkCode == VK_CONTROL || vkCode == VK_MENU)
		return IsModifierKeyDown(m_keyDown, vkCode);

	if (vkCode >= 0 && vkCode < MAX_KEYS)
		return m_keyDown[vkCode];
	return false;
}

bool KeyboardMouseInput::IsKeyPressed(int vkCode) const
{
	if (vkCode == VK_SHIFT || vkCode == VK_CONTROL || vkCode == VK_MENU)
		return IsModifierKeyDown(m_keyPressed, vkCode);

	if (vkCode >= 0 && vkCode < MAX_KEYS)
		return m_keyPressed[vkCode];
	return false;
}

bool KeyboardMouseInput::IsKeyReleased(int vkCode) const
{
	if (vkCode == VK_SHIFT || vkCode == VK_CONTROL || vkCode == VK_MENU)
		return IsModifierKeyDown(m_keyReleased, vkCode);

	if (vkCode >= 0 && vkCode < MAX_KEYS)
		return m_keyReleased[vkCode];
	return false;
}

bool KeyboardMouseInput::IsMouseButtonDown(int button) const
{
	if (button >= 0 && button < MAX_MOUSE_BUTTONS)
		return m_mouseButtonDown[button];
	return false;
}

bool KeyboardMouseInput::IsMouseButtonPressed(int button) const
{
	if (button >= 0 && button < MAX_MOUSE_BUTTONS)
		return m_mouseBtnPressed[button];
	return false;
}

bool KeyboardMouseInput::IsMouseButtonReleased(int button) const
{
	if (button >= 0 && button < MAX_MOUSE_BUTTONS)
		return m_mouseBtnReleased[button];
	return false;
}

void KeyboardMouseInput::ConsumeMouseDelta(float &dx, float &dy)
{
	dx = (float)m_mouseDeltaAccumX;
	dy = (float)m_mouseDeltaAccumY;
	m_mouseDeltaAccumX = 0;
	m_mouseDeltaAccumY = 0;
}

void KeyboardMouseInput::SetMouseGrabbed(bool grabbed)
{
	if (m_mouseGrabbed == grabbed)
		return;

	m_mouseGrabbed = grabbed;
	if (grabbed && g_hWnd)
	{
		while (ShowCursor(FALSE) >= 0) {}
		ClipCursorToWindow(g_hWnd);

		RECT rc;
		GetClientRect(g_hWnd, &rc);
		POINT center;
		center.x = (rc.right - rc.left) / 2;
		center.y = (rc.bottom - rc.top) / 2;
		ClientToScreen(g_hWnd, &center);
		SetCursorPos(center.x, center.y);

		m_mouseDeltaAccumX = 0;
		m_mouseDeltaAccumY = 0;
	}
	else if (!grabbed && !m_cursorHiddenForUI && g_hWnd)
	{
		while (ShowCursor(TRUE) < 0) {}
		ClipCursor(NULL);
	}
}

void KeyboardMouseInput::SetCursorHiddenForUI(bool hidden)
{
	if (m_cursorHiddenForUI == hidden)
		return;

	m_cursorHiddenForUI = hidden;
	if (hidden && g_hWnd)
	{
		while (ShowCursor(FALSE) >= 0) {}
		ClipCursorToWindow(g_hWnd);

		RECT rc;
		GetClientRect(g_hWnd, &rc);
		POINT center;
		center.x = (rc.right - rc.left) / 2;
		center.y = (rc.bottom - rc.top) / 2;
		ClientToScreen(g_hWnd, &center);
		SetCursorPos(center.x, center.y);

		m_mouseDeltaAccumX = 0;
		m_mouseDeltaAccumY = 0;
	}
	else if (!hidden && !m_mouseGrabbed && g_hWnd)
	{
		while (ShowCursor(TRUE) < 0) {}
		ClipCursor(NULL);
	}
}

static void ClipCursorToWindow(HWND hWnd)
{
	if (!hWnd) return;
	RECT rc;
	GetClientRect(hWnd, &rc);
	POINT topLeft = { rc.left, rc.top };
	POINT bottomRight = { rc.right, rc.bottom };
	ClientToScreen(hWnd, &topLeft);
	ClientToScreen(hWnd, &bottomRight);
	RECT clipRect = { topLeft.x, topLeft.y, bottomRight.x, bottomRight.y };
	ClipCursor(&clipRect);
}

void KeyboardMouseInput::SetWindowFocused(bool focused)
{
	m_windowFocused = focused;
	if (focused)
	{
		if (m_mouseGrabbed || m_cursorHiddenForUI)
		{
			while (ShowCursor(FALSE) >= 0) {}
			ClipCursorToWindow(g_hWnd);
		}
		else
		{
			while (ShowCursor(TRUE) < 0) {}
			ClipCursor(NULL);
		}
	}
	else
	{
		while (ShowCursor(TRUE) < 0) {}
		ClipCursor(NULL);
	}
}

float KeyboardMouseInput::GetMoveX() const
{
	float x = 0.0f;
	if (m_keyDown[KEY_LEFT])  x += 1.0f;
	if (m_keyDown[KEY_RIGHT]) x -= 1.0f;
	return x;
}

float KeyboardMouseInput::GetMoveY() const
{
	float y = 0.0f;
	if (m_keyDown[KEY_FORWARD])  y += 1.0f;
	if (m_keyDown[KEY_BACKWARD]) y -= 1.0f;
	return y;
}

float KeyboardMouseInput::GetLookX(float sensitivity) const
{
	return (float)m_mouseDeltaX * sensitivity;
}

float KeyboardMouseInput::GetLookY(float sensitivity) const
{
	return (float)(-m_mouseDeltaY) * sensitivity;
}

void KeyboardMouseInput::OnChar(wchar_t c)
{
	int next = (m_charBufferHead + 1) % CHAR_BUFFER_SIZE;
	if (next != m_charBufferTail)
	{
		m_charBuffer[m_charBufferHead] = c;
		m_charBufferHead = next;
	}
}

bool KeyboardMouseInput::ConsumeChar(wchar_t &outChar)
{
	if (m_charBufferTail == m_charBufferHead)
		return false;
	outChar = m_charBuffer[m_charBufferTail];
	m_charBufferTail = (m_charBufferTail + 1) % CHAR_BUFFER_SIZE;
	return true;
}

void KeyboardMouseInput::ClearCharBuffer()
{
	m_charBufferHead = 0;
	m_charBufferTail = 0;
}

#endif // _WINDOWS64
