#pragma once
// 4J Stu - Represents Java standard library class

#include "InputStream.h"

class ByteArrayInputStream : public InputStream
{
protected:
	byteArray buf; //An array of bytes that was provided by the creator of the stream.
	unsigned int count; //The index one greater than the last valid character in the input stream buffer.
	unsigned int mark; //The currently marked position in the stream.
	unsigned int pos; //The index of the next character to read from the input stream buffer.

public:
	ByteArrayInputStream(byteArray buf, unsigned int offset, unsigned int length);
	ByteArrayInputStream(byteArray buf);
	virtual ~ByteArrayInputStream();
	virtual int read();
	virtual int read(byteArray b);
	virtual int read(byteArray b, unsigned int offset, unsigned int length);
	virtual void close();
	virtual int64_t skip(int64_t n);

	// 4J Stu Added - Sometimes we don't want to delete the data on destroying this
	void reset() { buf = byteArray(); count = 0; mark = 0; pos = 0; }
};