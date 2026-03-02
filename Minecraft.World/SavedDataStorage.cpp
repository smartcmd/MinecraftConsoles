#include "stdafx.h"
#include "net.minecraft.world.level.saveddata.h"
#include "net.minecraft.world.level.storage.h"
#include "net.minecraft.world.entity.ai.village.h"
#include "com.mojang.nbt.h"
#include "File.h"
#include "SavedDataStorage.h"

#include "ConsoleSaveFileIO.h"

SavedDataStorage::SavedDataStorage(LevelStorage *levelStorage) 
{
	/*
	cache = new unordered_map<wstring, shared_ptr<SavedData> >;
	savedDatas = new vector<shared_ptr<SavedData> >;
	usedAuxIds = new unordered_map<wstring, short*>;
	*/

    this->levelStorage = levelStorage;
    loadAuxValues();
}

shared_ptr<SavedData> SavedDataStorage::get(const type_info& clazz, const wstring& id)
{
	AUTO_VAR(it, cache.find( id ));
	if (it != cache.end()) return (*it).second;

	shared_ptr<SavedData> data = nullptr;
    if (levelStorage != NULL)
	{
		//File file = levelStorage->getDataFile(id);
		ConsoleSavePath file = levelStorage->getDataFile(id);
		if (!file.getName().empty() && levelStorage->getSaveFile()->doesFileExist( file ) ) 
		{
			// mob = dynamic_pointer_cast<Mob>(Mob::_class->newInstance( level ));
		    //data = clazz.getConstructor(String.class).newInstance(id);

			if( clazz == typeid(MapItemSavedData) )
			{
				data = dynamic_pointer_cast<SavedData>( shared_ptr<MapItemSavedData>(new MapItemSavedData(id)) );
			}
			else if( clazz == typeid(Villages) )
			{
				data = dynamic_pointer_cast<SavedData>( shared_ptr<Villages>(new Villages(id) ) );
			}
			else
			{
				// Handling of new SavedData class required
				__debugbreak();
			}

		    ConsoleSaveFileInputStream fis = ConsoleSaveFileInputStream(levelStorage->getSaveFile(), file);
		    CompoundTag *root = NbtIo::readCompressed(&fis);
		    fis.close();

		    data->load(root->getCompound(L"data"));
		}
    }

    if (data != NULL)
	{
        cache.insert( unordered_map<wstring, shared_ptr<SavedData> >::value_type( id , data ) );
        savedDatas.push_back(data);
    }
    return data;
}

void SavedDataStorage::set(const wstring& id, shared_ptr<SavedData> data) 
{
	if (data == NULL)
	{
		// TODO 4J Stu - throw new RuntimeException("Can't set null data");
		assert( false );
	}
	AUTO_VAR(it, cache.find(id));
	if ( it != cache.end() )
	{
		AUTO_VAR(it2, find( savedDatas.begin(), savedDatas.end(), it->second ));
		if( it2 != savedDatas.end() )
		{
			savedDatas.erase( it2 );
		}
		cache.erase( it );
	}
	cache.insert( cacheMapType::value_type(id, data) );
	savedDatas.push_back(data);
}

void SavedDataStorage::save()
{
	AUTO_VAR(itEnd, savedDatas.end());
	for (AUTO_VAR(it, savedDatas.begin()); it != itEnd; it++)
	{
        shared_ptr<SavedData> data = *it; //savedDatas->at(i);
        if (data->isDirty())
		{
            save(data);
            data->setDirty(false);
        }
    }
}

void SavedDataStorage::save(shared_ptr<SavedData> data)
{
    if (levelStorage == NULL) return;
    //File file = levelStorage->getDataFile(data->id);
	ConsoleSavePath file = levelStorage->getDataFile(data->id);
	if (!file.getName().empty())
	{
        CompoundTag *dataTag = new CompoundTag();
        data->save(dataTag);

        CompoundTag *tag = new CompoundTag();
        tag->putCompound(L"data", dataTag);

        ConsoleSaveFileOutputStream fos = ConsoleSaveFileOutputStream(levelStorage->getSaveFile(), file);
        NbtIo::writeCompressed(tag, &fos);
        fos.close();

		delete tag;
    }
}

void SavedDataStorage::loadAuxValues()
{
    usedAuxIds.clear();

    if (levelStorage == NULL) return;
    //File file = levelStorage->getDataFile(L"idcounts");
	ConsoleSavePath file = levelStorage->getDataFile(L"idcounts");
	if (!file.getName().empty() && levelStorage->getSaveFile()->doesFileExist( file ) )
	{
		ConsoleSaveFileInputStream fis = ConsoleSaveFileInputStream(levelStorage->getSaveFile(), file);
        DataInputStream dis = DataInputStream(&fis);
        CompoundTag *tags = NbtIo::read(&dis);
        dis.close();

		Tag *tag;
		vector<Tag *> *allTags = tags->getAllTags();
		AUTO_VAR(itEnd, allTags->end());
		for (AUTO_VAR(it, allTags->begin()); it != itEnd; it++)
		{
			tag = *it; //tags->getAllTags()->at(i);

            if (dynamic_cast<ShortTag *>(tag) != NULL)
			{
                ShortTag *sTag = (ShortTag *) tag;
                wstring id = sTag->getName();
                short val = sTag->data;
                usedAuxIds.insert( uaiMapType::value_type( id, val ) );
            }
        }
		delete allTags;
    }
}

int SavedDataStorage::getFreeAuxValueFor(const wstring& id)
{
	AUTO_VAR(it, usedAuxIds.find( id ));
    short val = 0;
    if ( it != usedAuxIds.end() )
	{
		val = (*it).second;
        val++;
    }

	usedAuxIds[id] = val;
    if (levelStorage == NULL) return val;
    //File file = levelStorage->getDataFile(L"idcounts");
	ConsoleSavePath file = levelStorage->getDataFile(L"idcounts");
    if (!file.getName().empty())
	{
        CompoundTag *tag = new CompoundTag();

		// TODO 4J Stu - This was iterating over the keySet in Java, so potentially we are looking at more items?
		AUTO_VAR(itEndAuxIds, usedAuxIds.end());
		for(uaiMapType::iterator it2 = usedAuxIds.begin(); it2 != itEndAuxIds; it2++)
		{
			short value = it2->second;
			tag->putShort( (wchar_t *) it2->first.c_str(), value);
        }

		ConsoleSaveFileOutputStream fos = ConsoleSaveFileOutputStream(levelStorage->getSaveFile(), file);
        DataOutputStream dos = DataOutputStream(&fos);
        NbtIo::write(tag, &dos);
        dos.close();
    }
    return val;
}

// 4J Added
int SavedDataStorage::getAuxValueForMap(PlayerUID xuid, int dimension, int centreXC, int centreZC, int scale)
{
	if( levelStorage == NULL )
	{
		switch(dimension)
		{
		case -1:
			return MAP_NETHER_DEFAULT_INDEX;
		case 1:
			return MAP_END_DEFAULT_INDEX;
		case 0:
		default:
			return MAP_OVERWORLD_DEFAULT_INDEX;
		}
	}
	else
	{
		return levelStorage->getAuxValueForMap(xuid, dimension, centreXC, centreZC, scale);
	}
}
