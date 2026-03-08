#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "MapItemSavedData.h"
#include "../Minecraft.Client/PlayerList.h"
#include "../Minecraft.Client/MinecraftServer.h"
#include "../Minecraft.Client/ServerPlayer.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.storage.h"
#include "../Minecraft.Client/PlayerConnection.h"

const int MapItemSavedData::END_PORTAL_DECORATION_KEY = -1;

// 4J added entityId param
MapItemSavedData::MapDecoration::MapDecoration(char img, char x, char y, char rot, int entityId, bool visible)
{
	this->img = img;
	this->x = x;
	this->y = y;
	this->rot = rot;
	this->entityId = entityId;
	this->visible = visible;
}

MapItemSavedData::HoldingPlayer::HoldingPlayer(shared_ptr<Player> player, const MapItemSavedData *parent) : parent( parent ), player( player )
{
	// inited outside of ctor
	rowsDirtyMin = intArray(MapItem::IMAGE_WIDTH);
	rowsDirtyMax = intArray(MapItem::IMAGE_WIDTH);

	tick = 0;
	sendPosTick = 0;
	step = 0;
	hasSentInitial = false;

	// java ctor
	//this->player = player;
	for (unsigned int i = 0; i < rowsDirtyMin.length; i++)
	{
		rowsDirtyMin[i] = 0;
		rowsDirtyMax[i] = MapItem::IMAGE_HEIGHT - 1;
	}
}

MapItemSavedData::HoldingPlayer::~HoldingPlayer()
{
	delete rowsDirtyMin.data;
	delete rowsDirtyMax.data;
	delete lastSentDecorations.data;
}

