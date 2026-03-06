#pragma once

class ChunkRebuildData;

class Tesselator {
    // private static boolean TRIANGLE_MODE = false;
	friend ChunkRebuildData;
private:
	static bool TRIANGLE_MODE;
    static bool USE_VBO;

    static const int MAX_MEMORY_USE = 16 * 1024 * 1024;
    static const int MAX_FLOATS = MAX_MEMORY_USE / 4 / 2;

    intArray *_array;

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
	float xoo, yoo, zoo;
    int _normal;

	// 4J - added for thread local storage
public:
	static void CreateNewThreadStorage(int bytes);
private:
	static DWORD tlsIdx;
public:
	static Tesselator *getInstance();

private:
	bool tesselating;
	bool mipmapEnable;	// 4J added

    bool vboMode;
    IntBuffer *vboIds;
    int vboId;
    int vboCounts;
    int size;

    Tesselator(int size);
public:
    Tesselator *getUniqueInstance(int size);
    void end();
private:
	void clear();

	// 4J - added to handle compact quad vertex format, which need packaged up as quads
	unsigned int m_ix[4],m_iy[4],m_iz[4];
	unsigned int m_clr[4];
	unsigned int m_u[4], m_v[4];
	unsigned int m_t2[4];
	void packCompactQuad();

#ifdef __PSVITA__
	// AP - alpha cut out is expensive on vita. Use this to defer primitives that use icons with alpha
	bool alphaCutOutEnabled;

	// this is the cut out enabled vertex array
    intArray *_array2;
    int vertices2;
    int p2;
#endif

public:

	// 4J MGH - added, to calculate tight bounds
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

	void begin();
    void begin(int mode);
	void useCompactVertices(bool enable); // 4J added
	bool getCompactVertices();			// AP added
	void useProjectedTexture(bool enable);	// 4J added
    void tex(float u, float v);
	void tex2(int tex2);	// 4J - change brought forward from 1.8.2
    void color(float r, float g, float b);
    void color(float r, float g, float b, float a);
    void color(int r, int g, int b);
    void color(int r, int g, int b, int a);
    void color(uint8_t r, uint8_t g, uint8_t b);
    void vertexUV(float x, float y, float z, float u, float v);
    void vertex(float x, float y, float z);
    void color(int c);
    void color(int c, int alpha);
    void noColor();
    void normal(float x, float y, float z);
    void offset(float xo, float yo, float zo);
    void addOffset(float x, float y, float z);
	bool setMipmapEnable(bool enable);	// 4J added

	bool hasMaxVertices(); // 4J Added
#ifdef __PSVITA__
	// AP - alpha cut out is expensive on vita. Use this to defer primitives that use icons with alpha
	void setAlphaCutOut(bool enable);
	bool getCutOutFound();

	// AP - a faster way of creating a compressed tile quad
	void tileQuad(float x1, float y1, float z1, float u1, float v1, float r1, float g1, float b1, int tex1,
					float x2, float y2, float z2, float u2, float v2, float r2, float g2, float b2, int tex2,
					float x3, float y3, float z3, float u3, float v3, float r3, float g3, float b3, int tex3,
					float x4, float y4, float z4, float u4, float v4, float r4, float g4, float b4, int tex4);
	// AP - a faster way of creating rain quads
	void tileRainQuad(float x1, float y1, float z1, float u1, float v1, 
								float x2, float y2, float z2, float u2, float v2, 
								float x3, float y3, float z3, float u3, float v3, 
								float x4, float y4, float z4, float u4, float v4, 
								float r1, float g1, float b1, float a1,
								float r2, float g2, float b2, float a2, int tex1);
	// AP - a faster way of creating particles
	void tileParticleQuad(float x1, float y1, float z1, float u1, float v1, 
								float x2, float y2, float z2, float u2, float v2, 
								float x3, float y3, float z3, float u3, float v3, 
								float x4, float y4, float z4, float u4, float v4, 
								float r1, float g1, float b1, float a1);
#endif

};
