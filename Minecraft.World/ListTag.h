#pragma once
using namespace std;
#include "Tag.h"

template <class T> class ListTag : public Tag
{
private:
	vector<Tag *> list;
	byte type;

public:
	ListTag() : Tag(L"") {}
	ListTag(const wstring &name) : Tag(name) {}

	void write(DataOutput *dos)
	{
		if (list.size() > 0) type = (list[0])->getId();
		else type = 1;

		dos->writeByte(type);
		dos->writeInt(static_cast<int>(list.size()));

		for ( auto& it : list )
			it->write(dos);
	}

	void load(DataInput *dis, int tagDepth)
	{
		type = dis->readByte();
		int size = dis->readInt();
        if (size < 0 || size > MAX_DEPTH)
			size = 0;

		list.clear();
		for (int i = 0; i < size; i++)
		{
			Tag *tag = Tag::newTag(type, L"");
            if (tag == nullptr) break;
			tag->load(dis, tagDepth);
			list.push_back(tag);
		}
	}

	byte getId() { return TAG_List; }

	wstring toString()
	{
		static wchar_t buf[64];
		swprintf(buf,64,L"%zu entries of type %ls",list.size(),Tag::getTagName(type));
		return wstring( buf );
	}

	void print(char *prefix, ostream out)
	{
		Tag::print(prefix, out);

		out << prefix << "{" << endl;

		char *newPrefix = new char[ strlen(prefix) + 4 ];
		strcpy( newPrefix, prefix);
		strcat( newPrefix, "   ");
		for ( auto& it : list )
			it->print(newPrefix, out);
		delete[] newPrefix;
		out << prefix << "}" << endl;
	}

	void add(T *tag)
	{
		type = tag->getId();
		// 4J: List tag write/load doesn't preserve tag names so remove them so we can safely do comparisons
		// There are a few ways I could have fixed this but this seems the least invasive, most complete fix
		// (covers other items that also use list tags and require equality checks to work)
		// considering we can't change the write/load functions.
		tag->setName(L"");
		list.push_back(tag);
	}

	T *get(int index)
	{
		return static_cast<T *>(list[index]);
	}

	int size()
	{
		return static_cast<int>(list.size());
	}

	virtual ~ListTag()
	{
		for ( auto& it : list )
		{
			delete it;
		}
	}

	virtual Tag *copy()
	{
		ListTag<T> *res = new ListTag<T>(getName());
		res->type = type;
		for ( auto& it : list )
		{
			T *copy = static_cast<T *>(it->copy());
			res->list.push_back(copy);
		}
		return res;
	}

	virtual bool equals(Tag *obj)
	{
		if (Tag::equals(obj))
		{
			ListTag *o = static_cast<ListTag *>(obj);
			if (type == o->type)
			{
				bool equal = false;
				if(list.size() == o->list.size())
				{
					equal = true;
					// 4J Stu - Pretty inefficient method, but I think we can live with it give how often it will happen, and the small sizes of the data sets
					for ( auto& it : list )
					{
						bool thisMatches = false;
						for( auto it2 : o->list )
						{
							if(it->equals(it2))
							{
								thisMatches = true;
								break;
							}
						}
						if(!thisMatches)
						{
							equal = false;
							break;
						}
					}
				}

				//return list->equals(o->list);
				return equal;
			}
		}
		return false;
	}
};