charArray MapItemSavedData::HoldingPlayer::nextUpdatePacket(shared_ptr<ItemInstance> itemInstance)
{
	if (!hasSentInitial)
	{
		charArray data(2);
		data[0] = HEADER_METADATA;
		data[1] = parent->scale;

		hasSentInitial = true;
		return data;
	}
	if (--sendPosTick < 0)
	{
		sendPosTick = 4;

		unsigned int playerDecorationsSize = (int)parent->decorations.size();
		unsigned int nonPlayerDecorationsSize = (int)parent->nonPlayerDecorations.size();
		charArray data = charArray( (playerDecorationsSize + nonPlayerDecorationsSize ) * DEC_PACKET_BYTES + 1);
		data[0] = 1;
		for (unsigned int i = 0; i < parent->decorations.size(); i++)
		{
			MapDecoration *md = parent->decorations.at(i);
#ifdef _LARGE_WORLDS
			data[i * DEC_PACKET_BYTES + 1] = (char) (md->img);
			data[i * DEC_PACKET_BYTES + 8] = (char) (md->rot & 0xF);
#else
			data[i * DEC_PACKET_BYTES + 1] = (char) ((md->img << 4) | (md->rot & 0xF));
#endif
			data[i * DEC_PACKET_BYTES + 2] = md->x;
			data[i * DEC_PACKET_BYTES + 3] = md->y;
			data[i * DEC_PACKET_BYTES + 4] = md->entityId & 0xFF;
			data[i * DEC_PACKET_BYTES + 5] = (md->entityId>>8) & 0xFF;
			data[i * DEC_PACKET_BYTES + 6] = (md->entityId>>16) & 0xFF;
			data[i * DEC_PACKET_BYTES + 7] = (md->entityId>>24) & 0x7F;
			data[i * DEC_PACKET_BYTES + 7] |= md->visible ? 0x80 : 0x0;
		}
		unsigned int dataIndex = playerDecorationsSize;
		for(auto it : parent->nonPlayerDecorations)
		{
			MapDecoration *md = it.second;
#ifdef _LARGE_WORLDS
			data[dataIndex * DEC_PACKET_BYTES + 1] = (char) (md->img);
			data[dataIndex * DEC_PACKET_BYTES + 8] = (char) (md->rot & 0xF);
#else
			data[dataIndex * DEC_PACKET_BYTES + 1] = (char) ((md->img << 4) | (md->rot & 0xF));
#endif
			data[dataIndex * DEC_PACKET_BYTES + 2] = md->x;
			data[dataIndex * DEC_PACKET_BYTES + 3] = md->y;
			data[dataIndex * DEC_PACKET_BYTES + 4] = md->entityId & 0xFF;
			data[dataIndex * DEC_PACKET_BYTES + 5] = (md->entityId>>8) & 0xFF;
			data[dataIndex * DEC_PACKET_BYTES + 6] = (md->entityId>>16) & 0xFF;
			data[dataIndex * DEC_PACKET_BYTES + 7] = (md->entityId>>24) & 0x7F;
			data[dataIndex * DEC_PACKET_BYTES + 7] |= md->visible ? 0x80 : 0x0;

			++dataIndex;
		}
		bool thesame = !itemInstance->isFramed();
		if (lastSentDecorations.data == NULL || lastSentDecorations.length != data.length)
		{
			thesame = false;
		}
		else
		{
			for (unsigned int i = 0; i < data.length; i++)
			{
				if ( data[i] != lastSentDecorations[i])
				{
					thesame = false;
					break;
				}
			}
		}

		if (!thesame)
		{
			if( lastSentDecorations.data != NULL )
			{
				delete[] lastSentDecorations.data;
			}
			// Make a copy of data, as the calling function presumes it can destroy the returned data
			lastSentDecorations = charArray(data.length);
			memcpy(lastSentDecorations.data, data.data, data.length);
			return data;
		}
		delete data.data;
	}
	shared_ptr<ServerPlayer> servPlayer = dynamic_pointer_cast<ServerPlayer>(player);
	for (int d = 0; d < 10; d++)
	{
		int column = (tick++ * 11) % (MapItem::IMAGE_WIDTH);

		if (rowsDirtyMin[column] >= 0)
		{
			int len = rowsDirtyMax[column] - rowsDirtyMin[column] + 1;
			int min = rowsDirtyMin[column];

			charArray data = charArray(len + 3);
			data[0] = HEADER_COLOURS;
			data[1] = (char) column;
			data[2] = (char) min;
			for (unsigned int y = 0; y < data.length - 3; y++)
			{
				data[y + 3] = parent->colors[(y + min) * MapItem::IMAGE_WIDTH + column];
			}
			rowsDirtyMax[column] = -1;
			rowsDirtyMin[column] = -1;
			return data;
		}
	}
	return charArray();
}


MapItemSavedData::MapItemSavedData(const wstring& id) : SavedData( id )
{
	x = z = 0;
	dimension = 0;
	scale = 0;
	colors = byteArray( MapItem::IMAGE_WIDTH * MapItem::IMAGE_HEIGHT );
}

MapItemSavedData::~MapItemSavedData()
{
	delete colors.data;
	for( unsigned int i = 0; i < decorations.size(); i++ )
	{
		delete decorations[i];
	}
}

void MapItemSavedData::load(CompoundTag *tag)
{
	dimension = tag->getByte(L"dimension");
	x = tag->getInt(L"xCenter");
	z = tag->getInt(L"zCenter");
	scale = tag->getByte(L"scale");
	if (scale < 0) scale = 0;
	if (scale > MAX_SCALE) scale = MAX_SCALE;

	int width = tag->getShort(L"width");
	int height = tag->getShort(L"height");
	if (width == MapItem::IMAGE_WIDTH && height == MapItem::IMAGE_HEIGHT)
	{
		colors = tag->getByteArray(L"colors");
	}
	else
	{
		byteArray newColors = tag->getByteArray(L"colors");
		//4J
		if(colors.data != NULL)
		{
			delete[] colors.data;
		}
		//End4J
		colors = byteArray(MapItem::IMAGE_WIDTH * MapItem::IMAGE_HEIGHT);
		int xo = (MapItem::IMAGE_WIDTH - width) / 2;
		int yo = (MapItem::IMAGE_HEIGHT - height) / 2;
		for (int y = 0; y < height; y++)
		{
			int yt = y + yo;
			if (yt < 0 && yt >= MapItem::IMAGE_HEIGHT) continue;
			for (int x = 0; x < width; x++)
			{
				int xt = x + xo;
				if (xt < 0 && xt >= MapItem::IMAGE_WIDTH) continue;
				colors[xt + yt * MapItem::IMAGE_WIDTH] = newColors[x + y * width];
			}
		}
	}
}

