#include "stdafx.h"
#include "BasicTypeContainers.h"

#include "DataInputStream.h"

//Creates a DataInputStream that uses the specified underlying InputStream.
//Parameters:
//in - the specified input stream
DataInputStream::DataInputStream(InputStream *in) : stream( in )
{
}

//Reads the next byte of data from this input stream. The value byte is returned as an int in the range 0 to 255.
//If no byte is available because the end of the stream has been reached, the value -1 is returned.
//This method blocks until input data is available, the end of the stream is detected, or an exception is thrown.
//This method simply performs in.read() and returns the result.
int DataInputStream::read()
{
	return stream->read();
}

//Reads some number of bytes from the contained input stream and stores them into the buffer array b.
//The number of bytes actually read is returned as an integer. This method blocks until input data is available,
//end of file is detected, or an exception is thrown.
//If b is null, a NullPointerException is thrown. If the length of b is zero, then no bytes are read and 0 is returned;
//otherwise, there is an attempt to read at least one byte. If no byte is available because the stream is at end of file,
//the value -1 is returned; otherwise, at least one byte is read and stored into b.
//
//The first byte read is stored into element b[0], the next one into b[1], and so on. The number of bytes read is, at most,
//equal to the length of b. Let k be the number of bytes actually read; these bytes will be stored in elements b[0] through b[k-1],
//leaving elements b[k] through b[b.length-1] unaffected.
//
//The read(b) method has the same effect as:
//
// read(b, 0, b.length) 
// 
//Overrides:
//read in class FilterInputStream
//Parameters:
//b - the buffer into which the data is read.
//Returns:
//the total number of bytes read into the buffer, or -1 if there is no more data because the end of the stream has been reached.
int DataInputStream::read(byteArray b)
{
	return read( b, 0, b.length );
}

//Reads up to len bytes of data from the contained input stream into an array of bytes. An attempt is made to read as many as len bytes,
//but a smaller number may be read, possibly zero. The number of bytes actually read is returned as an integer.
//This method blocks until input data is available, end of file is detected, or an exception is thrown.
//
//If len is zero, then no bytes are read and 0 is returned; otherwise, there is an attempt to read at least one byte.
//If no byte is available because the stream is at end of file, the value -1 is returned; otherwise, at least one byte is read and stored into b.
//
//The first byte read is stored into element b[off], the next one into b[off+1], and so on. The number of bytes read is,
//at most, equal to len. Let k be the number of bytes actually read; these bytes will be stored in elements b[off] through b[off+k-1],
//leaving elements b[off+k] through b[off+len-1] unaffected.
//
//In every case, elements b[0] through b[off] and elements b[off+len] through b[b.length-1] are unaffected.
//
//Overrides:
//read in class FilterInputStream
//Parameters:
//b - the buffer into which the data is read.
//off - the start offset in the destination array b
//len - the maximum number of bytes read.
//Returns:
//the total number of bytes read into the buffer, or -1 if there is no more data because the end of the stream has been reached.
int DataInputStream::read(byteArray b, unsigned int offset, unsigned int length)
{
	return stream->read( b, offset, length );
}

//Closes this input stream and releases any system resources associated with the stream. This method simply performs in.close()
void DataInputStream::close()
{
	stream->close();
}

//Reads one input byte and returns true if that byte is nonzero, false if that byte is zero. This method is suitable for reading
//the byte written by the writeBoolean method of interface DataOutput.
//Returns:
//the boolean value read.
bool DataInputStream::readBoolean()
{
	return stream->read() != 0;
}

//Reads and returns one input byte. The byte is treated as a signed value in the range -128 through 127, inclusive.
//This method is suitable for reading the byte written by the writeByte method of interface DataOutput.
//Returns:
//the 8-bit value read.
byte DataInputStream::readByte()
{
	return (byte) stream->read();
}

unsigned char DataInputStream::readUnsignedByte()
{
	return (unsigned char) stream->read();
}

//Reads two input bytes and returns a char value. Let a be the first byte read and b be the second byte. The value returned is:
//(char)((a << 8) | (b & 0xff))
// 
//This method is suitable for reading bytes written by the writeChar method of interface DataOutput.
//Returns:
//the char value read.
wchar_t DataInputStream::readChar()
{
	int a = stream->read();
	int b = stream->read();
	return (wchar_t)((a << 8) | (b & 0xff)); 
}

