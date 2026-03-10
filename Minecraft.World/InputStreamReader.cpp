#include "stdafx.h"

#include "InputStream.h"
#include "DataInputStream.h"
#include "InputStreamReader.h"

//Creates an InputStreamReader that uses the default charset.
//Parameters:
//in - An InputStream
InputStreamReader::InputStreamReader(InputStream *in) : stream( new DataInputStream( in ) )
{
}

//Closes the stream and releases any system resources associated with it.
//Once the stream has been closed, further read(), ready(), mark(), reset(), or skip() invocations will throw an IOException.
//Closing a previously closed stream has no effect.
void InputStreamReader::close()
{
	stream->close();
}

//Reads a single character.
//Returns:
//The character read, or -1 if the end of the stream has been reached
int InputStreamReader::read()
{
	return stream->readUTFChar();
}

//Reads characters into a portion of an array.
//Parameters:
//cbuf - Destination buffer
//offset - Offset at which to start storing characters
//length - Maximum number of characters to read
//Returns:
//The number of characters read, or -1 if the end of the stream has been reached
int InputStreamReader::read(wchar_t cbuf[], unsigned int offset, unsigned int length)
{
	unsigned int charsRead = 0;
	for( unsigned int i = offset; i < offset + length; i++ )
	{
		wchar_t value = static_cast<wchar_t>(stream->readUTFChar());
		if( value != -1 )
		{
			cbuf[i] = value;
			charsRead++;
		}
		// TODO 4J Stu - The read might throw an exception? In which case we should return -1
		else break;
	}
	return charsRead;
}