void MapItemSavedData::save(CompoundTag *tag)
{
	tag->putByte(L"dimension", dimension);
	tag->putInt(L"xCenter", x);
	tag->putInt(L"zCenter", z);
	tag->putByte(L"scale", scale);
	tag->putShort(L"width", (short) MapItem::IMAGE_WIDTH);
	tag->putShort(L"height", (short) MapItem::IMAGE_HEIGHT);
	tag->putByteArray(L"colors", colors);
}

void MapItemSavedData::tickCarriedBy(shared_ptr<Player> player, shared_ptr<ItemInstance> item)
{
	if (carriedByPlayers.find(player) == carriedByPlayers.end())
	{
		shared_ptr<HoldingPlayer> hp = shared_ptr<HoldingPlayer>( new HoldingPlayer(player, this ) );
		carriedByPlayers.insert( playerHoldingPlayerMapType::value_type(player, hp) );
		carriedBy.push_back(hp);
	}

	for( unsigned int i = 0; i < decorations.size(); i++ )
	{
		delete decorations[i];
	}
	decorations.clear();

	// 4J Stu - Put this block back in if you want to display entity positions on a map (see below)
#if 0
	nonPlayerDecorations.clear();
#endif
	bool addedPlayers = false;
    for (auto it = carriedBy.begin(); it != carriedBy.end();)
    {
		shared_ptr<HoldingPlayer> hp = *it;

		// 4J Stu - Players in the same dimension as an item frame with a map need to be sent this data, so don't remove them
		if (hp->player->removed ) //|| (!hp->player->inventory->contains(item) && !item->isFramed() ))
		{
            auto it2 = carriedByPlayers.find(shared_ptr<Player>(hp->player));
            if( it2 != carriedByPlayers.end() )
			{
				carriedByPlayers.erase( it2 );
			}
			it = carriedBy.erase( std::find(carriedBy.begin(), carriedBy.end(), hp) );
		}
		else
		{
			++it;

			Level *playerLevel = hp->player->level;
			if(!playerLevel->isClientSide && hp->player->dimension == 0 && (playerLevel->getLevelData()->getHasStrongholdEndPortal() || playerLevel->getLevelData()->getHasStronghold() ) )
			{
				bool atLeastOnePlayerInTheEnd = false;
				PlayerList *players = MinecraftServer::getInstance()->getPlayerList();
				for( const auto& serverPlayer : players->players)
				{
					if(serverPlayer->dimension == 1)
					{
						atLeastOnePlayerInTheEnd = true;
						break;
					}
				}

                auto currentPortalDecoration = nonPlayerDecorations.find(END_PORTAL_DECORATION_KEY);
                if( currentPortalDecoration == nonPlayerDecorations.end() && atLeastOnePlayerInTheEnd)
				{
					float origX = 0.0f;
					float origZ = 0.0f;

					if(playerLevel->getLevelData()->getHasStrongholdEndPortal())
					{
						origX = playerLevel->getLevelData()->getXStrongholdEndPortal();
						origZ = playerLevel->getLevelData()->getZStrongholdEndPortal();
					}
					else
					{
						origX = playerLevel->getLevelData()->getXStronghold()<<4;
						origZ = playerLevel->getLevelData()->getZStronghold()<<4;
					}

					float xd = (float) ( origX - x ) / (1 << scale);
					float yd = (float) ( origZ - z ) / (1 << scale);
					char x = (char) (xd * 2 + 0.5);
					char y = (char) (yd * 2 + 0.5);
					int size = MAP_SIZE - 1;
#ifdef _LARGE_WORLDS
					if (xd < -size || yd < -size || xd > size || yd > size)
					{

						if (xd <= -size) x = (byte) (size * 2 + 2.5);
						if (yd <= -size) y = (byte) (size * 2 + 2.5);
						if (xd >= size) x = (byte) (size * 2 + 1);
						if (yd >= size) y = (byte) (size * 2 + 1);
					}
#endif
					//decorations.push_back(new MapDecoration(4, x, y, 0));
					nonPlayerDecorations.insert( unordered_map<int, MapDecoration *>::value_type( END_PORTAL_DECORATION_KEY, new MapDecoration(4, x, y, 0, END_PORTAL_DECORATION_KEY, true) ) );
				}
				else if ( currentPortalDecoration != nonPlayerDecorations.end() && !atLeastOnePlayerInTheEnd )
				{
					delete currentPortalDecoration->second;
					nonPlayerDecorations.erase( currentPortalDecoration );
				}
			}

			if (item->isFramed())
			{
				//addDecoration(1, player.level, "frame-" + item.getFrame().entityId, item.getFrame().xTile, item.getFrame().zTile, item.getFrame().dir * 90);

				if( nonPlayerDecorations.find( item->getFrame()->entityId ) == nonPlayerDecorations.end() )
				{
					float xd = (float) ( item->getFrame()->xTile - x ) / (1 << scale);
					float yd = (float) ( item->getFrame()->zTile - z ) / (1 << scale);
					char x = (char) (xd * 2 + 0.5);
					char y = (char) (yd * 2 + 0.5);
					int size = MAP_SIZE - 1;
					char rot = (char) ( (item->getFrame()->dir * 90) * 16 / 360);
					if (dimension < 0)
					{
						int s = (int) (playerLevel->getLevelData()->getDayTime() / 10);
						rot = (char) ((s * s * 34187121 + s * 121) >> 15 & 15);
					}
#ifdef _LARGE_WORLDS
					if (xd < -size || yd < -size || xd > size || yd > size)
					{

						if (xd <= -size) x = (byte) (size * 2 + 2.5);
						if (yd <= -size) y = (byte) (size * 2 + 2.5);
						if (xd >= size) x = (byte) (size * 2 + 1);
						if (yd >= size) y = (byte) (size * 2 + 1);
					}
#endif
					//decorations.push_back(new MapDecoration(7, x, y, 0));
					nonPlayerDecorations.insert( unordered_map<int, MapDecoration *>::value_type( item->getFrame()->entityId, new MapDecoration(12, x, y, rot, item->getFrame()->entityId, true) ) );
				}
			}

			// 4J Stu - Put this block back in if you want to display entity positions on a map (see above as well)
#if 0
			for(auto& ent : playerLevel->entities)
			{
				if((ent->GetType() & eTYPE_ENEMY) == 0) continue;

				float xd = (float) ( ent->x - x ) / (1 << scale);
				float yd = (float) ( ent->z - z ) / (1 << scale);
				char x = (char) (xd * 2 + 0.5);
				char y = (char) (yd * 2 + 0.5);
				int size = MAP_SIZE - 1;
				char rot = 0;
				if (dimension < 0)
				{
					int s = step / 10;
					rot = (char) ((s * s * 34187121 + s * 121) >> 15 & 15);
				}
#ifdef _LARGE_WORLDS
				if (xd < -size || yd < -size || xd > size || yd > size)
				{

					if (xd <= -size) x = (byte) (size * 2 + 2.5);
					if (yd <= -size) y = (byte) (size * 2 + 2.5);
					if (xd >= size) x = (byte) (size * 2 + 1);
					if (yd >= size) y = (byte) (size * 2 + 1);
				}
#endif
				//decorations.push_back(new MapDecoration(7, x, y, 0));
				nonPlayerDecorations.insert( unordered_map<int, MapDecoration *>::value_type( ent->entityId, new MapDecoration(4, x, y, rot, ent->entityId, true) ) );
			}
#endif

			// 4J-PB - display all the players in the map
			// For the xbox, x and z are 0
			if (hp->player->dimension == this->dimension && !addedPlayers)
			{
				addedPlayers = true;

				PlayerList *players = MinecraftServer::getInstance()->getPlayerList();
				for(auto& decorationPlayer : players->players)
				{
					if(decorationPlayer!=NULL && decorationPlayer->dimension == this->dimension)
					{
						float xd = (float) (decorationPlayer->x - x) / (1 << scale);
						float yd = (float) (decorationPlayer->z - z) / (1 << scale);
						char x = (char) (xd * 2);
						char y = (char) (yd * 2);
						int size = MAP_SIZE; // - 1;
						char rot;
						char imgIndex;

#ifdef _LARGE_WORLDS
						if (xd > -size && yd > -size && xd <= size && yd <= size)

#endif
						{
							rot = (char) (decorationPlayer->yRot * 16 / 360 + 0.5);
							if (dimension < 0)
							{
								int s = (int) (playerLevel->getLevelData()->getDayTime() / 10);
								rot = (char) ((s * s * 34187121 + s * 121) >> 15 & 15);
							}

							// 4J Stu - As we have added new icons for players on a new row below
							// other icons used in Java we need to move our index to the next row
							imgIndex = (int)decorationPlayer->getPlayerIndex();
							if(imgIndex>3) imgIndex += 4;
						}
#ifdef _LARGE_WORLDS
						else //if (abs(xd) < MAP_SIZE * 5 && abs(yd) < MAP_SIZE * 5)
						{
							// 4J Stu - As we have added new icons for players on a new row below
							// other icons used in Java we need to move our index to the next row
							imgIndex = (int)decorationPlayer->getPlayerIndex();
							if(imgIndex>3) imgIndex += 4;
							imgIndex += 16; // Add 16 to indicate that it's on the next texture

							rot = 0;
							size--; // Added to match the old adjusted size
							if (xd <= -size) x = (byte) (size * 2 + 2.5);
							if (yd <= -size) y = (byte) (size * 2 + 2.5);
							if (xd >= size) x = (byte) (size * 2 + 1);
							if (yd >= size) y = (byte) (size * 2 + 1);
						}
#endif

						MemSect(45);
						decorations.push_back(new MapDecoration(imgIndex, x, y, rot, decorationPlayer->entityId, (decorationPlayer == hp->player || decorationPlayer->canShowOnMaps()) ));
						MemSect(0);
					}
				}
			}

// 			float xd = (float) (hp->player->x - x) / (1 << scale);
// 			float yd = (float) (hp->player->z - z) / (1 << scale);
// 			int ww = 64;
// 			int hh = 64;
// 			if (xd >= -ww && yd >= -hh && xd <= ww && yd <= hh)
// 			{
// 				char img = 0;
// 				char x = (char) (xd * 2 + 0.5);
// 				char y = (char) (yd * 2 + 0.5);
// 				char rot = (char) (player->yRot * 16 / 360 + 0.5);
// 				if (dimension < 0)
// 				{
// 					int s = step / 10;
// 					rot = (char) ((s * s * 34187121 + s * 121) >> 15 & 15);
// 				}
// 				if (hp->player->dimension == this->dimension)
// 				{
// 					decorations.push_back(new MapDecoration(img, x, y, rot));
// 				}
// 			}
		}
	}
}

