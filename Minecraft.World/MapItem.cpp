#include "stdafx.h"
#include "net.minecraft.network.packet.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.chunk.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.level.saveddata.h"
#include "net.minecraft.world.level.storage.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.h"
#include "MapItem.h"
#include "net.minecraft.world.inventory.h"
#include "JavaMath.h"

MapItem::MapItem(int id) : ComplexItem(id)
{
	setStackedByData(true);
}

shared_ptr<MapItemSavedData> MapItem::getSavedData(short idNum, Level *level)
{
	std::wstring id = wstring( L"map_" ) + std::to_wstring(idNum);
	shared_ptr<MapItemSavedData> mapItemSavedData = dynamic_pointer_cast<MapItemSavedData>(level->getSavedData(typeid(MapItemSavedData), id));

	if (mapItemSavedData == nullptr)
	{
		// 4J Stu - This call comes from ClientConnection, but i don't see why we should be trying to work out
		// the id again when it's passed as a param. In any case that won't work with the new map setup
		//int aux = level->getFreeAuxValueFor(L"map");
		int aux = idNum;

		id = wstring( L"map_" ) + std::to_wstring(aux);
		mapItemSavedData = std::make_shared<MapItemSavedData>(id);

		level->setSavedData(id, (shared_ptr<SavedData> ) mapItemSavedData);
	}

	return mapItemSavedData;
}

shared_ptr<MapItemSavedData> MapItem::getSavedData(shared_ptr<ItemInstance> itemInstance, Level *level)
{
	MemSect(31);
	std::wstring id = wstring( L"map_" ) + std::to_wstring(itemInstance->getAuxValue() );
	MemSect(0);
	shared_ptr<MapItemSavedData> mapItemSavedData = dynamic_pointer_cast<MapItemSavedData>( level->getSavedData(typeid(MapItemSavedData), id ) );

	bool newData = false;
	if (mapItemSavedData == nullptr)
	{
		// 4J Stu - I don't see why we should be trying to work out the id again when it's passed as a param.
		// In any case that won't work with the new map setup
		//itemInstance->setAuxValue(level->getFreeAuxValueFor(L"map"));

		id = wstring( L"map_" ) + std::to_wstring(itemInstance->getAuxValue() );
		mapItemSavedData = std::make_shared<MapItemSavedData>(id);

		newData = true;
	}

	mapItemSavedData->scale = 3;
#ifndef _LARGE_WORLDS
	// 4J-PB - for Xbox maps, we'll centre them on the origin of the world, since we can fit the whole world in our map
	mapItemSavedData->x = 0;
	mapItemSavedData->z = 0;
#endif

	if( newData )
	{
#ifdef _LARGE_WORLDS
		int scale = MapItemSavedData::MAP_SIZE * 2 * (1 << mapItemSavedData->scale);
		mapItemSavedData->x = Math::round(static_cast<float>(level->getLevelData()->getXSpawn()) / scale) * scale;
		mapItemSavedData->z = Math::round(level->getLevelData()->getZSpawn() / scale) * scale;
#endif
		mapItemSavedData->dimension = static_cast<byte>(level->dimension->id);

		mapItemSavedData->setDirty();

		level->setSavedData(id, (shared_ptr<SavedData> ) mapItemSavedData);
	}

	return mapItemSavedData;
}

