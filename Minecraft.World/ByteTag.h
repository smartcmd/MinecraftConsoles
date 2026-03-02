#pragma once
#include "Tag.h"

class ByteTag : public Tag
{
public:
	byte data;
	ByteTag(const wstring &name) : Tag(name) {}
	ByteTag(const wstring &name, byte data) : Tag(name) {this->data = data; }
	
	void write(DataOutput *dos) { dos->writeByte(data); }
	void load(DataInput *dis, int tagDepth) { data = dis->readByte(); }

	byte getId() { return TAG_Byte; }
	wstring toString()
	{
		static wchar_t buf[32];
		swprintf(buf,32,L"%d",data);
		return wstring( buf );
	}

	bool equals(Tag *obj)
	{
		if (Tag::equals(obj))
		{
			ByteTag *o = (ByteTag *) obj;
			return data == o->data;
		}
		return false;
	}

	Tag *copy()
	{
		return new ByteTag(getName(), data);
	}
};