charArray MapItemSavedData::getUpdatePacket(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
    auto it = carriedByPlayers.find(player);
    if (it == carriedByPlayers.end() ) return charArray();

	shared_ptr<HoldingPlayer> hp = it->second;
	return hp->nextUpdatePacket(itemInstance);
}

void MapItemSavedData::setDirty(int x, int y0, int y1)
{
	SavedData::setDirty();

	for (auto& hp : carriedBy)
	{
		if (hp->rowsDirtyMin[x] < 0 || hp->rowsDirtyMin[x] > y0) hp->rowsDirtyMin[x] = y0;
		if (hp->rowsDirtyMax[x] < 0 || hp->rowsDirtyMax[x] < y1) hp->rowsDirtyMax[x] = y1;
	}
}

void MapItemSavedData::handleComplexItemData(charArray &data)
{
	if (data[0] == HEADER_COLOURS)
	{
		int xx = data[1] & 0xff;
		int yy = data[2] & 0xff;
		for (unsigned int y = 0; y < data.length - 3; y++)
		{
			colors[(y + yy) * MapItem::IMAGE_WIDTH + xx] = data[y + 3];
		}
		setDirty();

	}
	else if (data[0] == HEADER_DECORATIONS)
	{
		for( unsigned int i = 0; i < decorations.size(); i++ )
		{
			delete decorations[i];
		}
		decorations.clear();
		for (unsigned int i = 0; i < (data.length - 1) / DEC_PACKET_BYTES; i++)
		{
#ifdef _LARGE_WORLDS
			char img = data[i * DEC_PACKET_BYTES + 1];
			char rot = data[i * DEC_PACKET_BYTES + 8];
#else
			// 4J-PB - this gives the wrong result
			char img = (char) ( (((int)data[i * DEC_PACKET_BYTES + 1])&0xF0) >> 4);
			char rot = (char) (data[i * DEC_PACKET_BYTES + 1] & 0xF);
#endif
			char x = data[i * DEC_PACKET_BYTES + 2];
			char y = data[i * DEC_PACKET_BYTES + 3];
			int entityId = (((int)data[i * DEC_PACKET_BYTES + 4])&0xFF) | ( (((int)data[i * DEC_PACKET_BYTES + 5])&0xFF)<<8) | ((((int)data[i * DEC_PACKET_BYTES + 6])&0xFF)<<16) | ((((int)data[i * DEC_PACKET_BYTES + 7])&0x7F)<<24);
			bool visible = (data[i * DEC_PACKET_BYTES + 7] & 0x80) != 0;
			decorations.push_back(new MapDecoration(img, x, y, rot, entityId, visible));
		}
	}
	else if (data[0] == HEADER_METADATA)
	{
		scale = data[1];
	}
}

