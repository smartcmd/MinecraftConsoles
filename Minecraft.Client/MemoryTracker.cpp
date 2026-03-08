#include "stdafx.h"
#include "MemoryTracker.h"
#include "../Minecraft.World/IntBuffer.h"
#include "../Minecraft.World/ByteBuffer.h"
#include "../Minecraft.World/FloatBuffer.h"

unordered_map<int,int> MemoryTracker::GL_LIST_IDS;
vector<int> MemoryTracker::TEXTURE_IDS;

int MemoryTracker::genLists(int count)
{
	int id = glGenLists(count);
	GL_LIST_IDS.insert( pair<int,int>(id,count) );
	return id;
}

int MemoryTracker::genTextures()
{
	int id = glGenTextures();
	TEXTURE_IDS.push_back(id);
	return id;
}

void MemoryTracker::releaseLists(int id)
{
    auto it = GL_LIST_IDS.find(id);
    if( it != GL_LIST_IDS.end() )
	{
		glDeleteLists(id, it->second);
		GL_LIST_IDS.erase(it);
	}
}

void MemoryTracker::releaseTextures()
{
	for (int i = 0; i < TEXTURE_IDS.size(); i++)
	{
		glDeleteTextures(TEXTURE_IDS.at(i));
	}
	TEXTURE_IDS.clear();
}

void MemoryTracker::release()
{
	//for (Map.Entry<Integer, Integer> entry : GL_LIST_IDS.entrySet())
	for(auto& it : GL_LIST_IDS)
	{
		glDeleteLists(it.first, it.second);
	}
	GL_LIST_IDS.clear();

	releaseTextures();
}

ByteBuffer *MemoryTracker::createByteBuffer(int size)
{
	// 4J - was ByteBuffer.allocateDirect(size).order(ByteOrder.nativeOrder())
	ByteBuffer *bb = ByteBuffer::allocate(size);
	return bb;
}

IntBuffer *MemoryTracker::createIntBuffer(int size)
{
	return createByteBuffer(size << 2)->asIntBuffer();
}

FloatBuffer *MemoryTracker::createFloatBuffer(int size)
{
	return createByteBuffer(size << 2)->asFloatBuffer();
}