//Reads some bytes from an input stream and stores them into the buffer array b. The number of bytes read is equal to the length of b.
//This method blocks until one of the following conditions occurs:
//
//b.length bytes of input data are available, in which case a normal return is made.
//End of file is detected, in which case an EOFException is thrown.
//An I/O error occurs, in which case an IOException other than EOFException is thrown.
//If b is null, a NullPointerException is thrown. If b.length is zero, then no bytes are read. Otherwise, the first byte read is
//stored into element b[0], the next one into b[1], and so on. If an exception is thrown from this method, then it may be that some but
//not all bytes of b have been updated with data from the input stream.
//
//Parameters:
//b - the buffer into which the data is read.
bool DataInputStream::readFully(byteArray b)
{
	// TODO 4J Stu - I am not entirely sure if this matches the implementation of the Java library
	// TODO 4J Stu - Need to handle exceptions here is we throw them in other InputStreams
	for(unsigned int i = 0; i < b.length ;i++)
	{
		int byteRead = stream->read();
		if( byteRead == -1 )
		{
			return false;
		}
		else
		{
			b[i] = byteRead;
		}
	}
	return true;
}

bool DataInputStream::readFully(charArray b)
{
	// TODO 4J Stu - I am not entirely sure if this matches the implementation of the Java library
	// TODO 4J Stu - Need to handle exceptions here is we throw them in other InputStreams
	for(unsigned int i = 0; i < b.length ;i++)
	{
		int byteRead = stream->read();
		if( byteRead == -1 )
		{
			return false;
		}
		else
		{
			b[i] = byteRead;
		}
	}
	return true;
}

//Reads eight input bytes and returns a double value. It does this by first constructing a long value in exactly the manner
//of the readlong method, then converting this long value to a double in exactly the manner of the method Double.longBitsToDouble.
//This method is suitable for reading bytes written by the writeDouble method of interface DataOutput.
//Returns:
//the double value read.
double DataInputStream::readDouble()
{
	__int64 bits = readLong();

	return Double::longBitsToDouble( bits );
}

//Reads four input bytes and returns a float value. It does this by first constructing an int value in exactly the manner
//of the readInt method, then converting this int value to a float in exactly the manner of the method Float.intBitsToFloat.
//This method is suitable for reading bytes written by the writeFloat method of interface DataOutput.
//Returns:
//the float value read.
float DataInputStream::readFloat()
{
	int bits = readInt();

	return Float::intBitsToFloat( bits );
}

//Reads four input bytes and returns an int value. Let a-d be the first through fourth bytes read. The value returned is:
// 
// (((a & 0xff) << 24) | ((b & 0xff) << 16) |
//  ((c & 0xff) << 8) | (d & 0xff))
// 
//This method is suitable for reading bytes written by the writeInt method of interface DataOutput.
//Returns:
//the int value read.
int DataInputStream::readInt()
{
	int a = stream->read();
	int b = stream->read();
	int c = stream->read();
	int d = stream->read();
	int bits = (((a & 0xff) << 24) | ((b & 0xff) << 16) |
		((c & 0xff) << 8) | (d & 0xff));
	return bits;
}

//Reads eight input bytes and returns a long value. Let a-h be the first through eighth bytes read. The value returned is:
// 
// (((long)(a & 0xff) << 56) |
//  ((long)(b & 0xff) << 48) |
//  ((long)(c & 0xff) << 40) |
//  ((long)(d & 0xff) << 32) |
//  ((long)(e & 0xff) << 24) |
//  ((long)(f & 0xff) << 16) |
//  ((long)(g & 0xff) <<  8) |
//  ((long)(h & 0xff)))
// 
//This method is suitable for reading bytes written by the writeLong method of interface DataOutput.
//
//Returns:
//the long value read.
__int64 DataInputStream::readLong()
{
	__int64 a = stream->read();
	__int64 b = stream->read();
	__int64 c = stream->read();
	__int64 d = stream->read();
	__int64 e = stream->read();
	__int64 f = stream->read();
	__int64 g = stream->read();
	__int64 h = stream->read();

	__int64 bits = (((a & 0xff) << 56) |
		((b & 0xff) << 48) |
		((c & 0xff) << 40) |
		((d & 0xff) << 32) |
		((e & 0xff) << 24) |
		((f & 0xff) << 16) |
		((g & 0xff) <<  8) |
		((h & 0xff)));

	return bits;
}

//Reads two input bytes and returns a short value. Let a be the first byte read and b be the second byte. The value returned is:
//(short)((a << 8) | (b & 0xff))
// 
//This method is suitable for reading the bytes written by the writeShort method of interface DataOutput.
//Returns:
//the 16-bit value read.
short DataInputStream::readShort()
{
	int a = stream->read();
	int b = stream->read();
	return (short)((a << 8) | (b & 0xff));
}

