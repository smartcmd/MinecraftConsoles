#pragma once

class DataInput
{
public:
	virtual int read() = 0;
	virtual int read(byteArray b) = 0;
	virtual int read(byteArray b, unsigned int offset, unsigned int length) = 0;
	virtual bool readBoolean() = 0;
	virtual uint8_t readByte() = 0;
	virtual unsigned char readUnsignedByte() = 0;
	virtual bool readFully(byteArray a) = 0;
	virtual double readDouble() = 0;
	virtual float readFloat() = 0;
	virtual int readInt() = 0;
	virtual __int64 readLong() = 0;
	virtual short readShort() = 0;
	virtual wchar_t readChar() = 0;
	virtual wstring readUTF() = 0;
	virtual PlayerUID readPlayerUID() = 0; // 4J Added
	virtual int skipBytes(int n) = 0;
};
