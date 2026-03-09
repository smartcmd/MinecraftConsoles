#pragma once
#include "InputOutputStream.h"
#include "Tag.h"

class DoubleTag : public Tag
{
public:
	double data;
	DoubleTag(const wstring &name) : Tag(name) {}
	DoubleTag(const wstring &name, double data) : Tag(name) {this->data = data; }
	
	void write(DataOutput *dos) { dos->writeDouble(data); }
	void load(DataInput *dis, int tagDepth) { data = dis->readDouble(); }

	byte getId() { return TAG_Double; }
	wstring toString()
	{
		static wchar_t buf[32];
		swprintf(buf,32,L"%f",data);
		return wstring( buf );
	}

	Tag *copy()
	{
		return new DoubleTag(getName(), data);
	}

	bool equals(Tag *obj)
	{
		if (Tag::equals(obj))
		{
			DoubleTag *o = static_cast<DoubleTag *>(obj);
			return data == o->data;
		}
		return false;
	}
};