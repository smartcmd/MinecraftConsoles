#pragma once
#include "Tag.h"

class LongTag : public Tag
{
public:
	__int64 data;
	LongTag(const wstring &name) : Tag(name) {}
	LongTag(const wstring &name, __int64 data) : Tag(name) {this->data = data; }
	
	void write(DataOutput *dos) { dos->writeLong(data); }
	void load(DataInput *dis) { data = dis->readLong(); }

	byte getId() { return TAG_Long; }
	wstring toString()
	{
		static wchar_t buf[32];
		swprintf(buf,32,L"%I64d",data);
		return wstring(buf);
	}

	Tag *copy()
	{
		return new LongTag(getName(), data);
	}

	bool equals(Tag *obj)
	{
		if (Tag::equals(obj))
		{
			LongTag *o = (LongTag *) obj;
			return data == o->data;
		}
		return false;
	}
};