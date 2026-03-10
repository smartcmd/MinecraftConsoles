#pragma once
#include <xsocialpost.h>

const int GL_MODELVIEW_MATRIX = 0;
const int GL_PROJECTION_MATRIX = 1;
const int GL_MODELVIEW = 0;
const int GL_PROJECTION = 1;
const int GL_TEXTURE = 2;

// These things required for tex gen

const int GL_S = 0;
const int GL_T = 1;
const int GL_R = 2;
const int GL_Q = 3;

const int GL_TEXTURE_GEN_S = 0;
const int GL_TEXTURE_GEN_T = 1;
const int GL_TEXTURE_GEN_Q = 2;
const int GL_TEXTURE_GEN_R = 3;

const int GL_TEXTURE_GEN_MODE = 0;
const int GL_OBJECT_LINEAR = 0;
const int GL_EYE_LINEAR = 1;
const int GL_OBJECT_PLANE = 0;
const int GL_EYE_PLANE = 1;


// These things are used by glEnable/glDisable so must be different and non-zero (zero is used by things we haven't assigned yet)
const int GL_TEXTURE_2D = 1;
const int GL_BLEND = 2;
const int GL_CULL_FACE = 3;
const int GL_ALPHA_TEST = 4;
const int GL_DEPTH_TEST = 5;
const int GL_FOG = 6;
const int GL_LIGHTING = 7;
const int GL_LIGHT0 = 8;
const int GL_LIGHT1 = 9;


const int GL_DEPTH_BUFFER_BIT = D3DCLEAR_ZBUFFER;
const int GL_COLOR_BUFFER_BIT = D3DCLEAR_TARGET;

const int GL_QUADS = D3DPT_QUADLIST;
const int GL_TRIANGLES = D3DPT_TRIANGLELIST;
const int GL_LINE_STRIP = D3DPT_LINESTRIP;
const int GL_TRIANGLE_STRIP = D3DPT_TRIANGLESTRIP;
const int GL_TRIANGLE_FAN = D3DPT_TRIANGLEFAN;
const int GL_LINES=D3DPT_LINELIST;
const int GL_SRC_ALPHA = D3DBLEND_SRCALPHA;
const int GL_ONE_MINUS_SRC_ALPHA = D3DBLEND_INVSRCALPHA;
const int GL_ONE = D3DBLEND_ONE;
const int GL_ZERO = D3DBLEND_ZERO;
const int GL_DST_ALPHA = D3DBLEND_DESTALPHA;
const int GL_SRC_COLOR = D3DBLEND_SRCCOLOR;
const int GL_DST_COLOR = D3DBLEND_DESTCOLOR;
const int GL_ONE_MINUS_DST_COLOR = D3DBLEND_INVDESTCOLOR;
const int GL_ONE_MINUS_SRC_COLOR = D3DBLEND_INVSRCCOLOR;
const int GL_CONSTANT_ALPHA = D3DBLEND_CONSTANTALPHA;
const int GL_ONE_MINUS_CONSTANT_ALPHA = D3DBLEND_INVCONSTANTALPHA;

const int GL_GREATER = D3DCMP_GREATER;
const int GL_EQUAL = D3DCMP_EQUAL;
const int GL_LEQUAL = D3DCMP_LESSEQUAL;
const int GL_GEQUAL = D3DCMP_GREATEREQUAL;
const int GL_ALWAYS = D3DCMP_ALWAYS;

const int GL_TEXTURE_MIN_FILTER = 1;
const int GL_TEXTURE_MAG_FILTER = 2;
const int GL_TEXTURE_WRAP_S = 3;
const int GL_TEXTURE_WRAP_T = 4;

const int GL_NEAREST = 0;
const int GL_LINEAR = 1;
const int GL_EXP = 2;
const int GL_NEAREST_MIPMAP_LINEAR = 0;	// TODO - mipmapping bit of this

const int GL_CLAMP = 0;
const int GL_REPEAT = 1;

const int GL_FOG_START = 1;
const int GL_FOG_END = 2;
const int GL_FOG_MODE = 3;
const int GL_FOG_DENSITY = 4;
const int GL_FOG_COLOR = 5;

const int GL_POSITION = 1;
const int GL_AMBIENT = 2;
const int GL_DIFFUSE = 3;
const int GL_SPECULAR = 4;

const int GL_LIGHT_MODEL_AMBIENT = 1;


class C4JRender
{
public:
	void Tick();
	void UpdateGamma(unsigned short usGamma);

	// Matrix stack
	void MatrixMode(int type);
	void MatrixSetIdentity();
	void MatrixTranslate(float x,float y,float z);
	void MatrixRotate(float angle, float x, float y, float z);
	void MatrixScale(float x, float y, float z);
	void MatrixPerspective(float fovy, float aspect, float zNear, float zFar);
	void MatrixOrthogonal(float left,float right,float bottom,float top,float zNear,float zFar);
	void MatrixPop();
	void MatrixPush();
	void MatrixMult(float *mat);
	const float *MatrixGet(int type);
	void Set_matrixDirty();

	// Core
	void Initialise(IDirect3DDevice9 *pDevice);
	void InitialiseContext();
	void Present();
	void Clear(int flags, D3DRECT *pRect = nullptr);
	void SetClearColour(D3DCOLOR colour);
	bool IsWidescreen();
	bool IsHiDef();
	void CaptureThumbnail(LPD3DXBUFFER *pngOut);
	void CaptureScreen(LPD3DXBUFFER *jpgOut, XSOCIAL_PREVIEWIMAGE *previewOut);
	void BeginConditionalSurvey(int identifier);
	void EndConditionalSurvey();
	void BeginConditionalRendering(int identifier);
	void EndConditionalRendering();

