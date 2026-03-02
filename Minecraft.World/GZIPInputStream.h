#pragma once
// 4J Stu - We are not using GZIP compression, so this is just a pass through class

#include "InputStream.h"

class GZIPInputStream : public InputStream
{
private:
	InputStream *stream;
public:
	GZIPInputStream( InputStream *out ) : stream( out ) {};
	virtual int read() { return stream->read(); };
	virtual int read(byteArray b) { return stream->read( b ); };
	virtual int read(byteArray b, unsigned int offset, unsigned int length) { return stream->read(b, offset, length); };
	virtual void close() { return stream->close(); };
	virtual __int64 skip(__int64 n) { return 0; };
};