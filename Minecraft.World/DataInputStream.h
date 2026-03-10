#pragma once
// 4J Stu - Represents Java standard library class (although we miss out an intermediate inheritance class that we don't care about)

#include "InputStream.h"
#include "DataInput.h"

class DataInputStream : public InputStream, public DataInput
{
private:
	InputStream *stream;

public:
	DataInputStream(InputStream *in);
	virtual int read();
	virtual int read(byteArray b);
	virtual int read(byteArray b, unsigned int offset, unsigned int length);
	virtual void close();
	virtual bool readBoolean();
	virtual byte readByte();
	virtual unsigned char readUnsignedByte();
	virtual unsigned short readUnsignedShort();
	virtual wchar_t readChar();
	virtual bool readFully(byteArray b);
	virtual bool readFully(charArray b);
	virtual double readDouble();
	virtual float readFloat();
	virtual int readInt();
	virtual int64_t readLong();
	virtual short readShort();
	virtual wstring readUTF();
	void deleteChildStream();
	virtual int readUTFChar();
	virtual PlayerUID readPlayerUID(); // 4J Added
	virtual int64_t skip(int64_t n);
	virtual int skipBytes(int n);
};