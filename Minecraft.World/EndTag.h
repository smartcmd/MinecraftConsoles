#pragma once
#include "Tag.h"

class EndTag : public Tag
{
public:
	EndTag() : Tag(L"") {}
	EndTag(const wstring &name) : Tag(name) {}

	void load(DataInput *dis, int tagDepth) {};
	void write(DataOutput *dos) {};

	byte getId() { return TAG_End; }
	wstring toString() { return wstring( L"END" ); }

	Tag *copy()
	{
		return new EndTag();
	}

	bool equals(Tag *obj)
	{
		return Tag::equals(obj);
	}
};