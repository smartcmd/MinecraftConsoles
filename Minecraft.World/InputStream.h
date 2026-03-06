#pragma once
using namespace std;
// 4J Stu - Represents Java standard lib abstract

class InputStream
{
public:
	virtual ~InputStream() {}

	virtual int read() = 0;
	virtual int read(byteArray b) = 0;
	virtual int read(byteArray b, unsigned int offset, unsigned int length) = 0;
	virtual void close() = 0;
	virtual int64_t skip(int64_t n) = 0;

	static InputStream *getResourceAsStream(const wstring &fileName);
};