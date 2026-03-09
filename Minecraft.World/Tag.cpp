#include "stdafx.h"
#include "Tag.h"
#include "EndTag.h"
#include "ByteTag.h"
#include "ByteArrayTag.h"
#include "DoubleTag.h"
#include "FloatTag.h"
#include "IntTag.h"
#include "LongTag.h"
#include "ShortTag.h"
#include "StringTag.h"
#include "ListTag.h"
#include "CompoundTag.h"

Tag::Tag(const wstring &name)
{
	if (name.empty())
	{
		this->name = L"";
	}
	else
	{
		this->name = name;
	}
}

// 4J - Was Object obj
bool Tag::equals(Tag *obj)
{
	if (obj == nullptr )// || !(obj instanceof Tag))
	{
		return false;
	}
	Tag *o = (Tag *) obj;
	if (getId() != o->getId())
	{
		return false;
	}
	if ( (name.empty() && !o->name.empty()) || (!name.empty() && o->name.empty()))
	{
		return false;
	}
	if (!name.empty() && name.compare(o->name) != 0)
	{
		return false;
	}
	return true;
}

void Tag::print(ostream out)
{
	out << "";
}

void Tag::print(char *prefix, wostream out)
{
	wstring name = getName();

	out << prefix;
	out << getTagName(getId());
	if ( name.length() > 0)
	{
		out << L"(\"" << name << L"\")";
	}
	out << L": ";
	out << toString() << endl;
}

wstring Tag::getName()
{
	return name;
}

Tag *Tag::setName(const wstring& name)
{
	this->name = name;
	return this;
}

Tag *Tag::readNamedTag(DataInput *dis)
{
	return readNamedTag(dis,0);
}

Tag *Tag::readNamedTag(DataInput *dis, int tagDepth)
{
	byte type = dis->readByte();
	if (type == 0) return new EndTag();

	// 4J Stu - readByte can return -1, so if it's that then also mark as the end tag
	if(type == 255)
	{
		app.DebugPrintf("readNamedTag read a type of 255\n");
#ifndef _CONTENT_PACKAGE
		__debugbreak();
#endif
		return new EndTag();
	}

	wstring name = dis->readUTF();//new String(bytes, "UTF-8");

	Tag *tag = newTag(type, name);
	//        short length = dis.readShort();
	//        byte[] bytes = new byte[length];
	//        dis.readFully(bytes);

	tag->load(dis, tagDepth);
	return tag;
}

void Tag::writeNamedTag(Tag *tag, DataOutput *dos)
{
	dos->writeByte(tag->getId());
	if (tag->getId() == Tag::TAG_End) return;

	//        byte[] bytes = tag.getName().getBytes("UTF-8");
	//        dos.writeShort(bytes.length);
	//        dos.write(bytes);
	dos->writeUTF(tag->getName());

	tag->write(dos);
}

Tag *Tag::newTag(byte type, const wstring &name)
{
	switch (type)
	{
	case TAG_End:
		return new EndTag(name);
	case TAG_Byte:
		return new ByteTag(name);
	case TAG_Short:
		return new ShortTag(name);
	case TAG_Int:
		return new IntTag(name);
	case TAG_Long:
		return new LongTag(name);
	case TAG_Float:
		return new FloatTag(name);
	case TAG_Double:
		return new DoubleTag(name);
	case TAG_Byte_Array:
		return new ByteArrayTag(name);
	case TAG_Int_Array:
		return new IntArrayTag(name);
	case TAG_String:
		return new StringTag(name);
	case TAG_List:
		return new ListTag<Tag>(name);
	case TAG_Compound:
		return new CompoundTag(name);
	}
	return nullptr;
}

const wchar_t *Tag::getTagName(byte type)
{
	switch (type)
	{
	case TAG_End:
		return L"TAG_End";
	case TAG_Byte:
		return L"TAG_Byte";
	case TAG_Short:
		return L"TAG_Short";
	case TAG_Int:
		return L"TAG_Int";
	case TAG_Long:
		return L"TAG_Long";
	case TAG_Float:
		return L"TAG_Float";
	case TAG_Double:
		return L"TAG_Double";
	case TAG_Byte_Array:
		return L"TAG_Byte_Array";
	case TAG_Int_Array:
		return L"TAG_Int_Array";
	case TAG_String:
		return L"TAG_String";
	case TAG_List:
		return L"TAG_List";
	case TAG_Compound:
		return L"TAG_Compound";
	}
	return L"UNKNOWN";
}