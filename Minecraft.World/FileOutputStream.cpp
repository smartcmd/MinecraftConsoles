#include "stdafx.h"
#include "File.h"
#include "FileOutputStream.h"

//Creates a file output stream to write to the file represented by the specified File object. A new FileDescriptor object is
//created to represent this file connection.
//First, if there is a security manager, its checkWrite method is called with the path represented by the file argument as its argument.
//
//If the file exists but is a directory rather than a regular file, does not exist but cannot be created, or cannot be opened
//for any other reason then a FileNotFoundException is thrown.
//
//Parameters:
//file - the file to be opened for writing.
FileOutputStream::FileOutputStream(const File &file) : m_fileHandle( INVALID_HANDLE_VALUE )
{
	if( file.exists() && file.isDirectory())
	{
		// TODO 4J Stu - FileNotFoundException
		return;
	}

#ifdef _DURANGO
	m_fileHandle = CreateFile(
		file.getPath().c_str() , // file name
		GENERIC_WRITE, // access mode
		0, // share mode // TODO 4J Stu - Will we need to share file? Probably not but...
		nullptr, // Unused
		OPEN_ALWAYS , // how to create
		FILE_ATTRIBUTE_NORMAL , // file attributes
		nullptr // Unsupported
		);
#else
	m_fileHandle = CreateFile(
		wstringtofilename(file.getPath()) , // file name
		GENERIC_WRITE, // access mode
		0, // share mode // TODO 4J Stu - Will we need to share file? Probably not but...
		nullptr, // Unused
		OPEN_ALWAYS , // how to create
		FILE_ATTRIBUTE_NORMAL , // file attributes
		nullptr // Unsupported
		);
#endif

	if( m_fileHandle == INVALID_HANDLE_VALUE )
	{
		DWORD error = GetLastError();
		// TODO 4J Stu - Any form of error/exception handling
	}
}

FileOutputStream::~FileOutputStream()
{
	if( m_fileHandle != INVALID_HANDLE_VALUE )
		CloseHandle( m_fileHandle );
}

//Writes the specified byte to this file output stream. Implements the write method of OutputStream.
//Parameters:
//b - the byte to be written.
void FileOutputStream::write(unsigned int b)
{	
	DWORD numberOfBytesWritten;

	byte value = static_cast<byte>(b);

	BOOL result = WriteFile(
		m_fileHandle, // handle to file
		&value, // data buffer
		1, // number of bytes to write
		&numberOfBytesWritten, // number of bytes written
		nullptr // overlapped buffer
		);

	if( result == 0 )
	{
		// TODO 4J Stu - Some kind of error handling
	}
	else if( numberOfBytesWritten == 0 )
	{
		// File pointer is past the end of the file
	}
}

//Writes b.length bytes from the specified byte array to this file output stream.
//Parameters:
//b - the data.
void FileOutputStream::write(byteArray b)
{
	DWORD numberOfBytesWritten;

	BOOL result = WriteFile(
		m_fileHandle, // handle to file
		&b.data, // data buffer
		b.length, // number of bytes to write
		&numberOfBytesWritten, // number of bytes written
		nullptr // overlapped buffer
		);

	if( result == 0 )
	{
		// TODO 4J Stu - Some kind of error handling
	}
	else if( numberOfBytesWritten == 0 || numberOfBytesWritten != b.length  )
	{
		// File pointer is past the end of the file
	}
}

//Writes len bytes from the specified byte array starting at offset off to this file output stream.
//Parameters:
//b - the data.
//off - the start offset in the data.
//len - the number of bytes to write.
void FileOutputStream::write(byteArray b, unsigned int offset, unsigned int length)
{
	// 4J Stu - We don't want to write any more than the array buffer holds
	assert( length <= ( b.length - offset ) );

	DWORD numberOfBytesWritten;

	BOOL result = WriteFile(
		m_fileHandle, // handle to file
		&b[offset], // data buffer
		length, // number of bytes to write
		&numberOfBytesWritten, // number of bytes written
		nullptr // overlapped buffer
		);

	if( result == 0 )
	{
		// TODO 4J Stu - Some kind of error handling
	}
	else if( numberOfBytesWritten == 0 || numberOfBytesWritten != length  )
	{
		// File pointer is past the end of the file
	}
}
//
//Closes this file output stream and releases any system resources associated with this stream.
//This file output stream may no longer be used for writing bytes.
//If this stream has an associated channel then the channel is closed as well.
void FileOutputStream::close()
{
	BOOL result = CloseHandle( m_fileHandle );

	if( result == 0 )
	{
		// TODO 4J Stu - Some kind of error handling
	}

	// Stop the dtor from trying to close it again
	m_fileHandle = INVALID_HANDLE_VALUE;
}
