#pragma once

#include "Buffer.h"
#include "Definitions.h"

class IntBuffer;
class FloatBuffer;

class ByteBuffer : public Buffer
{
protected:
	byte *buffer;
	ByteOrder byteOrder;

public:
	ByteBuffer(unsigned int capacity);
	static ByteBuffer *allocateDirect(int capacity);
	ByteBuffer( unsigned int capacity, byte *backingArray );
	virtual ~ByteBuffer();

	static ByteBuffer *wrap(byteArray &b);
	static ByteBuffer *allocate(unsigned int capacity);
	void order(ByteOrder a);
	ByteBuffer *flip();
	byte *getBuffer();
	int getSize();
	int getInt();
	int getInt(unsigned int index);
	void get(byteArray) {}	// 4J - TODO
	byte get(int index);
	int64_t getLong();
	short getShort();
	void getShortArray(shortArray &s);
	ByteBuffer *put(int index, byte b);
	ByteBuffer *putInt(int value);
	ByteBuffer *putInt(unsigned int index, int value);
	ByteBuffer *putShort(short value);
	ByteBuffer *putShortArray(shortArray &s);
	ByteBuffer *putLong(int64_t value);
	ByteBuffer *put(byteArray inputArray);
	byteArray array();
	IntBuffer *asIntBuffer();
	FloatBuffer *asFloatBuffer();

};


#ifdef __PS3__
// we're using the RSX now to upload textures to vram, so we need th main ram textures allocated from io space
class ByteBuffer_IO : public ByteBuffer
{
public:
	ByteBuffer_IO(unsigned int capacity);
	~ByteBuffer_IO();
};

#endif // __PS3__