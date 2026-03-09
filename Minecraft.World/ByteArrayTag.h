#pragma once
#include "Tag.h"
#include "System.h"

class ByteArrayTag : public Tag
{
public:
	byteArray data;
	bool m_ownData;
	
	ByteArrayTag(const wstring &name) : Tag(name) { m_ownData = false; }
	ByteArrayTag(const wstring &name, byteArray data, bool ownData = false) : Tag(name) {this->data = data; m_ownData = ownData;}			// 4J - added ownData param
	~ByteArrayTag() { if(m_ownData) delete [] data.data; }
	
	void write(DataOutput *dos)
	{
		dos->writeInt(data.length);
		dos->write(data);
	}

	void load(DataInput *dis, int tagDepth)
	{
		int length = dis->readInt();
		
		if ( data.data ) delete[] data.data;
		data  = byteArray(length);
		dis->readFully(data);
	}

	byte getId() { return TAG_Byte_Array; }

	wstring toString()
	{
		static wchar_t buf[32];
		swprintf(buf, 32, L"[%d bytes]",data.length);
		return wstring( buf );
	}

	bool equals(Tag *obj)
	{
		if (Tag::equals(obj))
		{
			ByteArrayTag *o = static_cast<ByteArrayTag *>(obj);
			return ((data.data == nullptr && o->data.data == nullptr) || (data.data != nullptr && data.length == o->data.length && memcmp(data.data, o->data.data, data.length) == 0) );
		}
		return false;
	}

	Tag *copy()
	{
		byteArray cp = byteArray(data.length);
		System::arraycopy(data, 0, &cp, 0, data.length);
		return new ByteArrayTag(getName(), cp, true);
	}
};