#include "stdafx.h"

#include "File.h"
#include "FileInputStream.h"

//Creates a FileInputStream by opening a connection to an actual file, the file named by the File object file in the file system.
//A new FileDescriptor object is created to represent this file connection.
//First, if there is a security manager, its checkRead method is called with the path represented by the file argument as its argument.
//
//If the named file does not exist, is a directory rather than a regular file, or for some other reason cannot be opened for reading
//then a FileNotFoundException is thrown.
//
//Parameters:
//file - the file to be opened for reading.
//Throws:
//FileNotFoundException - if the file does not exist, is a directory rather than a regular file, or for some other reason cannot be
//opened for reading.
//SecurityException - if a security manager exists and its checkRead method denies read access to the file.
FileInputStream::FileInputStream(const File &file)
{
	const char *pchFilename=wstringtofilename(file.getPath());
#ifdef _UNICODE
	m_fileHandle = CreateFile(
		file.getPath().c_str(), // file name
		GENERIC_READ, // access mode
		0, // share mode // TODO 4J Stu - Will we need to share file? Probably not but...
		nullptr, // Unused
		OPEN_EXISTING , // how to create // TODO 4J Stu - Assuming that the file already exists if we are opening to read from it
		FILE_FLAG_SEQUENTIAL_SCAN, // file attributes
		nullptr // Unsupported
		);
#else
	m_fileHandle = CreateFile(
		pchFilename, // file name
		GENERIC_READ, // access mode
		0, // share mode // TODO 4J Stu - Will we need to share file? Probably not but...
		nullptr, // Unused
		OPEN_EXISTING , // how to create // TODO 4J Stu - Assuming that the file already exists if we are opening to read from it
		FILE_FLAG_SEQUENTIAL_SCAN, // file attributes
		nullptr // Unsupported
		);
#endif

	if( m_fileHandle == INVALID_HANDLE_VALUE )
	{
		// TODO 4J Stu - Any form of error/exception handling
		//__debugbreak();
		app.FatalLoadError();
	}
}

FileInputStream::~FileInputStream()
{
	if( m_fileHandle != INVALID_HANDLE_VALUE )
		CloseHandle( m_fileHandle );
}

//Reads a byte of data from this input stream. This method blocks if no input is yet available.
//Returns:
//the next byte of data, or -1 if the end of the file is reached.
int FileInputStream::read()
{
	byte byteRead = 0;
	DWORD numberOfBytesRead;

	BOOL bSuccess = ReadFile(
		m_fileHandle, // handle to file
		&byteRead, // data buffer
		1, // number of bytes to read
		&numberOfBytesRead, // number of bytes read
		nullptr // overlapped buffer
		);

	if( bSuccess==FALSE )
	{
		// TODO 4J Stu - Some kind of error handling
		app.FatalLoadError();
		//return -1;
	}
	else if( numberOfBytesRead == 0 )
	{
		// File pointer is past the end of the file
		return -1;
	}

	return byteRead;
}

//Reads up to b.length bytes of data from this input stream into an array of bytes. This method blocks until some input is available.
//Parameters:
//b - the buffer into which the data is read.
//Returns:
//the total number of bytes read into the buffer, or -1 if there is no more data because the end of the file has been reached.
int FileInputStream::read(byteArray b)
{
	DWORD numberOfBytesRead;

	BOOL bSuccess = ReadFile(
		m_fileHandle, // handle to file
		b.data, // data buffer
		b.length, // number of bytes to read
		&numberOfBytesRead, // number of bytes read
		nullptr // overlapped buffer
		);

	if( bSuccess==FALSE )
	{
		// TODO 4J Stu - Some kind of error handling
		app.FatalLoadError();
		//return -1;
	}
	else if( numberOfBytesRead == 0 )
	{
		// File pointer is past the end of the file
		return -1;
	}

	return numberOfBytesRead;
}

//Reads up to len bytes of data from this input stream into an array of bytes. If len is not zero, the method blocks until some input
//is available; otherwise, no bytes are read and 0 is returned.
//Parameters:
//b - the buffer into which the data is read.
//off - the start offset in the destination array b
//len - the maximum number of bytes read.
//Returns:
//the total number of bytes read into the buffer, or -1 if there is no more data because the end of the file has been reached.
int FileInputStream::read(byteArray b, unsigned int offset, unsigned int length)
{
	// 4J Stu - We don't want to read any more than the array buffer can hold
	assert( length <= ( b.length - offset ) );

	DWORD numberOfBytesRead;

	BOOL bSuccess = ReadFile(
		m_fileHandle, // handle to file
		&b[offset], // data buffer
		length, // number of bytes to read
		&numberOfBytesRead, // number of bytes read
		nullptr // overlapped buffer
		);

	if( bSuccess==FALSE )
	{
		// TODO 4J Stu - Some kind of error handling
		app.FatalLoadError();
		//return -1;
	}
	else if( numberOfBytesRead == 0 )
	{
		// File pointer is past the end of the file
		return -1;
	}

	return numberOfBytesRead;
}

//Closes this file input stream and releases any system resources associated with the stream.
//If this stream has an associated channel then the channel is closed as well.
void FileInputStream::close()
{
	if(m_fileHandle==INVALID_HANDLE_VALUE)
	{
		//printf("\n\nFileInputStream::close - TRYING TO CLOSE AN INVALID FILE HANDLE\n\n");
		return;
	}

	BOOL result = CloseHandle( m_fileHandle );

	if( result == 0 )
	{
		// TODO 4J Stu - Some kind of error handling
	}

	// Stop the dtor from trying to close it again
	m_fileHandle = INVALID_HANDLE_VALUE;
}


//Skips n bytes of input from this input stream. Fewer bytes might be skipped if the end of the input stream is reached. The actual number k of bytes to be skipped is equal to the smaller of n and count-pos. The value k is added into pos and k is returned.
//Overrides:
//skip in class InputStream
//Parameters:
//n - the number of bytes to be skipped.
//Returns:
//the actual number of bytes skipped.
int64_t FileInputStream::skip(int64_t n)
{
#ifdef _XBOX
	LARGE_INTEGER li;
	li.QuadPart = n;
	li.LowPart = SetFilePointer(m_fileHandle, li.LowPart, &li.HighPart, FILE_CURRENT);

	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
	{
		li.QuadPart = 0;
	}

	return li.QuadPart;
#else
	return 0;
#endif
}