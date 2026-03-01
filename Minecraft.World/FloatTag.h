#pragma once
#include "InputOutputStream.h"
#include "Tag.h"

class FloatTag : public Tag
{
public:
	float data;
	FloatTag(const wstring &name) : Tag(name) {}
	FloatTag(const wstring &name, float data) : Tag(name) {this->data = data; }
	
	void write(DataOutput *dos) { dos->writeFloat(data); }
	void load(DataInput *dis, int tagDepth) { data = dis->readFloat(); }

	byte getId() { return TAG_Float; }
	wstring toString()
	{
		static wchar_t buf[32];
		swprintf(buf, 32, L"%f",data);
		return wstring( buf );
	}

	Tag *copy()
	{
		return new FloatTag(getName(), data);
	}

	bool equals(Tag *obj)
	{
		if (Tag::equals(obj))
		{
			FloatTag *o = (FloatTag *) obj;
			return data == o->data;
		}
		return false;
	}
};