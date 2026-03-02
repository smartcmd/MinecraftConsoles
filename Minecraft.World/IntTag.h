#pragma once
#include "Tag.h"

class IntTag : public Tag
{
public:
	int data;
	IntTag(const wstring &name) : Tag(name) {}
	IntTag(const wstring &name, int data) : Tag(name) {this->data = data; }
	
	void write(DataOutput *dos) { dos->writeInt(data); }
	void load(DataInput *dis, int tagDepth) { data = dis->readInt(); }

	byte getId() { return TAG_Int; }
	wstring toString()
	{
		static wchar_t buf[32];
		swprintf(buf, 32, L"%d", data);
		return wstring( buf );
	}

	Tag *copy()
	{
		return new IntTag(getName(), data);
	}

	bool equals(Tag *obj)
	{
		if (Tag::equals(obj))
		{
			IntTag *o = (IntTag *) obj;
			return data == o->data;
		}
		return false;
	}
};