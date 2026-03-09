#pragma once

#include "Tag.h"
#include "System.h"

class IntArrayTag : public Tag
{
public:
	intArray data;

	IntArrayTag(const wstring &name) : Tag(name)
	{
		data = intArray();
	}

	IntArrayTag(const wstring &name, intArray data) : Tag(name)
	{
		this->data = data;
	}

	~IntArrayTag()
	{
		delete [] data.data;
	}

	void write(DataOutput *dos)
	{
		dos->writeInt(data.length);
		for (unsigned int i = 0; i < data.length; i++)
		{
			dos->writeInt(data[i]);
		}
	}

	void load(DataInput *dis, int tagDepth)
	{
		int length = dis->readInt();

		if ( data.data ) delete[] data.data;
		data = intArray(length);
		for (int i = 0; i < length; i++)
		{
			data[i] = dis->readInt();
		}
	}

	byte getId() { return TAG_Int_Array; }

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
			IntArrayTag *o = static_cast<IntArrayTag *>(obj);
			return ((data.data == nullptr && o->data.data == nullptr) || (data.data != nullptr && data.length == o->data.length && memcmp(data.data, o->data.data, data.length * sizeof(int)) == 0) );
		}
		return false;
	}

	Tag *copy()
	{
		intArray cp = intArray(data.length);
		System::arraycopy(data, 0, &cp, 0, data.length);
		return new IntArrayTag(getName(), cp);
	}
};