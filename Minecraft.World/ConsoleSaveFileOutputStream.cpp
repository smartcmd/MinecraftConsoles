#include "stdafx.h"
#include "File.h"
#include "ConsoleSaveFileOutputStream.h"

#include "ConsoleSaveFile.h"

//Creates a file output stream to write to the file represented by the specified File object. A new FileDescriptor object is
//created to represent this file connection.
//First, if there is a security manager, its checkWrite method is called with the path represented by the file argument as its argument.
//
//If the file exists but is a directory rather than a regular file, does not exist but cannot be created, or cannot be opened
//for any other reason then a FileNotFoundException is thrown.
//
//Parameters:
//file - the file to be opened for writing.
ConsoleSaveFileOutputStream::ConsoleSaveFileOutputStream(ConsoleSaveFile *saveFile, const ConsoleSavePath &file)
{
	m_saveFile = saveFile;

	m_file = m_saveFile->createFile(file);

	m_saveFile->setFilePointer( m_file, 0, nullptr, FILE_BEGIN );
}

ConsoleSaveFileOutputStream::ConsoleSaveFileOutputStream(ConsoleSaveFile *saveFile, FileEntry *file)
{
	m_saveFile = saveFile;

	m_file = file;

	m_saveFile->setFilePointer( m_file, 0, nullptr, FILE_BEGIN );
}

//Writes the specified byte to this file output stream. Implements the write method of OutputStream.
//Parameters:
//b - the byte to be written.
void ConsoleSaveFileOutputStream::write(unsigned int b)
{	
	DWORD numberOfBytesWritten;

	byte value = static_cast<byte>(b);

	BOOL result = m_saveFile->writeFile(
		m_file,
		&value, // data buffer
		1, // number of bytes to write
		&numberOfBytesWritten // number of bytes written
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
void ConsoleSaveFileOutputStream::write(byteArray b)
{
	DWORD numberOfBytesWritten;

	BOOL result = m_saveFile->writeFile(
		m_file,
		b.data, // data buffer
		b.length, // number of bytes to write
		&numberOfBytesWritten // number of bytes written
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
void ConsoleSaveFileOutputStream::write(byteArray b, unsigned int offset, unsigned int length)
{
	// 4J Stu - We don't want to write any more than the array buffer holds
	assert( length <= ( b.length - offset ) );

	DWORD numberOfBytesWritten;

	BOOL result = m_saveFile->writeFile(
		m_file,
		&b[offset], // data buffer
		length, // number of bytes to write
		&numberOfBytesWritten // number of bytes written
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
void ConsoleSaveFileOutputStream::close()
{
	if( m_saveFile != nullptr )
	{
		BOOL result = m_saveFile->closeHandle( m_file );

		if( result == 0 )
		{
			// TODO 4J Stu - Some kind of error handling
		}

		// Stop the dtor from trying to close it again
		m_saveFile = nullptr;
	}
}
