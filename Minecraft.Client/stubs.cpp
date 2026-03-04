#include "stdafx.h"

#ifdef _WINDOWS64
#include "Windows64\KeyboardMouseInput.h"

int Mouse::getX()
{
	return KMInput.GetMouseX();
}

int Mouse::getY()
{
	// Return Y in bottom-up coordinates (OpenGL convention, matching original Java LWJGL Mouse)
	RECT rect;
	GetClientRect(KMInput.GetHWnd(), &rect);
	return (rect.bottom - 1) - KMInput.GetMouseY();
}

bool Mouse::isButtonDown(int button)
{
	return KMInput.IsMouseDown(button);
}

int Keyboard::keyToVK(int key)
{
	if (key >= Keyboard::KEY_A && key <= Keyboard::KEY_Z)
		return 'A' + (key - Keyboard::KEY_A);
	if (key >= Keyboard::KEY_1 && key <= Keyboard::KEY_9)
		return '1' + (key - Keyboard::KEY_1);
	if (key == Keyboard::KEY_0)        return '0';
	if (key == Keyboard::KEY_SPACE)    return VK_SPACE;
	if (key == Keyboard::KEY_LSHIFT)   return VK_LSHIFT;
	if (key == Keyboard::KEY_RSHIFT)   return VK_RSHIFT;
	if (key == Keyboard::KEY_ESCAPE)   return VK_ESCAPE;
	if (key == Keyboard::KEY_RETURN)   return VK_RETURN;
	if (key == Keyboard::KEY_BACK)     return VK_BACK;
	if (key == Keyboard::KEY_TAB)      return VK_TAB;
	if (key == Keyboard::KEY_UP)       return VK_UP;
	if (key == Keyboard::KEY_DOWN)     return VK_DOWN;
	if (key == Keyboard::KEY_LEFT)     return VK_LEFT;
	if (key == Keyboard::KEY_RIGHT)    return VK_RIGHT;
	if (key == Keyboard::KEY_LCONTROL) return VK_LCONTROL;
	if (key == Keyboard::KEY_RCONTROL) return VK_RCONTROL;
	if (key == Keyboard::KEY_LALT)     return VK_LMENU;
	if (key == Keyboard::KEY_RALT)     return VK_RMENU;
	return 0;
}

bool Keyboard::isKeyDown(int key)
{
	// Map Keyboard constants to Windows virtual key codes
	if (key == Keyboard::KEY_LSHIFT) return KMInput.IsKeyDown(VK_LSHIFT);
	if (key == Keyboard::KEY_RSHIFT) return KMInput.IsKeyDown(VK_RSHIFT);
	if (key == Keyboard::KEY_ESCAPE) return KMInput.IsKeyDown(VK_ESCAPE);
	if (key == Keyboard::KEY_RETURN) return KMInput.IsKeyDown(VK_RETURN);
	if (key == Keyboard::KEY_BACK)   return KMInput.IsKeyDown(VK_BACK);
	if (key == Keyboard::KEY_SPACE)  return KMInput.IsKeyDown(VK_SPACE);
	if (key == Keyboard::KEY_TAB)    return KMInput.IsKeyDown(VK_TAB);
	if (key == Keyboard::KEY_UP)     return KMInput.IsKeyDown(VK_UP);
	if (key == Keyboard::KEY_DOWN)   return KMInput.IsKeyDown(VK_DOWN);
	if (key == Keyboard::KEY_LEFT)   return KMInput.IsKeyDown(VK_LEFT);
	if (key == Keyboard::KEY_RIGHT)  return KMInput.IsKeyDown(VK_RIGHT);
	if (key >= Keyboard::KEY_A && key <= Keyboard::KEY_Z)
		return KMInput.IsKeyDown('A' + (key - Keyboard::KEY_A));
	if (key >= Keyboard::KEY_1 && key <= Keyboard::KEY_9)
		return KMInput.IsKeyDown('1' + (key - Keyboard::KEY_1));
	if (key == Keyboard::KEY_0)       return KMInput.IsKeyDown('0');
	if (key == Keyboard::KEY_LCONTROL) return KMInput.IsKeyDown(VK_LCONTROL);
	if (key == Keyboard::KEY_RCONTROL) return KMInput.IsKeyDown(VK_RCONTROL);
	if (key == Keyboard::KEY_LALT)    return KMInput.IsKeyDown(VK_LMENU);
	if (key == Keyboard::KEY_RALT)    return KMInput.IsKeyDown(VK_RMENU);
	return false;
}
#endif

