#include "stdafx.h"

#include "ByteArrayOutputStream.h"

// Creates a new byte array output stream. The buffer capacity is initially 32 bytes, though its size increases if necessary.
ByteArrayOutputStream::ByteArrayOutputStream()
{
    count = 0;
    buf = byteArray(32);
}

// Creates a new byte array output stream, with a buffer capacity of the specified size, in bytes.
// Parameters:
// size - the initial size.
ByteArrayOutputStream::ByteArrayOutputStream(unsigned int size)
{
    count = 0;
    buf = byteArray(size);
}

ByteArrayOutputStream::~ByteArrayOutputStream()
{
    if (buf.data != NULL)
    {
        delete[] buf.data;
    }
}

// Writes the specified byte to this byte array output stream.
// Parameters:
// b - the byte to be written.
void ByteArrayOutputStream::write(unsigned int b)
{
    // If we will fill the buffer we need to make it bigger
    if (count + 1 >= buf.length)
    {
        buf.resize(buf.length * 2);
    }

    buf[count] = (byte)b;
    count++;
}

// Writes b.length bytes from the specified byte array to this output stream.
// The general contract for write(b) is that it should have exactly the same effect as the call write(b, 0, b.length).
void ByteArrayOutputStream::write(byteArray b)
{
    write(b, 0, b.length);
}

// Writes len bytes from the specified byte array starting at offset off to this byte array output stream.
// Parameters:
// b - the data.
// off - the start offset in the data.
// len - the number of bytes to write.
void ByteArrayOutputStream::write(byteArray b, unsigned int offset, unsigned int length)
{

    if (offset > b.length || length > b.length - offset)
    {
        return;
    }

    if (length > 0xFFFFFFFF - count)
    {
        return;
    }

    // If we will fill the buffer we need to make it bigger
    if (count + length >= buf.length)
    {
        unsigned int newSize = (std::max)(count + length + 1, buf.length * 2);
        if (newSize <= buf.length)
        {
            return;
        }
        buf.resize(newSize);
    }

    XMemCpy(&buf[count], &b[offset], length);

    count += length;
}

// Closing a ByteArrayOutputStream has no effect.
// The methods in this class can be called after the stream has been closed without generating an IOException.
void ByteArrayOutputStream::close()
{
}

// Creates a newly allocated byte array. Its size is the current size of this output stream and the valid contents of the buffer have been copied into it.
// Returns:
// the current contents of this output stream, as a byte array.
byteArray ByteArrayOutputStream::toByteArray()
{
    byteArray out(count);
    memcpy(out.data, buf.data, count);
    return out;
}