	// Vertex data handling
	typedef enum
	{
		VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1,		// Position 3 x float, texture 2 x float, colour 4 x byte, normal 4 x byte, padding 1 DWORD
		VERTEX_TYPE_PS3_TS2_CS1,				// Position 3 x short, texture 2 x short, colour 4 x byte, padding 1 DWORD
		VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1_LIT,	// as VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1 with lighting applied,
		VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1_TEXGEN, // as VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1 with tex gen
		VERTEX_TYPE_COUNT
	} eVertexType;

	// Pixel shader
	typedef enum
	{
		PIXEL_SHADER_TYPE_STANDARD,
		PIXEL_SHADER_TYPE_PROJECTION,
		PIXEL_SHADER_TYPE_LOD_FORCE,
		PIXEL_SHADER_COUNT
	} ePixelShaderType;

	typedef enum
	{
		VIEWPORT_TYPE_FULLSCREEN,
		VIEWPORT_TYPE_SPLIT_TOP,
		VIEWPORT_TYPE_SPLIT_BOTTOM,
		VIEWPORT_TYPE_SPLIT_LEFT,
		VIEWPORT_TYPE_SPLIT_RIGHT,
		VIEWPORT_TYPE_QUADRANT_TOP_LEFT,
		VIEWPORT_TYPE_QUADRANT_TOP_RIGHT,
		VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT,
		VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT,
	} eViewportType;

	void DrawVertices(D3DPRIMITIVETYPE PrimitiveType, int count, void *dataIn, eVertexType vType, C4JRender::ePixelShaderType psType);
	void DrawVertexBuffer(D3DPRIMITIVETYPE PrimitiveType, int count, IDirect3DVertexBuffer9 *buffer, C4JRender::eVertexType vType, C4JRender::ePixelShaderType psType);

	// Command buffers
	void CBuffLockStaticCreations();
	int  CBuffCreate(int count);
	void CBuffDelete(int first, int count);
	void CBuffStart(int index);
	void CBuffClear(int index);
	int	 CBuffSize(int index);
	void CBuffEnd(int vertexCount);
	bool CBuffCall(int index, bool full = true);
	void CBuffTick();
	void CBuffDeferredModeStart();
	void CBuffDeferredModeEnd();

	typedef enum
	{
		TEXTURE_FORMAT_RxGyBzAw,		// Normal 32-bit RGBA texture, 8 bits per component
		TEXTURE_FORMAT_R0G0B0Ax,		// One 8-bit component mapped to alpha channel, R=G=B=0
		TEXTURE_FORMAT_R1G1B1Ax,		// One 8-bit component mapped to alpha channel, R=G=B=1
		TEXTURE_FORMAT_RxGxBxAx,		// One 8-bit component mapped to all channels
		MAX_TEXTURE_FORMATS
	} eTextureFormat;

	// Textures
	int TextureCreate();
	void TextureFree(int idx);
	void TextureBind(int idx);
	void TextureBindVertex(int idx);
	void TextureSetTextureLevels(int levels);
	int  TextureGetTextureLevels();
	void TextureData(int width, int height, void *data, int level, eTextureFormat format = TEXTURE_FORMAT_RxGyBzAw);
	void TextureDataUpdate(void *data, int level);
	void TextureSetParam(int param, int value);
	HRESULT LoadTextureData(const char *szFilename,D3DXIMAGE_INFO *pSrcInfo, int **ppDataOut);
	HRESULT LoadTextureData(BYTE *pbData, DWORD dwBytes,D3DXIMAGE_INFO *pSrcInfo, int **ppDataOut);
	HRESULT SaveTextureData(const char *szFilename, D3DXIMAGE_INFO *pSrcInfo, int *ppDataOut);
	void TextureGetStats();

	// State control
	void StateSetColour(float r, float g, float b, float a);
	void StateSetDepthMask(bool enable);
	void StateSetBlendEnable(bool enable);
	void StateSetBlendFunc(int src, int dst);
	void StateSetBlendFactor(unsigned int colour);
	void StateSetAlphaFunc(int func, float param);
	void StateSetDepthFunc(int func);
	void StateSetFaceCull(bool enable);
	void StateSetFaceCullCW(bool cw);
	void StateSetLineWidth(float width);
	void StateSetWriteEnable(bool red, bool green, bool blue, bool alpha);
	void StateSetDepthTestEnable(bool enable);
	void StateSetAlphaTestEnable(bool enable);
	void StateSetDepthSlopeAndBias(float slope, float bias);
	void StateSetFogEnable(bool enable);
	void StateSetFogMode(int mode);
	void StateSetFogNearDistance(float dist);
	void StateSetFogFarDistance(float dist);
	void StateSetFogDensity(float density);
	void StateSetFogColour(float red, float green, float blue);
	void StateSetLightingEnable(bool enable);
	void StateSetVertexTextureUV( float u, float v);
	void StateSetLightColour(int light, float red, float green, float blue);
	void StateSetLightAmbientColour(float red, float green, float blue);
	void StateSetLightDirection(int light, float x, float y, float z);
	void StateSetLightEnable(int light, bool enable);
	void StateSetViewport(eViewportType viewportType);
	void StateSetEnableViewportClipPlanes(bool enable);
	void StateSetTexGenCol(int col, float x, float y, float z, float w, bool eyeSpace);
	void StateSetForceLOD(int LOD);
};


// Singleton
extern C4JRender RenderManager;