wstring Keyboard::getKeyName(int key)
{
	if (key >= KEY_A && key <= KEY_Z)
	{
		wchar_t ch = L'A' + (key - KEY_A);
		return wstring(1, ch);
	}
	if (key >= KEY_1 && key <= KEY_9)
	{
		wchar_t ch = L'1' + (key - KEY_1);
		return wstring(1, ch);
	}
	if (key == KEY_0) return L"0";
	if (key == KEY_SPACE) return L"SPACE";
	if (key == KEY_LSHIFT) return L"L.SHIFT";
	if (key == KEY_RSHIFT) return L"R.SHIFT";
	if (key == KEY_ESCAPE) return L"ESCAPE";
	if (key == KEY_BACK) return L"BACKSPACE";
	if (key == KEY_RETURN) return L"ENTER";
	if (key == KEY_UP) return L"UP";
	if (key == KEY_DOWN) return L"DOWN";
	if (key == KEY_LEFT) return L"LEFT";
	if (key == KEY_RIGHT) return L"RIGHT";
	if (key == KEY_TAB) return L"TAB";
	if (key == KEY_LCONTROL) return L"L.CTRL";
	if (key == KEY_RCONTROL) return L"R.CTRL";
	if (key == KEY_LALT) return L"L.ALT";
	if (key == KEY_RALT) return L"R.ALT";
	// Mouse buttons (negative keys)
	if (key < 0)
	{
		int button = key + 100;
		if (button == 0) return L"L.MOUSE";
		if (button == 1) return L"R.MOUSE";
		if (button == 2) return L"M.MOUSE";
	}
	return L"UNKNOWN";
}

int Keyboard::getKeyCount()
{
	return KEY_COUNT;
}

void glReadPixels(int,int, int, int, int, int, ByteBuffer *)
{
}

void glClearDepth(double)
{
}

void glVertexPointer(int, int, int, int)
{
}

void glVertexPointer(int, int, FloatBuffer *)
{
}

void glTexCoordPointer(int, int, int, int)
{
}

void glTexCoordPointer(int, int, FloatBuffer *)
{
}

void glNormalPointer(int, int, int)
{
}

void glNormalPointer(int, ByteBuffer *)
{
}

void glEnableClientState(int)
{
}

void glDisableClientState(int)
{
}

void glColorPointer(int, int, int, int)
{
}

void glColorPointer(int, bool, int, ByteBuffer *)
{
}

void glDrawArrays(int,int,int)
{
}

void glNormal3f(float,float,float)
{
}

void glGenQueriesARB(IntBuffer *)
{
}

void glBeginQueryARB(int,int)
{
}

void glEndQueryARB(int)
{
}

void glGetQueryObjectuARB(int,int,IntBuffer *)
{
}

void glShadeModel(int)
{
}

void glColorMaterial(int,int)
{
}

//1.8.2
void glClientActiveTexture(int)
{
}

void glActiveTexture(int)
{
}

void glFlush()
{
}

void glTexGeni(int,int,int)
{
}

#ifdef _XBOX
// 4J Stu - Added these to stop us needing to pull in loads of media libraries just to use Qnet
#include <xcam.h>
DWORD XCamInitialize(){ return 0; }
VOID XCamShutdown() {}
 
DWORD XCamCreateStreamEngine(
         CONST XCAM_STREAM_ENGINE_INIT_PARAMS *pParams,
         PIXCAMSTREAMENGINE *ppEngine
		 ) { return 0; }
 
DWORD XCamSetView(
         XCAMZOOMFACTOR ZoomFactor,
         LONG XCenter,
         LONG YCenter,
         PXOVERLAPPED pOverlapped
) { return 0; }
 
XCAMDEVICESTATE XCamGetStatus() { return XCAMDEVICESTATE_DISCONNECTED; }
#endif