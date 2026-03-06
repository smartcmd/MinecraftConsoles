#pragma once
#include "Tag.h"
#include "ListTag.h"
#include "ByteTag.h"
#include "DoubleTag.h"
#include "FloatTag.h"
#include "IntTag.h"
#include "LongTag.h"
#include "ShortTag.h"
#include "StringTag.h"
#include "ByteArrayTag.h"
#include "IntArrayTag.h"

class CompoundTag : public Tag    
{
private:
	unordered_map<wstring, Tag *> tags;

public:
	CompoundTag() : Tag(L"") {}
	CompoundTag(const wstring &name) : Tag(name) {}

	void write(DataOutput *dos)
	{
		for( auto& tag : tags )
		{
			Tag::writeNamedTag(tag.second, dos);
		}
		dos->writeByte(Tag::TAG_End);
	}


	void load(DataInput *dis, int tagDepth)
	{
		if(tagDepth > MAX_DEPTH)
		{
#ifndef _CONTENT_PACKAGE
			printf("Tried to read NBT tag with too high complexity, depth > %d" , MAX_DEPTH);
			__debugbreak();
#endif
			return;
		}
		tags.clear();
		Tag *tag;
		while ((tag = Tag::readNamedTag(dis))->getId() != Tag::TAG_End)
		{
			tags[tag->getName()] = tag;
		}
		delete tag;
	}

	vector<Tag *> *getAllTags()		// 4J - was collection
	{
		// 4J - was return tags.values();
		vector<Tag *> *ret = new vector<Tag *>;

		for( auto& tag : tags )
		{
			ret->push_back(tag.second);
		}
		return ret;
	}

	byte getId()
	{
		return TAG_Compound;
	}

	void put(const wstring &name, Tag *tag)
	{
		tags[name] = tag->setName(name);
	}

	void putByte(const wstring &name, byte value)
	{
		tags[name] = (new ByteTag(name,value));
	}

	void putShort(const wstring &name, short value)
	{
		tags[name] = (new ShortTag(name,value));
	}

	void putInt(const wstring &name, int value)
	{
		tags[name] = (new IntTag(name,value));
	}

	void putLong(const wstring &name, __int64 value)
	{
		tags[name] = (new LongTag(name,value));
	}

	void putFloat(const wstring &name, float value)
	{
		tags[name] = (new FloatTag(name,value));
	}

	void putDouble(const wstring &name, double value)
	{
		tags[name] = (new DoubleTag(name,value));
	}

	void putString(const wstring &name, const wstring& value)
	{
		tags[name] = (new StringTag(name,value));
	}

	void putByteArray(const wstring &name, byteArray value)
	{
		tags[name] = (new ByteArrayTag(name,value));
	}

	void putIntArray(const wstring &name, intArray value)
	{
		tags[name] = (new IntArrayTag(name, value));
	}

	void putCompound(const wstring &name, CompoundTag *value)
	{
		tags[name] = value->setName( wstring( name ) );
	}

	void putBoolean(const wstring &name, bool val)
	{
		putByte(name, val?(byte)1:0);
	}

	Tag *get(const wstring &name)
	{
		auto it = tags.find(name);
		if(it != tags.end()) return it->second;
		return NULL;
	}

	bool contains(const wstring &name)
	{
		return tags.find(name) != tags.end();
	}

	byte getByte(const wstring &name)
	{
		if (tags.find(name) == tags.end()) return (byte)0;
		return ((ByteTag *) tags[name])->data;
	}

	short getShort(const wstring &name)
	{
		if (tags.find(name) == tags.end()) return (short)0;
		return ((ShortTag *) tags[name])->data;
	}

	int getInt(const wstring &name)
	{
		if (tags.find(name) == tags.end()) return (int)0;
		return ((IntTag *) tags[name])->data;
	}

	__int64 getLong(const wstring &name)
	{
		if (tags.find(name) == tags.end()) return (__int64)0;
		return ((LongTag *) tags[name])->data;
	}

	float getFloat(const wstring &name)
	{
		if (tags.find(name) == tags.end()) return (float)0;
		return ((FloatTag *) tags[name])->data;
	}

	double getDouble(const wstring &name)
	{
		if (tags.find(name) == tags.end()) return (double)0;
		return ((DoubleTag *) tags[name])->data;
	}

	wstring getString(const wstring &name)
	{
		if (tags.find(name) == tags.end()) return wstring( L"" );
		return ((StringTag *) tags[name])->data;
	}

	byteArray getByteArray(const wstring &name)
	{
		if (tags.find(name) == tags.end()) return byteArray();
		return ((ByteArrayTag *) tags[name])->data;
	}

	intArray getIntArray(const wstring &name)
	{
		if (tags.find(name) == tags.end()) return intArray();
		return ((IntArrayTag *) tags[name])->data;
	}

	CompoundTag *getCompound(const wstring &name)
	{
		if (tags.find(name) == tags.end()) return new CompoundTag(name);
		return (CompoundTag *) tags[name];
	}

	ListTag<Tag> *getList(const wstring &name)
	{
		if (tags.find(name) == tags.end()) return new ListTag<Tag>(name);
		return (ListTag<Tag> *) tags[name];
	}

	bool getBoolean(const wstring &string)
	{
		return getByte(string)!=0;
	}

	void remove(const wstring &name)
	{
		auto it = tags.find(name);
		if(it != tags.end()) tags.erase(it);
		//tags.remove(name);
	}

	wstring toString()
	{
		static const int bufSize = 32;
		static wchar_t buf[bufSize];
		swprintf(buf,bufSize,L"%zu entries",tags.size());
		return wstring( buf );
	}

	void print(char *prefix, ostream out)
	{
		/*
		Tag::print(prefix, out);
		out << prefix << "{" << endl;

		char *newPrefix = new char[ strlen(prefix) + 4 ];
		strcpy( newPrefix, prefix);
		strcat( newPrefix, "   ");

		for( auto& it : tags )
		{
			it.second->print(newPrefix, out);
		}
		delete[] newPrefix;
		out << prefix << "}" << endl;
		*/
	}

	bool isEmpty()
	{
		return tags.empty();
	}

	virtual ~CompoundTag()
	{
		for( auto& tag : tags )
		{
			delete tag.second;
		}
	}

	Tag *copy()
	{
		CompoundTag *tag = new CompoundTag(getName());

		for( auto& it : tags )
		{			
			tag->put((wchar_t *)it.first.c_str(), it.second->copy());
		}
		return tag;
	}

	bool equals(Tag *obj)
	{
		if (Tag::equals(obj))
		{
			CompoundTag *o = (CompoundTag *) obj;

			if(tags.size() == o->tags.size())
			{
				bool equal = true;
				for( auto& it : tags )
				{
					auto itFind = o->tags.find(it.first);
					if(itFind == o->tags.end() || !it.second->equals(itFind->second) )
					{
						equal = false;
						break;
					}
				}
				return equal;
				//return tags.entrySet().equals(o.tags.entrySet());
			}
		}
		return false;
	}
};
