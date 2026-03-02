#pragma once
// 4J Stu - Represents Java standard library class (although we miss out an intermediate inheritance class that we don't care about)

#include "OutputStream.h"
#include "DataOutput.h"

class DataOutputStream : public OutputStream, public DataOutput
{
private:
	OutputStream *stream;

protected :
	int	written; //The number of bytes written to the data output stream so far.

public:
	DataOutputStream( OutputStream *out );

	// 4J Stu Added
	void deleteChildStream();

	virtual void write(unsigned int b);
	virtual void write(byteArray b);
	virtual void write(byteArray b, unsigned int offset, unsigned int length);
	virtual void close();
	virtual void writeByte(byte a);
	virtual void writeDouble(double a);
	virtual void writeFloat(float a);
	virtual void writeInt(int a);
	virtual void writeLong(__int64 a);
	virtual void writeShort(short a);
	virtual void writeChar(wchar_t a);
	virtual void writeChars(const wstring& a);
	virtual void writeBoolean(bool b);	
	virtual void writeUTF(const wstring& a);
	virtual void writePlayerUID(PlayerUID player);
	virtual void flush();
};