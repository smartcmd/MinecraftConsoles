#pragma once



const int GL_BYTE = 0;
const int GL_FLOAT = 0;
const int GL_UNSIGNED_BYTE = 0;

const int GL_COLOR_ARRAY = 0;
const int GL_VERTEX_ARRAY = 0;
const int GL_NORMAL_ARRAY = 0;
const int GL_TEXTURE_COORD_ARRAY = 0;

const int GL_COMPILE = 0;

const int GL_NORMALIZE = 0;

const int GL_RESCALE_NORMAL = 0;



const int GL_SMOOTH = 0;
const int GL_FLAT = 0;



const int GL_RGBA = 0;
const int GL_BGRA = 1;
const int GL_BGR = 0;

const int GL_SAMPLES_PASSED_ARB = 0;
const int GL_QUERY_RESULT_AVAILABLE_ARB = 0;
const int GL_QUERY_RESULT_ARB = 0;

const int GL_POLYGON_OFFSET_FILL = 0;

const int GL_FRONT = 0;
const int GL_BACK = 1;
const int GL_FRONT_AND_BACK = 2;

const int GL_COLOR_MATERIAL = 0;

const int GL_AMBIENT_AND_DIFFUSE = 0;

const int GL_TEXTURE1 = 0;
const int GL_TEXTURE0 = 1;

void glFlush();
void glTexGeni(int,int,int);
void glTexGen(int,int,FloatBuffer *);
void glReadPixels(int,int, int, int, int, int, ByteBuffer *);
void glClearDepth(double);
void glCullFace(int);
void glDeleteLists(int,int);
void glGenTextures(IntBuffer *);
int glGenTextures();
int glGenLists(int);
void glLight(int, int,FloatBuffer *);
void glLightModel(int, FloatBuffer *);
void glGetFloat(int a, FloatBuffer *b);
void glTexCoordPointer(int, int, int, int);
void glTexCoordPointer(int, int, FloatBuffer *);
void glNormalPointer(int, int, int);
void glNormalPointer(int, ByteBuffer *);
void glEnableClientState(int);
void glDisableClientState(int);
void glColorPointer(int, bool, int, ByteBuffer *);
void glColorPointer(int, int, int, int);
void glVertexPointer(int, int, int, int);
void glVertexPointer(int, int, FloatBuffer *);
void glDrawArrays(int,int,int);
void glTranslatef(float,float,float);
void glRotatef(float,float,float,float);
void glNewList(int,int);
void glEndList(int vertexCount = 0);
void glCallList(int);
void glPopMatrix();
void glPushMatrix();
void glColor3f(float,float,float);
void glScalef(float,float,float);
void glMultMatrixf(float *);
void glColor4f(float,float,float,float);
void glDisable(int);
void glEnable(int);
void glBlendFunc(int,int);
void glDepthMask(bool);
void glNormal3f(float,float,float);
void glDepthFunc(int);
void glMatrixMode(int);
void glLoadIdentity();
void glBindTexture(int,int);
void glTexParameteri(int,int,int);
void glTexImage2D(int,int,int,int,int,int,int,int,ByteBuffer *);
void glDeleteTextures(IntBuffer *);
void glDeleteTextures(int);
void glCallLists(IntBuffer *);
void glGenQueriesARB(IntBuffer *);
void glColorMask(bool,bool,bool,bool);
void glBeginQueryARB(int,int);
void glEndQueryARB(int);
void glGetQueryObjectuARB(int,int,IntBuffer *);
void glShadeModel(int);
void glPolygonOffset(float,float);
void glLineWidth(float);
void glScaled(double,double,double);
void gluPerspective(float,float,float,float);
void glClear(int);
void glViewport(int,int,int,int);
void glAlphaFunc(int,float);
void glOrtho(float,float,float,float,float,float);
void glClearColor(float,float,float,float);
void glFogi(int,int);
void glFogf(int,float);
void glFog(int,FloatBuffer *);
void glColorMaterial(int,int);
void glMultiTexCoord2f(int, float, float);

