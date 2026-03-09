#pragma once
#include "Tag.h"

class StringTag : public Tag
{
public:
	wstring data;
	StringTag(const wstring &name) : Tag(name) {}
	StringTag(const wstring &name, const wstring& data) : Tag(name) {this->data = data; }

	void write(DataOutput *dos)
	{
		dos->writeUTF(data);
	}

	void load(DataInput *dis, int tagDepth)
	{
		data = dis->readUTF();
	}

	byte getId() { return TAG_String; }

	wstring toString()
	{
		return data;
	}

	Tag *copy()
	{
		return new StringTag(getName(), data);
	}

	bool equals(Tag *obj)
	{
		if (Tag::equals(obj))
		{
			StringTag *o = static_cast<StringTag *>(obj);
			return ((data.empty() && o->data.empty()) || (!data.empty() && data.compare(o->data) == 0));
		}
		return false;
	}
};