//Reads in a string that has been encoded using a modified UTF-8 format. The general contract of readUTF is that it reads a representation
//of a Unicode character string encoded in modified UTF-8 format; this string of characters is then returned as a String.
//First, two bytes are read and used to construct an unsigned 16-bit integer in exactly the manner of the readUnsignedShort method .
//This integer value is called the UTF length and specifies the number of additional bytes to be read. These bytes are then converted
//to characters by considering them in groups. The length of each group is computed from the value of the first byte of the group.
//The byte following a group, if any, is the first byte of the next group.
//
//If the first byte of a group matches the bit pattern 0xxxxxxx (where x means "may be 0 or 1"), then the group consists of just that byte.
//The byte is zero-extended to form a character.
//
//If the first byte of a group matches the bit pattern 110xxxxx, then the group consists of that byte a and a second byte b.
//If there is no byte b (because byte a was the last of the bytes to be read), or if byte b does not match the bit pattern 10xxxxxx,
//then a UTFDataFormatException is thrown. Otherwise, the group is converted to the character:
//
//(char)(((a& 0x1F) << 6) | (b & 0x3F))
// 
//If the first byte of a group matches the bit pattern 1110xxxx, then the group consists of that byte a and two more bytes b and c.
//If there is no byte c (because byte a was one of the last two of the bytes to be read), or either byte b or byte c does not match the bit
//pattern 10xxxxxx, then a UTFDataFormatException is thrown. Otherwise, the group is converted to the character:
//
// (char)(((a & 0x0F) << 12) | ((b & 0x3F) << 6) | (c & 0x3F))
// 
//If the first byte of a group matches the pattern 1111xxxx or the pattern 10xxxxxx, then a UTFDataFormatException is thrown.
//If end of file is encountered at any time during this entire process, then an EOFException is thrown.
//
//After every group has been converted to a character by this process, the characters are gathered, in the same order in which their
//corresponding groups were read from the input stream, to form a String, which is returned.
//
//The writeUTF method of interface DataOutput may be used to write data that is suitable for reading by this method.
//
//Returns:
//a Unicode string.
wstring DataInputStream::readUTF()
{
	wstring outputString;
	int a = stream->read();
	int b = stream->read();
	unsigned short UTFLength = (unsigned short) (((a & 0xff) << 8) | (b & 0xff));

	//// 4J Stu - I decided while writing DataOutputStream that we didn't need to bother using the UTF8 format
	//// used in the java libs, and just write in/out as wchar_t all the time

	/*for( unsigned short i = 0; i < UTFLength; i++)
	{
		wchar_t theChar = readChar();
		outputString.push_back(theChar);
	}*/

	
	unsigned short currentByteIndex = 0;
	while( currentByteIndex < UTFLength )
	{
		int firstByte = stream->read();
		currentByteIndex++;

		if( firstByte == -1 )
			// TODO 4J Stu - EOFException
			break;

		// Masking patterns:
		// 10000000 = 0x80 // Match only highest bit
		// 11000000 = 0xC0 // Match only highest two bits
		// 11100000 = 0xE0 // Match only highest three bits
		// 11110000 = 0xF0 // Match only highest four bits

		// Matching patterns:
		// 10xxxxxx = 0x80 // ERROR, or second/third byte
		// 1111xxxx = 0xF0 //ERROR
		// 0xxxxxxx = 0x00 // One byte UTF
		// 110xxxxx = 0xC0 // Two byte UTF
		// 1110xxxx = 0xE0 // Three byte UTF
		if( ( (firstByte & 0xC0 ) == 0x80 ) || ( (firstByte & 0xF0) == 0xF0) )
		{
			// TODO 4J Stu - UTFDataFormatException
			break;
		}
		else if( (firstByte & 0x80) == 0x00 )
		{
			// One byte UTF
			wchar_t readChar = (wchar_t)firstByte;
			outputString.push_back( readChar );
			continue;
		}
		else if( (firstByte & 0xE0) == 0xC0 )
		{
			// Two byte UTF

			// No more bytes to read
			if( !(currentByteIndex < UTFLength) )
			{
				// TODO 4J Stu - UTFDataFormatException
				break;
			}

			int secondByte = stream->read();
			currentByteIndex++;

			// No second byte
			if( secondByte == -1 )
			{
				// TODO 4J Stu - EOFException
				break;
			}
			// Incorrect second byte pattern
			else if( (secondByte & 0xC0 ) != 0x80 )
			{
				// TODO 4J Stu - UTFDataFormatException
				break;
			}

			wchar_t readChar = (wchar_t)( ((firstByte& 0x1F) << 6) | (secondByte & 0x3F) );
			outputString.push_back( readChar );
			continue;
		}
		else if( (firstByte & 0xF0) == 0xE0 )
		{
			// Three byte UTF

			// No more bytes to read
			if( !(currentByteIndex < UTFLength) )
			{
				// TODO 4J Stu - UTFDataFormatException
				break;
			}

			int secondByte = stream->read();
			currentByteIndex++;

			// No second byte
			if( secondByte == -1 )
			{
				// TODO 4J Stu - EOFException
				break;
			}			

			// No more bytes to read
			if( !(currentByteIndex < UTFLength) )
			{
				// TODO 4J Stu - UTFDataFormatException
				break;
			}

			int thirdByte = stream->read();
			currentByteIndex++;

			// No third byte
			if( thirdByte == -1 )
			{
				// TODO 4J Stu - EOFException
				break;
			}
			// Incorrect second or third byte pattern
			else if( ( (secondByte & 0xC0 ) != 0x80 ) || ( (thirdByte & 0xC0 ) != 0x80 ) )
			{
				// TODO 4J Stu - UTFDataFormatException
				break;
			}

			wchar_t readChar = (wchar_t)(((firstByte & 0x0F) << 12) | ((secondByte & 0x3F) << 6) | (thirdByte & 0x3F));
			outputString.push_back( readChar );
			continue;
		}
	}
	
	return outputString;
}

