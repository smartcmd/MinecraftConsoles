#pragma once
#include <vectormath\cpp\vectormath_aos.h>
class ClipChunk_SPU
{
public:
	void *chunk;
	int globalIdx;
	bool visible;
	float aabb[6];
	int xm, ym, zm;
};


class LevelRenderer_cull_DataIn
{
public:
	static const int sc_listSize = 8000;
	float fdraw[6 * 4];
	Vectormath::Aos::Matrix4 clipMat;
	int numClipChunks;
	ClipChunk_SPU* pClipChunks;
	int numGlobalChunks;
	unsigned char* pGlobalChunkFlags;
	int chunkLists;
	int numToRender_layer0;
	int numToRender_layer1;
	int* listArray_layer0;		// where to output the display lists to be rendered
	int* listArray_layer1;
	float* zDepth_layer0;
	float* zDepth_layer1;
	float maxDepthRender;
	float maxHeightRender;
   	int padding[3];
};
