#pragma once

#ifdef __PS3__
#ifdef SN_TARGET_PS3_SPU
typedef unsigned int DWORD;
typedef unsigned char byte;
#include <float.h>
#else 
#include "..\..\..\stdafx.h"
class ChunkRebuildData;
#endif
#endif
class intArray_SPU
{
public:
	unsigned int* data;

	intArray_SPU(unsigned int* preAllocData) {	data = preAllocData;  }
	~intArray_SPU(){ }
};

class ChunkRebuildData;

class Tesselator_SPU 
{
#ifndef  SN_TARGET_PS3_SPU
	friend ChunkRebuildData;
#endif
    // private static boolean TRIANGLE_MODE = false;
public:
	static bool TRIANGLE_MODE;
    static bool USE_VBO;

    static const int MAX_MEMORY_USE = 16 * 1024 * 1024;
    static const int MAX_FLOATS = MAX_MEMORY_USE / 4 / 2;

	void*			m_PPUArray;
	int				m_PPUOffset;
public:
    intArray_SPU*	_array;
private:
    int vertices;
    float u, v;
	int _tex2;
    int col;
    bool hasColor;
    bool hasTexture;
	bool hasTexture2;
    bool hasNormal;
    int p ;
	bool	useCompactFormat360; // 4J - added
	bool	useProjectedTexturePixelShader;	// 4J - added
public:
    int count;
private:
    bool _noColor;
    int mode;
    float xo, yo, zo;
    int _normal;

	// 4J - added for thread local storage
public:
// 	static void CreateNewThreadStorage(int bytes);
private:
// 	static DWORD tlsIdx;
public:
 	static Tesselator_SPU *getInstance();

private:
	bool tesselating;
	bool mipmapEnable;	// 4J added

    bool vboMode;
//     IntBuffer *vboIds;
    int vboId;
    int vboCounts;
    int size;

public:
	Tesselator_SPU() {}
     void end();
private:
	void clear();

	// 4J - added to handle compact quad vertex format, which need packaged up as quads
	unsigned int m_ix[4],m_iy[4],m_iz[4];
	unsigned int m_clr[4];
	unsigned int m_u[4], m_v[4];
	unsigned int m_t2[4];
	void packCompactQuad();
	void incData(int numInts);
public:

	class Bounds
	{
	public:
		void reset()
		{
			boundingBox[0] = FLT_MAX;
			boundingBox[1] = FLT_MAX;
			boundingBox[2] = FLT_MAX;
			boundingBox[3] = -FLT_MAX;
			boundingBox[4] = -FLT_MAX;
			boundingBox[5] = -FLT_MAX;
		}
		void addVert(float x, float y, float z)
		{
			if(x < boundingBox[0])
				boundingBox[0] = x;
			if(y < boundingBox[1])
				boundingBox[1] = y;
			if(z < boundingBox[2])
				boundingBox[2] = z;

			if(x > boundingBox[3])
				boundingBox[3] = x;
			if(y > boundingBox[4])
				boundingBox[4] = y;
			if(z > boundingBox[5])
				boundingBox[5] = z;
		}
		void addBounds(Bounds& ob)
		{
			if(ob.boundingBox[0] < boundingBox[0])
				boundingBox[0] = ob.boundingBox[0];
			if(ob.boundingBox[1] < boundingBox[1])
				boundingBox[1] = ob.boundingBox[1];
			if(ob.boundingBox[2] < boundingBox[2])
				boundingBox[2] = ob.boundingBox[2];

			if(ob.boundingBox[3] > boundingBox[3])
				boundingBox[3] = ob.boundingBox[3];
			if(ob.boundingBox[4] > boundingBox[4])
				boundingBox[4] = ob.boundingBox[4];
			if(ob.boundingBox[5] > boundingBox[5])
				boundingBox[5] = ob.boundingBox[5];
	}
		float boundingBox[6];	// 4J MGH added

	} bounds;



	void beginData();
	void endData();

	void begin();
    void begin(int mode);
	void useCompactVertices(bool enable); // 4J added
	void useProjectedTexture(bool enable);	// 4J added
    void tex(float u, float v);
	void tex2(int tex2);	// 4J - change brought forward from 1.8.2
    void color(float r, float g, float b);
    void color(float r, float g, float b, float a);
    void color(int r, int g, int b);
    void color(int r, int g, int b, int a);
    void color(byte r, byte g, byte b);
    void vertexUV(float x, float y, float z, float u, float v);
    void vertex(float x, float y, float z);
    void color(int c);
    void color(int c, int alpha);
    void noColor();
    void normal(float x, float y, float z);
    void offset(float xo, float yo, float zo);
    void addOffset(float x, float y, float z);
	bool setMipmapEnable(bool enable);	// 4J added
};
