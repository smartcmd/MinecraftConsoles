#include "stdafx.h"
#include "..\Minecraft.World\FloatBuffer.h"
#include "..\Minecraft.World\IntBuffer.h"
#include "..\Minecraft.World\ByteBuffer.h"

void glViewport(int x, int y, int w, int h)
{
	// We don't really need anything here because minecraft doesn't current do anything other than the default viewport
}

void glTranslatef(float x,float y,float z)
{
	RenderManager.MatrixTranslate(x,y,z);
}

void glRotatef(float angle, float x, float y, float z)
{
	RenderManager.MatrixRotate(angle*(PI/180.0f),x,y,z);
}

void glPopMatrix()
{
	RenderManager.MatrixPop();
}

void glPushMatrix()
{
	RenderManager.MatrixPush();
}

void glScalef(float x, float y, float z)
{
	RenderManager.MatrixScale(x,y,z);
}

void glMultMatrixf(float *m)
{
	RenderManager.MatrixMult(m);
}

void glMatrixMode(int type)
{
	RenderManager.MatrixMode(type);
}

void glLoadIdentity()
{
	RenderManager.MatrixSetIdentity();
}

// AAR - Use the aspect ratio passed by the caller.  For single-player this
// equals g_iAspectRatio (screen width / height), but for split-screen
// getFovAndAspect adjusts it to match the viewport dimensions.
void gluPerspective(float fovy, float aspect, float zNear, float zFar)
{
	RenderManager.MatrixPerspective(fovy, aspect, zNear, zFar);
}

void glOrtho(float left,float right,float bottom,float top,float zNear,float zFar)
{
	RenderManager.MatrixOrthogonal(left,right,bottom,top,zNear,zFar);
}

void glScaled(double x,double y,double z)
{
	RenderManager.MatrixScale(static_cast<float>(x),static_cast<float>(y),static_cast<float>(z));
}

void glGetFloat(int type, FloatBuffer *buff)
{
	memcpy(buff->_getDataPointer(),RenderManager.MatrixGet(type),64);
}

void glDeleteLists(int first,int count)
{
	RenderManager.CBuffDelete(first,count);
}

int glGenLists(int count)
{
	return RenderManager.CBuffCreate(count);
}

void glNewList(int index, int mode)
{
	RenderManager.CBuffStart(index);
}

void glEndList(int vertexCount)
{
#ifdef _XBOX
	RenderManager.CBuffEnd(vertexCount);
#else
	RenderManager.CBuffEnd();
#endif
}

void glCallList(int index)
{
	RenderManager.CBuffCall(index);
}

void glCallLists(IntBuffer *ib)
{
	for(unsigned int i = 0; i < ib->limit(); i++)
	{
		RenderManager.CBuffCall(ib->get(i));
	}
}

void glClear(int flags)
{
	RenderManager.Clear(flags);
}

void glClearColor(float r, float g, float b, float a)
{
#ifdef _XBOX
	int ir = (int)(r * 255.0f);	if( ir < 0 ) ir = 0;	if( ir > 255 ) ir = 255;
	int ig = (int)(g * 255.0f);	if( ig < 0 ) ig = 0;	if( ig > 255 ) ig = 255;
	int ib = (int)(b * 255.0f);	if( ib < 0 ) ib = 0;	if( ib > 255 ) ib = 255;
	int ia = (int)(a * 255.0f);	if( ia < 0 ) ia = 0;	if( ia > 255 ) ia = 255;

	RenderManager.SetClearColour(D3DCOLOR_RGBA(ir,ig,ib,ia));
#else
	float rgba[4] = {r,g,b,a};
	RenderManager.SetClearColour(rgba);
#endif
}

void Display::update()
{
}

void Display::swapBuffers()
{
}

void glBindTexture(int target,int texture)
{
	RenderManager.TextureBind(texture);
}

void glTexImage2D(int target,int level,int internalformat,int width,int height,int border,int format,int type, ByteBuffer *data)
{
	RenderManager.TextureData(width,height,data->getBuffer(),level);
}

void glDeleteTextures(IntBuffer *ib)
{
	for(unsigned int i = 0; i < ib->limit(); i++)
	{
		RenderManager.TextureFree(ib->get(i));
	}
}

// 4J Stu - I'm pretty sure this is what it should do
void glDeleteTextures(int id)
{
	RenderManager.TextureFree(id);
}

void glGenTextures(IntBuffer *ib)
{
	for(unsigned int i = 0; i < ib->limit(); i++)
	{
		ib->put(RenderManager.TextureCreate());
	}
}

// 4J Stu - I'm pretty sure this is what it should do
int glGenTextures()
{
	return RenderManager.TextureCreate();
}

void glColor3f(float r, float g, float b)
{
	RenderManager.StateSetColour(r,g,b,1.0f);
}

void glColor4f(float r, float g, float b, float a)
{
	RenderManager.StateSetColour(r,g,b,a);
}