//1.8.2
void glClientActiveTexture(int);
void glActiveTexture(int);

class GL11
{
public:
	static const int GL_SMOOTH = 0;
	static const int GL_FLAT = 0;
	static void glShadeModel(int) {};
};

class ARBVertexBufferObject
{
public:
	static const int GL_ARRAY_BUFFER_ARB = 0;
	static const int GL_STREAM_DRAW_ARB = 0;
	static void glBindBufferARB(int, int) {}
	static void glBufferDataARB(int, ByteBuffer *, int) {}
	static void glGenBuffersARB(IntBuffer *) {}
};


class Level;
class Player;
class Textures;
class Font;
class MapItemSavedData;
class Mob;
class Particles
{
public:
	void render(float) {}
	void tick() {}
};

class BufferedImage;

class Graphics
{
public:
	void drawImage(BufferedImage *, int, int, void *) {}
	void dispose() {}
};

class ZipEntry
{
};
class InputStream;

class File;
class ZipFile
{
public:
	ZipFile(File *file) {}
	InputStream *getInputStream(ZipEntry *entry) { return NULL; }
	ZipEntry *getEntry(const wstring& name) {return NULL;}
	void close() {}
};

class ImageIO
{
public:
	static BufferedImage *read(InputStream *in) { return NULL; }
};

class Keyboard
{
public:
	static void create() {}
	static void destroy() {}
#ifdef _WINDOWS64
	static bool isKeyDown(int key);
#else
	static bool isKeyDown(int) {return false;}
#endif
	static wstring getKeyName(int) { return L"KEYNAME"; }
	static void enableRepeatEvents(bool) {}
	static const int KEY_A = 0;
	static const int KEY_B = 1;
	static const int KEY_C = 2;
	static const int KEY_D = 3;
	static const int KEY_E = 4;
	static const int KEY_F = 5;
	static const int KEY_G = 6;
	static const int KEY_H = 7;
	static const int KEY_I = 8;
	static const int KEY_J = 9;
	static const int KEY_K = 10;
	static const int KEY_L = 11;
	static const int KEY_M = 12;
	static const int KEY_N = 13;
	static const int KEY_O = 14;
	static const int KEY_P = 15;
	static const int KEY_Q = 16;
	static const int KEY_R = 17;
	static const int KEY_S = 18;
	static const int KEY_T = 19;
	static const int KEY_U = 20;
	static const int KEY_V = 21;
	static const int KEY_W = 22;
	static const int KEY_X = 23;
	static const int KEY_Y = 24;
	static const int KEY_Z = 25;
	static const int KEY_SPACE = 26;
	static const int KEY_LSHIFT = 27;
	static const int KEY_ESCAPE = 28;
	static const int KEY_BACK = 29;
	static const int KEY_RETURN = 30;
	static const int KEY_RSHIFT = 31;
	static const int KEY_UP = 32;
	static const int KEY_DOWN = 33;
	static const int KEY_TAB = 34;
	static const int KEY_LEFT = 35;
	static const int KEY_RIGHT = 36;
};

class Mouse
{
public:
	static void create() {}
	static void destroy() {}
#ifdef _WINDOWS64
	static int getX();
	static int getY();
	static bool isButtonDown(int button);
#else
	static int getX() { return 0; }
	static int getY() { return 0; }
	static bool isButtonDown(int) { return false; }
#endif
};

class Display
{
public:
	static bool isActive() {return true;}
	static void update();
	static void swapBuffers();
	static void destroy() {}
};

class BackgroundDownloader
{
public:
	BackgroundDownloader(File workDir, Minecraft* minecraft) {}
	void start() {}
	void halt() {}
	void forceReload() {}
};

class Color
{
public:
	static int HSBtoRGB(float,float,float) {return 0;}
};