void MapItem::update(Level *level, shared_ptr<Entity> player, shared_ptr<MapItemSavedData> data)
{
	if ( (level->dimension->id != data->dimension) || !player->instanceof(eTYPE_PLAYER) )
	{
		// Wrong dimension, abort
		return;
	}

	int w = MapItem::IMAGE_WIDTH;
	int h = MapItem::IMAGE_HEIGHT;

	int scale = 1 << data->scale;

	int xo = data->x;
	int zo = data->z;

	int xp = Mth::floor(player->x - xo) / scale + w / 2;
	int zp = Mth::floor(player->z - zo) / scale + h / 2;

	int rad = 128 / scale;
	if (level->dimension->hasCeiling)
	{
		rad /= 2;
	}
	shared_ptr<MapItemSavedData::HoldingPlayer> hp = data->getHoldingPlayer(dynamic_pointer_cast<Player>(player));
	hp->step++;

	for (int x = xp - rad + 1; x < xp + rad; x++)
	{
		if ((x & 15) != (hp->step & 15)) continue;

		int yd0 = 255;
		int yd1 = 0;

		double ho = 0;
		for (int z = zp - rad - 1; z < zp + rad; z++)
		{
			if (x < 0 || z < -1 || x >= w || z >= h) continue;

			int xd = x - xp;
			int zd = z - zp;

			bool ditherBlack = xd * xd + zd * zd > (rad - 2) * (rad - 2);

			int xx = (xo / scale + x - w / 2) * scale;
			int zz = (zo / scale + z - h / 2) * scale;

			int count[256];
			memset( count,0,sizeof(int)*256);

			LevelChunk *lc = level->getChunkAt(xx, zz);
			if(lc->isEmpty()) continue;
			int xso = ((xx)) & 15;
			int zso = ((zz)) & 15;
			int liquidDepth = 0;

			double hh = 0;
			if (level->dimension->hasCeiling)
			{
				int ss = xx + zz * 231871;
				ss = ss * ss * 31287121 + ss * 11;
				if (((ss >> 20) & 1) == 0) count[Tile::dirt_Id] += 10;
				else count[Tile::stone_Id] += 10;
				hh = 100;
			}
			else
			{
				for (int xs = 0; xs < scale; xs++)
				{
					for (int zs = 0; zs < scale; zs++)
					{
						int yy = lc->getHeightmap(xs + xso, zs + zso) + 1;
						int t = 0;
						if (yy > 1)
						{
							bool ok = false;
							do
							{
								ok = true;
								t = lc->getTile(xs + xso, yy - 1, zs + zso);
								if (t == 0) ok = false;
								else if (yy > 0 && t > 0 && Tile::tiles[t]->material->color == MaterialColor::none)
								{
									ok = false;
								}

								if (!ok)
								{
									yy--;
									if (yy <= 0) break;
									t = lc->getTile(xs + xso, yy - 1, zs + zso);
								}

							} while (yy > 0 && !ok);

							if (yy > 0 && t != 0 && Tile::tiles[t]->material->isLiquid())
							{
								int y = yy - 1;
								int below = 0;
								do
								{
									below = lc->getTile(xs + xso, y--, zs + zso);
									liquidDepth++;
								} while (y > 0 && below != 0 && Tile::tiles[below]->material->isLiquid());
							}
						}
						hh += yy / static_cast<double>(scale * scale);

						count[t]++;
					}
				}
			}
			liquidDepth /= scale * scale;

			int best = 0;
			int tBest = 0;
			for (int j = 0; j < 256; j++)
			{
				if (count[j] > best)
				{
					tBest = j;
					best = count[j];
				}
			}

			double diff = ((hh - ho) * 4 / (scale + 4)) + (((x + z) & 1) - 0.5) * 0.4;
			int br = 1;
			if (diff > +0.6) br = 2;
			if (diff < -0.6) br = 0;

			int col = 0;
			if (tBest > 0)
			{
				MaterialColor *mc = Tile::tiles[tBest]->material->color;
				if (mc == MaterialColor::water)
				{
					diff = (liquidDepth * 0.1) + ((x + z) & 1) * 0.2;
					br = 1;
					if (diff < 0.5) br = 2;
					if (diff > 0.9) br = 0;
				}
				col = mc->id;
			}

			ho = hh;

			if (z < 0) continue;
			if (xd * xd + zd * zd >= rad * rad) continue;
			if (ditherBlack && ((x + z) & 1) == 0)
			{
				continue;
			}
			byte oldColor = data->colors[x + z * w];
			byte newColor = static_cast<byte>(col * 4 + br);
			if (oldColor != newColor)
			{
				if (yd0 > z) yd0 = z;
				if (yd1 < z) yd1 = z;
				data->colors[x + z * w] = newColor;
			}
		}
		if (yd0 <= yd1)
		{
			data->setDirty(x, yd0, yd1);
		}
	}
}