shared_ptr<MapItemSavedData::HoldingPlayer> MapItemSavedData::getHoldingPlayer(shared_ptr<Player> player)
{
	shared_ptr<HoldingPlayer> hp = nullptr;
    auto it = carriedByPlayers.find(player);

    if (it == carriedByPlayers.end())
	{
		hp = shared_ptr<HoldingPlayer>( new HoldingPlayer(player, this) );
		carriedByPlayers[player] = hp;
		carriedBy.push_back(hp);
	}
	else
	{
		hp = it->second;
	}

	return hp;
}

// 4J Added
// We only have one map per player per dimension, so if they pickup someone elses map we merge their map data with ours
// so that we can see everything that they discovered but still only have one map data ourself
void MapItemSavedData::mergeInMapData(shared_ptr<MapItemSavedData> dataToAdd)
{
	int w = MapItem::IMAGE_WIDTH;
	int h = MapItem::IMAGE_HEIGHT;

	for (int x = 0; x < w; x++)
	{
		int yd0 = 255;
		int yd1 = 0;

		for (int z = 0; z < h; z++)
		{
			byte oldColor = colors[x + z * w];
			byte newColor = dataToAdd->colors[x + z * w];
			if (oldColor == 0 && oldColor != newColor)
			{
				if (yd0 > z) yd0 = z;
				if (yd1 < z) yd1 = z;
				colors[x + z * w] = newColor;
			}
		}
		if (yd0 <= yd1)
		{
			setDirty(x, yd0, yd1);
		}
	}
}

void MapItemSavedData::removeItemFrameDecoration(shared_ptr<ItemInstance> item)
{
	if ( !item )
		return;

	std::shared_ptr<ItemFrame> frame = item->getFrame();
	if ( !frame )
		return;

	auto frameDecoration = nonPlayerDecorations.find(frame->entityId);
	if ( frameDecoration != nonPlayerDecorations.end() )
	{
		delete frameDecoration->second;
		nonPlayerDecorations.erase( frameDecoration );
	}
}
