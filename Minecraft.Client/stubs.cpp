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
	return false;
}
#endif

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