void glDisable(int state)
{
	switch(state)
	{
		case GL_TEXTURE_2D:
			RenderManager.TextureBind(-1);
			break;
		case GL_BLEND:
			RenderManager.StateSetBlendEnable(false);
			break;
		case GL_CULL_FACE:
			RenderManager.StateSetFaceCull(false);
			break;
		case GL_DEPTH_TEST:
			RenderManager.StateSetDepthTestEnable(false);
			break;
		case GL_ALPHA_TEST:
			RenderManager.StateSetAlphaTestEnable(false);
			break;
		case GL_FOG:
			RenderManager.StateSetFogEnable(false);
			break;
		case GL_LIGHTING:
			RenderManager.StateSetLightingEnable(false);
			break;
		case GL_LIGHT0:
			RenderManager.StateSetLightEnable(0,false);
			break;
		case GL_LIGHT1:
			RenderManager.StateSetLightEnable(1,false);
			break;
	}
}

void glEnable(int state)
{
	switch(state)
	{
		case GL_BLEND:
			RenderManager.StateSetBlendEnable(true);
			break;
		case GL_CULL_FACE:
			RenderManager.StateSetFaceCull(true);
			break;
		case GL_DEPTH_TEST:
			RenderManager.StateSetDepthTestEnable(true);
			break;
		case GL_ALPHA_TEST:
			RenderManager.StateSetAlphaTestEnable(true);
			break;
		case GL_FOG:
			RenderManager.StateSetFogEnable(true);
			break;
		case GL_LIGHTING:
			RenderManager.StateSetLightingEnable(true);
			break;
		case GL_LIGHT0:
			RenderManager.StateSetLightEnable(0,true);
			break;
		case GL_LIGHT1:
			RenderManager.StateSetLightEnable(1,true);
			break;
	}
}

void glDepthMask(bool enable)
{
	RenderManager.StateSetDepthMask(enable);
}

void glBlendFunc(int src, int dst)
{
	RenderManager.StateSetBlendFunc(src,dst);
}

void glAlphaFunc(int func,float param)
{
	RenderManager.StateSetAlphaFunc(func, param);
}

void glDepthFunc(int func)
{
#ifdef _XBOX
	RenderManager.StateSetDepthFunc(func);
#else
	RenderManager.StateSetDepthFunc(func);
#endif
}

void glTexParameteri(int target, int param, int value)
{
	RenderManager.TextureSetParam(param,value);
}

void glPolygonOffset(float factor, float units)
{
#ifdef __PS3__
	RenderManager.StateSetDepthSlopeAndBias(factor, units);
#else
	// DirectX specifies these offsets in z buffer 0 to 1 sort of range, whereas opengl seems to be in a 0 -> depth buffer size sort of range.
	// The slope factor is quite possibly different too. Magic factor for now anyway.
	const float magicFactor = 65536.0f;
	RenderManager.StateSetDepthSlopeAndBias(factor / magicFactor, units / magicFactor);
#endif
}

void glFogi(int param, int value)
{
	if( param == GL_FOG_MODE )
	{
		RenderManager.StateSetFogMode(value);
	}
}

void glFogf(int param, float value)
{
	switch(param)
	{
		case GL_FOG_START:
			RenderManager.StateSetFogNearDistance(value);
			break;
		case GL_FOG_END:
			RenderManager.StateSetFogFarDistance(value);
			break;
		case GL_FOG_DENSITY:
			RenderManager.StateSetFogDensity(value);
			break;
	}
}

void glFog(int param,FloatBuffer *values)
{
	if( param == GL_FOG_COLOR )
	{
		float *data = values->_getDataPointer();
		RenderManager.StateSetFogColour(data[0],data[1],data[2]);
	}
}

void glLight(int light, int mode,FloatBuffer *values)
{
	int idx;
	if( light == GL_LIGHT0 )
	{
		idx = 0;
	}
	else if( light == GL_LIGHT1 )
	{
		idx = 1;
	}
	else return;
	float *data =values->_getDataPointer();
	switch( mode )
	{
	case GL_POSITION:
		RenderManager.StateSetLightDirection(idx, data[0], data[1], data[2]);
		break;
	case GL_DIFFUSE:
		RenderManager.StateSetLightColour(idx, data[0], data[1], data[2]);
		break;
	case GL_AMBIENT:
		break;
	case GL_SPECULAR:
		break;
	}
}

void glLightModel(int mode, FloatBuffer *values)
{
	float *data =values->_getDataPointer();
	if( mode == GL_LIGHT_MODEL_AMBIENT )
	{
		RenderManager.StateSetLightAmbientColour(data[0],data[1],data[2]);
	}
}

void glLineWidth(float width)
{
	RenderManager.StateSetLineWidth(width);
}

void glColorMask(bool red, bool green, bool blue, bool alpha)
{
	RenderManager.StateSetWriteEnable(red, green, blue, alpha);
}

void glMultiTexCoord2f(int, float u , float v)
{
	// Clamp these values just to be safe - the lighting code can get broken if we pass things to StateSetVertexTextureUV that are >= 1
	if( u > 255.0f ) u = 255.0f;
	if( v > 255.0f ) v = 255.0f;

	RenderManager.StateSetVertexTextureUV( u / 256.0f, v / 256.0f);
}

void glTexGen(int coord, int mode, FloatBuffer *vec)
{
	float *data = vec->_getDataPointer();

	RenderManager.StateSetTexGenCol( coord, data[0], data[1], data[2], data[3], mode == GL_EYE_PLANE );
}

void glCullFace(int dir)
{
	RenderManager.StateSetFaceCullCW( dir == GL_BACK);
}