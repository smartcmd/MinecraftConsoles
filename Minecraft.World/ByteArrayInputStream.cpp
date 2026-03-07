#include "stdafx.h"

#include "InputOutputStream.h"

//Creates ByteArrayInputStream that uses buf as its buffer array. The initial value of pos is offset and
//the initial value of count is the minimum of offset+length and buf.length. The buffer array is not copied.
//The buffer's mark is set to the specified offset.
//Parameters:
//buf - the input buffer.
//offset - the offset in the buffer of the first byte to read.
//length - the maximum number of bytes to read from the buffer.
ByteArrayInputStream::ByteArrayInputStream(byteArray buf, unsigned int offset, unsigned int length)
	: pos( offset ), count( min( offset+length, buf.length ) ), mark( offset )
{
	this->buf = buf;
}

//Creates a ByteArrayInputStream so that it uses buf as its buffer array. The buffer array is not copied.
//The initial value of pos is 0 and the initial value of count is the length of buf.
//Parameters:
//buf - the input buffer.
ByteArrayInputStream::ByteArrayInputStream(byteArray buf)
	: pos( 0 ), count( buf.length ), mark( 0 )
{
	this->buf = buf;
}

//Reads the next byte of data from this input stream. The value byte is returned as an int in the range 0 to 255.
//If no byte is available because the end of the stream has been reached, the value -1 is returned.
//This read method cannot block.
//Returns:
//the next byte of data, or -1 if the end of the stream has been reached.
int ByteArrayInputStream::read()
{
	if( pos >= count )
		return -1;
	else
		return buf[pos++];
}

//Reads some number of bytes from the input stream and stores them into the buffer array b.
//The number of bytes actually read is returned as an integer. This method blocks until input data is available,
//end of file is detected, or an exception is thrown.
//If the length of b is zero, then no bytes are read and 0 is returned; otherwise, there is an attempt to read at least one byte.
//If no byte is available because the stream is at the end of the file, the value -1 is returned; otherwise,
//at least one byte is read and stored into b.
//
//The first byte read is stored into element b[0], the next one into b[1], and so on. The number of bytes read is,
//at most, equal to the length of b. Let k be the number of bytes actually read; these bytes will be stored in elements b[0] through b[k-1],
//leaving elements b[k] through b[b.length-1] unaffected.
//
//The read(b) method for class InputStream has the same effect as:
//
// read(b, 0, b.length)
//Parameters:
//b - the buffer into which the data is read.
//Returns:
//the total number of bytes read into the buffer, or -1 is there is no more data because the end of the stream has been reached.
int ByteArrayInputStream::read(byteArray b)
{
	return read( b, 0, b.length );
}

//Reads up to len bytes of data into an array of bytes from this input stream. If pos equals count,
//then -1 is returned to indicate end of file. Otherwise, the number k of bytes read is equal to the smaller of len and count-pos.
//If k is positive, then bytes buf[pos] through buf[pos+k-1] are copied into b[off] through b[off+k-1] in the manner
//performed by System.arraycopy. The value k is added into pos and k is returned.
//This read method cannot block.
//Parameters:
//b - the buffer into which the data is read.
//off - the start offset in the destination array b
//len - the maximum number of bytes read.
//Returns:
//the total number of bytes read into the buffer, or -1 if there is no more data because the end of the stream has been reached.
int ByteArrayInputStream::read(byteArray b, unsigned int offset, unsigned int length)
{
	if( pos == count )
		return -1;

	int k = min( length, count-pos );
	XMemCpy( &b[offset], &buf[pos], k );
	//std::copy( buf->data+pos, buf->data+pos+k, b->data + offset ); // Or this instead?

	pos += k;

	return k;
}

//Closing a ByteArrayInputStream has no effect.
//The methods in this class can be called after the stream has been closed without generating an IOException.
void ByteArrayInputStream::close()
{
	return;
}

//Skips n bytes of input from this input stream. Fewer bytes might be skipped if the end of the input stream is reached. The actual number k of bytes to be skipped is equal to the smaller of n and count-pos. The value k is added into pos and k is returned.
//Overrides:
//skip in class InputStream
//Parameters:
//n - the number of bytes to be skipped.
//Returns:
//the actual number of bytes skipped.
int64_t ByteArrayInputStream::skip(int64_t n)
{
	int newPos = pos + n;

	if(newPos > count) newPos = count;

	int k = newPos - pos;
	pos = newPos;

	return k;
}

ByteArrayInputStream::~ByteArrayInputStream()
{
	if(buf.data != NULL) delete [] buf.data;
}