int DataInputStream::readUTFChar()
{
	int returnValue = -1;
	int firstByte = stream->read();

	if( firstByte == -1 )
		// TODO 4J Stu - EOFException
		return returnValue;

	// Masking patterns:
	// 10000000 = 0x80 // Match only highest bit
	// 11000000 = 0xC0 // Match only highest two bits
	// 11100000 = 0xE0 // Match only highest three bits
	// 11110000 = 0xF0 // Match only highest four bits

	// Matching patterns:
	// 10xxxxxx = 0x80 // ERROR, or second/third byte
	// 1111xxxx = 0xF0 //ERROR
	// 0xxxxxxx = 0x00 // One byte UTF
	// 110xxxxx = 0xC0 // Two byte UTF
	// 1110xxxx = 0xE0 // Three byte UTF
	if( ( (firstByte & 0xC0 ) == 0x80 ) || ( (firstByte & 0xF0) == 0xF0) )
	{
		// TODO 4J Stu - UTFDataFormatException
		return returnValue;
	}
	else if( (firstByte & 0x80) == 0x00 )
	{
		// One byte UTF
		returnValue = firstByte;
	}
	else if( (firstByte & 0xE0) == 0xC0 )
	{
		// Two byte UTF
		int secondByte = stream->read();

		// No second byte
		if( secondByte == -1 )
		{
			// TODO 4J Stu - EOFException
			return returnValue;
		}
		// Incorrect second byte pattern
		else if( (secondByte & 0xC0 ) != 0x80 )
		{
			// TODO 4J Stu - UTFDataFormatException
			return returnValue;
		}

		returnValue = ((firstByte& 0x1F) << 6) | (secondByte & 0x3F);
	}
	else if( (firstByte & 0xF0) == 0xE0 )
	{
		// Three byte UTF

		int secondByte = stream->read();

		// No second byte
		if( secondByte == -1 )
		{
			// TODO 4J Stu - EOFException
			return returnValue;
		}			

		int thirdByte = stream->read();

		// No third byte
		if( thirdByte == -1 )
		{
			// TODO 4J Stu - EOFException
			return returnValue;
		}
		// Incorrect second or third byte pattern
		else if( ( (secondByte & 0xC0 ) != 0x80 ) || ( (thirdByte & 0xC0 ) != 0x80 ) )
		{
			// TODO 4J Stu - UTFDataFormatException
			return returnValue;
		}

		returnValue = (((firstByte & 0x0F) << 12) | ((secondByte & 0x3F) << 6) | (thirdByte & 0x3F));
	}
	return returnValue;
}

// 4J Added
PlayerUID DataInputStream::readPlayerUID()
{
	PlayerUID returnValue;
#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
	for(int idPos=0;idPos<sizeof(PlayerUID); idPos++)
		((char*)&returnValue)[idPos] = readByte();
#elif defined(_DURANGO)
	returnValue = readUTF();
#else
	returnValue = readLong();
#endif // PS3
	return returnValue;
}

void DataInputStream::deleteChildStream()
{
	delete stream;
}

//Skips n bytes of input from this input stream. Fewer bytes might be skipped if the end of the input stream is reached. The actual number k of bytes to be skipped is equal to the smaller of n and count-pos. The value k is added into pos and k is returned.
//Overrides:
//skip in class InputStream
//Parameters:
//n - the number of bytes to be skipped.
//Returns:
//the actual number of bytes skipped.
__int64 DataInputStream::skip(__int64 n)
{
	return stream->skip(n);
}

int DataInputStream::skipBytes(int n)
{
	return skip(n);
}