void MapItem::inventoryTick(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Entity> owner, int slot, bool selected)
{
	if (level->isClientSide) return;

	shared_ptr<MapItemSavedData> data = getSavedData(itemInstance, level);
	if ( owner->instanceof(eTYPE_PLAYER) )
	{
		shared_ptr<Player> player = dynamic_pointer_cast<Player>(owner);

		// 4J Stu - If the player has a map that belongs to another player, then merge the data over and change this map id to the owners id
		int ownersAuxValue = level->getAuxValueForMap(player->getXuid(), data->dimension, data->x, data->z, data->scale);
		if(ownersAuxValue != itemInstance->getAuxValue() )
		{
			shared_ptr<MapItemSavedData> ownersData = getSavedData(ownersAuxValue,level);

			ownersData->x = data->x;
			ownersData->z = data->z;
			ownersData->scale = data->scale;
			ownersData->dimension = data->dimension;

			itemInstance->setAuxValue( ownersAuxValue );
			ownersData->tickCarriedBy(player, itemInstance );
			ownersData->mergeInMapData(data);
			player->inventoryMenu->broadcastChanges();

			data = ownersData;
		}
		else
		{
			data->tickCarriedBy(player, itemInstance);
		}
	}

	if (selected)
	{
		update(level, owner, data);
	}
}

shared_ptr<Packet> MapItem::getUpdatePacket(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	charArray data = MapItem::getSavedData(itemInstance, level)->getUpdatePacket(itemInstance, level, player);

	if (data.data == nullptr || data.length == 0) return nullptr;

	shared_ptr<Packet> retval = std::make_shared<ComplexItemDataPacket>(static_cast<short>(Item::map->id), static_cast<short>(itemInstance->getAuxValue()), data);
	delete data.data;
	return retval;
}

void MapItem::onCraftedBy(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	wchar_t buf[64];

	int mapScale = 3;
#ifdef _LARGE_WORLDS
	int scale = MapItemSavedData::MAP_SIZE * 2 * (1 << mapScale);
	int centreXC = static_cast<int>(Math::round(player->x / scale) * scale);
	int centreZC = static_cast<int>(Math::round(player->z / scale) * scale);
#else
	// 4J-PB - for Xbox maps, we'll centre them on the origin of the world, since we can fit the whole world in our map
	int centreXC = 0;
	int centreZC = 0;
#endif

	itemInstance->setAuxValue(level->getAuxValueForMap(player->getXuid(), player->dimension, centreXC, centreZC, mapScale));

	swprintf(buf,64,L"map_%d", itemInstance->getAuxValue());
	std::wstring id = wstring(buf);

	shared_ptr<MapItemSavedData> data = getSavedData(itemInstance->getAuxValue(), level);
	// 4J Stu - We only have one map per player per dimension, so don't reset the one that they have
	// when a new one is created
	if( data == nullptr )
	{
		data = std::make_shared<MapItemSavedData>(id);
	}
	level->setSavedData(id, (shared_ptr<SavedData> ) data);

	data->scale = mapScale;
	// 4J-PB - for Xbox maps, we'll centre them on the origin of the world, since we can fit the whole world in our map
	data->x = centreXC;
	data->z = centreZC;
	data->dimension = static_cast<byte>(level->dimension->id);
	data->setDirty();
}

// 4J - Don't want
/*
void appendHoverText(ItemInstance itemInstance, Player player, List<String> lines, boolean advanced) {
	MapItemSavedData data = getSavedData(itemInstance, player.level);

	if (advanced) {
		if (data == null) {
			lines.add("Unknown map");
		} else {
			lines.add("Scaling at 1:" + (1 << data.scale));
			lines.add("(Level " + data.scale + "/" + MapItemSavedData.MAX_SCALE + ")");
		}
	}
}
*/
