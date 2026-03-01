#pragma once
#include "Tag.h"

class ShortTag : public Tag
{
public:
	short data;
	ShortTag(const wstring &name) : Tag(name) {}
	ShortTag(const wstring &name, int data) : Tag(name) {this->data = data; }
	
	void write(DataOutput *dos) { dos->writeShort(data); }
	void load(DataInput *dis, int tagDepth) { data = dis->readShort(); }

	byte getId() { return TAG_Short; }
	wstring toString()
	{
		static wchar_t buf[32];
		swprintf(buf,32,L"%d",data);
		return wstring( buf );
	}

	Tag *copy()
	{
		return new ShortTag(getName(), data);
	}

	bool equals(Tag *obj)
	{
		if (Tag::equals(obj))
		{
			ShortTag *o = (ShortTag *) obj;
			return data == o->data;
		}
		return false;
	}
};