#include "stdafx.h"
#include "System.h"
#include "BasicTypeContainers.h"
#include "InputOutputStream.h"
#include "net.minecraft.network.packet.h"
#include "PacketListener.h"
#include "Packet.h"
#include "com.mojang.nbt.h"

#ifndef _CONTENT_PACKAGE
#include "..\Minecraft.Client\Minecraft.h"
#include "..\Minecraft.Client\Gui.h"
#endif

void Packet::staticCtor()
{
	//nextPrint = 0;

	// 4J - Note that item IDs are now defined in virtual method for each packet type

	// 4J Stu - The values for canSendToAnyClient may not necessarily be the correct choices
	map(0, true, true, true, false, typeid(KeepAlivePacket), KeepAlivePacket::create);
	map(1, true, true, true, false, typeid(LoginPacket), LoginPacket::create);
	map(2, true, true, true, false, typeid(PreLoginPacket), PreLoginPacket::create);
	map(3, true, true, true, false, typeid(ChatPacket), ChatPacket::create);
	map(4, true, false, false, true, typeid(SetTimePacket), SetTimePacket::create);
	map(5, true, false, false, true, typeid(SetEquippedItemPacket), SetEquippedItemPacket::create);
	map(6, true, false, true, true, typeid(SetSpawnPositionPacket), SetSpawnPositionPacket::create);
	map(7, false, true, false, false, typeid(InteractPacket), InteractPacket::create);
	map(8, true, false, true, true, typeid(SetHealthPacket), SetHealthPacket::create);
	map(9, true, true, true, false, typeid(RespawnPacket), RespawnPacket::create);

	map(10, true, true, true, false, typeid(MovePlayerPacket), MovePlayerPacket::create);	
	map(11, true, true, true, true, typeid(MovePlayerPacket::Pos), MovePlayerPacket::Pos::create);
	map(12, true, true, true, true, typeid(MovePlayerPacket::Rot), MovePlayerPacket::Rot::create);
	map(13, true, true, true, true, typeid(MovePlayerPacket::PosRot), MovePlayerPacket::PosRot::create);

	map(14, false, true, false, false, typeid(PlayerActionPacket), PlayerActionPacket::create);
	map(15, false, true, false, false, typeid(UseItemPacket), UseItemPacket::create);
	map(16, false, true, false, false, typeid(SetCarriedItemPacket), SetCarriedItemPacket::create);
	// 4J-PB - we need to send to any client for the sleep in bed
	//map(17, true, false, false, false, EntityActionAtPositionPacket));
	map(17, true, false, true, false, typeid(EntityActionAtPositionPacket), EntityActionAtPositionPacket::create);
	// 4J-PB - we need to send to any client for the wake up from sleeping
	//map(18, true, true, false, false, AnimatePacket));
	map(18, true, true, true, false, typeid(AnimatePacket), AnimatePacket::create);
	map(19, false, true, false, false, typeid(PlayerCommandPacket), PlayerCommandPacket::create);

	map(20, true, false, false, true, typeid(AddPlayerPacket), AddPlayerPacket::create);
	map(22, true, false, true, true, typeid(TakeItemEntityPacket), TakeItemEntityPacket::create);
	map(23, true, false, false, true, typeid(AddEntityPacket), AddEntityPacket::create);
	map(24, true, false, false, true, typeid(AddMobPacket), AddMobPacket::create);
	map(25, true, false, false, false, typeid(AddPaintingPacket), AddPaintingPacket::create);
	map(26, true, false, false, false, typeid(AddExperienceOrbPacket), AddExperienceOrbPacket::create); // TODO New for 1.8.2 - Needs sendToAny?
	//map(27, false, true, false, false, PlayerInputPacket));
	// 4J-PB - needs to go to any player, due to the knockback effect when a played is hit
	map(28, true, false, true, true, typeid(SetEntityMotionPacket), SetEntityMotionPacket::create);
	map(29, true, false, false, true, typeid(RemoveEntitiesPacket), RemoveEntitiesPacket::create);

	map(30, true, false, false, false, typeid(MoveEntityPacket), MoveEntityPacket::create);
	map(31, true, false, false, true, typeid(MoveEntityPacket::Pos), MoveEntityPacket::Pos::create);
	map(32, true, false, false, true, typeid(MoveEntityPacket::Rot), MoveEntityPacket::Rot::create);
	map(33, true, false, false, true, typeid(MoveEntityPacket::PosRot), MoveEntityPacket::PosRot::create);
	map(34, true, false, false, true, typeid(TeleportEntityPacket), TeleportEntityPacket::create);
	map(35, true, false, false, false, typeid(RotateHeadPacket), RotateHeadPacket::create);

	// 4J - needs to go to any player, to create sound effect when a player is hit
	map(38, true, false, true, true, typeid(EntityEventPacket), EntityEventPacket::create);
	map(39, true, false, true, false, typeid(SetRidingPacket), SetRidingPacket::create);
	map(40, true, false, true, true, typeid(SetEntityDataPacket), SetEntityDataPacket::create);
	map(41, true, false, true, false, typeid(UpdateMobEffectPacket), UpdateMobEffectPacket::create);
	map(42, true, false, true, false, typeid(RemoveMobEffectPacket), RemoveMobEffectPacket::create);
	map(43, true, false, true, false, typeid(SetExperiencePacket), SetExperiencePacket::create);

	map(50, true, false, true, true, typeid(ChunkVisibilityPacket), ChunkVisibilityPacket::create);
	map(51, true, false, true, true, typeid(BlockRegionUpdatePacket), BlockRegionUpdatePacket::create); // Changed to LevelChunkPacket in Java but we aren't using that
	map(52, true, false, true, true, typeid(ChunkTilesUpdatePacket), ChunkTilesUpdatePacket::create);
	map(53, true, false, true, true, typeid(TileUpdatePacket), TileUpdatePacket::create);
	map(54, true, false, true, true, typeid(TileEventPacket), TileEventPacket::create);
	map(55, true, false, false, false, typeid(TileDestructionPacket), TileDestructionPacket::create);

	map(60, true, false, true, false, typeid(ExplodePacket), ExplodePacket::create);
	map(61, true, false, true, false, typeid(LevelEventPacket), LevelEventPacket::create);
	// 4J-PB - don't see the need for this, we can use 61
	map(62, true, false, true, false, typeid(LevelSoundPacket), LevelSoundPacket::create);
	//map(62, true, false, true, false, typeid(LevelSoundPacket), LevelSoundPacket::create);

	map(70, true, false, false, false, typeid(GameEventPacket), GameEventPacket::create);
	map(71, true, false, false, false, typeid(AddGlobalEntityPacket), AddGlobalEntityPacket::create);

	map(100, true, false, true, false, typeid(ContainerOpenPacket), ContainerOpenPacket::create);
	map(101, true, true, true, false, typeid(ContainerClosePacket), ContainerClosePacket::create);
	map(102, false, true, false, false, typeid(ContainerClickPacket), ContainerClickPacket::create);
#ifndef _CONTENT_PACKAGE
	// 4J Stu - We have some debug code that uses this packet to send data back to the server from the client
	// We may wish to add this into the real game at some point
	map(103, true, true, true, false, typeid(ContainerSetSlotPacket), ContainerSetSlotPacket::create);
#else
	map(103, true, false, true, false, typeid(ContainerSetSlotPacket), ContainerSetSlotPacket::create);
#endif
	map(104, true, false, true, false, typeid(ContainerSetContentPacket), ContainerSetContentPacket::create);
	map(105, true, false, true, false, typeid(ContainerSetDataPacket), ContainerSetDataPacket::create);
	map(106, true, true, true, false, typeid(ContainerAckPacket), ContainerAckPacket::create);
	map(107, true, true, true, false, typeid(SetCreativeModeSlotPacket), SetCreativeModeSlotPacket::create);
	map(108, false, true, false, false, typeid(ContainerButtonClickPacket), ContainerButtonClickPacket::create);

	map(130, true, true, true, false, typeid(SignUpdatePacket), SignUpdatePacket::create);
	map(131, true, false, true, false, typeid(ComplexItemDataPacket), ComplexItemDataPacket::create);
	map(132, true, false, false, false, typeid(TileEntityDataPacket), TileEntityDataPacket::create);

	// 4J Added
	map(150, false, true, false, false, typeid(CraftItemPacket), CraftItemPacket::create);
	map(151, false, true, true, false, typeid(TradeItemPacket), TradeItemPacket::create);
	map(152, false, true, false, false, typeid(DebugOptionsPacket), DebugOptionsPacket::create);
	map(153, true, true, false, false, typeid(ServerSettingsChangedPacket), ServerSettingsChangedPacket::create);
	map(154, true, true, true, false, typeid(TexturePacket), TexturePacket::create);
	map(155, true, false, true, true, typeid(ChunkVisibilityAreaPacket), ChunkVisibilityAreaPacket::create);
	map(156, true, false, false, true, typeid(UpdateProgressPacket), UpdateProgressPacket::create);
	map(157, true, true, true, false, typeid(TextureChangePacket), TextureChangePacket::create);
	map(158, true, false, true, false, typeid(UpdateGameRuleProgressPacket), UpdateGameRuleProgressPacket::create);
	map(159, false, true, false, false, typeid(KickPlayerPacket), KickPlayerPacket::create);
	map(160, true, true, true, false, typeid(TextureAndGeometryPacket), TextureAndGeometryPacket::create);
	map(161, true, true, true, false, typeid(TextureAndGeometryChangePacket), TextureAndGeometryChangePacket::create);

	map(162, true, false, false, false, typeid(MoveEntityPacketSmall), MoveEntityPacketSmall::create);
	map(163, true, false, false, true, typeid(MoveEntityPacketSmall::Pos), MoveEntityPacketSmall::Pos::create);
	map(164, true, false, false, true, typeid( MoveEntityPacketSmall::Rot), MoveEntityPacketSmall::Rot::create);
	map(165, true, false, false, true, typeid(MoveEntityPacketSmall::PosRot), MoveEntityPacketSmall::PosRot::create);
	map(166, true, true, false, false, typeid(XZPacket), XZPacket::create);
	map(167, false, true, false, false, typeid(GameCommandPacket), GameCommandPacket::create);

	map(200, true, false, true, false, typeid(AwardStatPacket), AwardStatPacket::create);
	map(201, true, true, false, false, typeid(PlayerInfoPacket), PlayerInfoPacket::create); // TODO New for 1.8.2 - Repurposed by 4J
	map(202, true, true, true, false, typeid(PlayerAbilitiesPacket), PlayerAbilitiesPacket::create);
	// 4J Stu - These added 1.3.2, but don't think we need them
	//map(203, true, true, true, false, ChatAutoCompletePacket.class);
	//map(204, false, true, true, false, ClientInformationPacket.class);
	map(205, false, true, true, false, typeid(ClientCommandPacket), ClientCommandPacket::create);
	map(250, true, true, true, false, typeid(CustomPayloadPacket), CustomPayloadPacket::create);
	// 4J Stu - These added 1.3.2, but don't think we need them
	//map(252, true, true, SharedKeyPacket.class);
	//map(253, true, false, ServerAuthDataPacket.class);
	map(254, false, true, false, false, typeid(GetInfoPacket), GetInfoPacket::create); // TODO New for 1.8.2 - Needs sendToAny?
	map(255, true, true, true, false, typeid(DisconnectPacket), DisconnectPacket::create);
}

IllegalArgumentException::IllegalArgumentException(const wstring& information)
{
	this->information = information;
}

IOException::IOException(const wstring& information)
{
	this->information = information;
}

Packet::Packet() : createTime( System::currentTimeMillis() )
{
	shouldDelay = false;
}

unordered_map<int, packetCreateFn> Packet::idToCreateMap;

unordered_set<int> Packet::clientReceivedPackets = unordered_set<int>();
unordered_set<int> Packet::serverReceivedPackets = unordered_set<int>();
unordered_set<int> Packet::sendToAnyClientPackets = unordered_set<int>();

// 4J Added
unordered_map<int, Packet::PacketStatistics *> Packet::outgoingStatistics = unordered_map<int, Packet::PacketStatistics *>();
vector<Packet::PacketStatistics *> Packet::renderableStats = vector<Packet::PacketStatistics *>();
int Packet::renderPos = 0;

// sendToAnyClient - true - send to anyone, false - Sends to one person per dimension per machine
void Packet::map(int id, bool receiveOnClient, bool receiveOnServer, bool sendToAnyClient, bool renderStats, const type_info& clazz, packetCreateFn createFn)
{
#if 0
	if (idToClassMap.count(id) > 0) throw new IllegalArgumentException(wstring(L"Duplicate packet id:") + _toString<int>(id));
	if (classToIdMap.count(clazz) > 0) throw new IllegalArgumentException(L"Duplicate packet class:"); // TODO + clazz);
#endif

	idToCreateMap.insert( unordered_map<int, packetCreateFn>::value_type(id, createFn) );

#ifndef _CONTENT_PACKAGE
#if PACKET_ENABLE_STAT_TRACKING
	Packet::PacketStatistics *packetStatistics = new PacketStatistics(id);
	outgoingStatistics[id] = packetStatistics;

	if(renderStats)
	{
		renderableStats.push_back( packetStatistics );
	}
#endif
#endif

	if (receiveOnClient)
	{
		clientReceivedPackets.insert(id);
	}
	if (receiveOnServer)
	{
		serverReceivedPackets.insert(id);
	}
	if(sendToAnyClient)
	{
		sendToAnyClientPackets.insert(id);
	}
}

// 4J Added to record data for outgoing packets
void Packet::recordOutgoingPacket(shared_ptr<Packet> packet)
{
#ifndef _CONTENT_PACKAGE
#if PACKET_ENABLE_STAT_TRACKING
	AUTO_VAR(it, outgoingStatistics.find(packet->getId()));

	if( it == outgoingStatistics.end() )
	{
		Packet::PacketStatistics *packetStatistics = new PacketStatistics(packet->getId());
		outgoingStatistics[packet->getId()] = packetStatistics;
		packetStatistics->addPacket(packet->getEstimatedSize());
	}
	else
	{
		it->second->addPacket(packet->getEstimatedSize());
	}
#endif
#endif
}

void Packet::renderPacketStats(int id)
{
	AUTO_VAR(it, outgoingStatistics.find(id));

	if( it != outgoingStatistics.end() )
	{
		it->second->renderStats();
	}
}

void Packet::renderAllPacketStats()
{
#ifndef _CONTENT_PACKAGE
#if PACKET_ENABLE_STAT_TRACKING
	Minecraft *pMinecraft = Minecraft::GetInstance();
	pMinecraft->gui->renderStackedGraph(Packet::renderPos, 512, renderableStats.size(), &Packet::getIndexedStatValue );

	renderAllPacketStatsKey();

	Packet::renderPos++;
	Packet::renderPos%=511;
#endif
#endif
}

void Packet::renderAllPacketStatsKey()
{
#ifndef _CONTENT_PACKAGE	
#if PACKET_ENABLE_STAT_TRACKING
	Minecraft *pMinecraft = Minecraft::GetInstance();
	int total = Packet::renderableStats.size();
	for(unsigned int i = 0; i < total; ++i)
	{
		Packet::PacketStatistics *stat = Packet::renderableStats[i];
		float vary = (float)i/total;
		int fColour = floor(vary * 0xffffff);

		int colour = 0xff000000 + fColour;
		pMinecraft->gui->drawString( pMinecraft->font, stat->getLegendString(), 900, 30 + (10 * i), colour);
	}
#endif
#endif
}

__int64 Packet::getIndexedStatValue(unsigned int samplePos, unsigned int renderableId)
{
	__int64 val = 0;

#ifndef _CONTENT_PACKAGE
#if PACKET_ENABLE_STAT_TRACKING
	val = renderableStats[renderableId]->getCountSample(samplePos);
#endif
#endif

	return val;
}


shared_ptr<Packet> Packet::getPacket(int id) 
{
	// 4J - removed try/catch
	//    try
	//	{
	return idToCreateMap[id]();
	//    } 
	//	catch (exception e)
	//	{
	//		// TODO 4J JEV print stack trace, newInstance doesnt throw an exception in c++ yet.
	//        printf("Skipping packet with id %d" , id);
	//        return NULL;
	//    }
}

void Packet::writeBytes(DataOutputStream *dataoutputstream, byteArray bytes)
{
	dataoutputstream->writeShort(bytes.length);
	dataoutputstream->write(bytes);
}

byteArray Packet::readBytes(DataInputStream *datainputstream)
{
	int size = datainputstream->readShort();
	if (size < 0)
	{
		app.DebugPrintf("Key was smaller than nothing!  Weird key!");
#ifndef _CONTENT_PACKAGE
		__debugbreak();
#endif
		return byteArray();
		//throw new IOException("Key was smaller than nothing!  Weird key!");
	}

	byteArray bytes(size);
	datainputstream->read(bytes);

	return bytes;
}


bool Packet::canSendToAnyClient(shared_ptr<Packet> packet)
{
	int packetId = packet->getId();

	return sendToAnyClientPackets.count(packetId) != 0;
}

// 4J - now a pure virtual method
/*
int Packet::getId()
{
return id;
}
*/

unordered_map<int, Packet::PacketStatistics *> Packet::statistics = unordered_map<int, Packet::PacketStatistics *>();

//int Packet::nextPrint = 0;

shared_ptr<Packet> Packet::readPacket(DataInputStream *dis, bool isServer) // throws IOException TODO 4J JEV, should this declare a throws?
{
	int id = 0;
	shared_ptr<Packet> packet = nullptr;

	// 4J - removed try/catch
	//    try
	//	{
	id = dis->read();
	if (id == -1) return nullptr;

	if ((isServer && serverReceivedPackets.find(id) == serverReceivedPackets.end()) || (!isServer && clientReceivedPackets.find(id) == clientReceivedPackets.end()))
	{
		//app.DebugPrintf("Bad packet id %d\n", id);
		__debugbreak();
		assert(false);
		//            throw new IOException(wstring(L"Bad packet id ") + _toString<int>(id));
	}

	packet = getPacket(id);
	if (packet == NULL) assert(false);//throw new IOException(wstring(L"Bad packet id ") + _toString<int>(id));
	
	//app.DebugPrintf("%s reading packet %d\n", isServer ? "Server" : "Client", packet->getId());
	packet->read(dis);
	//    }
	//	catch (EOFException e) 
	//	{
	//       // reached end of stream
	//        OutputDebugString("Reached end of stream");
	//        return NULL;
	//    }

	// 4J - Don't bother tracking stats in a content package
	// 4J Stu - This changes a bit in 1.0.1, but we don't really use it so stick with what we have
#ifndef _CONTENT_PACKAGE
#if PACKET_ENABLE_STAT_TRACKING
	AUTO_VAR(it, statistics.find(id));

	if( it == statistics.end() )
	{
		Packet::PacketStatistics *packetStatistics = new PacketStatistics(id);
		statistics[id] = packetStatistics;
		packetStatistics->addPacket(packet->getEstimatedSize());
	}
	else
	{
		it->second->addPacket(packet->getEstimatedSize());
	}
#endif
#endif

	return packet;
}

void Packet::writePacket(shared_ptr<Packet> packet, DataOutputStream *dos) // throws IOException TODO 4J JEV, should this declare a throws?
{
	//app.DebugPrintf("Writing packet %d\n", packet->getId());
	dos->write(packet->getId());
	packet->write(dos);
}

void Packet::writeUtf(const wstring& value, DataOutputStream *dos) // throws IOException TODO 4J JEV, should this declare a throws?
{
#if 0
	if (value.length() > Short::MAX_VALUE) 
	{
		throw new IOException(L"String too big");
	}
#endif

	dos->writeShort((short)value.length());
	dos->writeChars(value);
}

wstring Packet::readUtf(DataInputStream *dis, int maxLength) // throws IOException TODO 4J JEV, should this declare a throws?
{

	short stringLength = dis->readShort();
	if (stringLength > maxLength)
	{
		wstringstream stream;
		stream << L"Received string length longer than maximum allowed (" << stringLength << " > " << maxLength << ")";
		assert(false);
		//        throw new IOException( stream.str() );
	}
	if (stringLength < 0)
	{
		assert(false);
		//        throw new IOException(L"Received string length is less than zero! Weird string!");
	}

	wstring builder = L"";
	for (int i = 0; i < stringLength; i++) 
	{
		wchar_t rc = dis->readChar();
		builder.push_back( rc );
	}

	return builder;
}

void Packet::PacketStatistics::addPacket(int bytes)
{
	if(count == 0)
	{
		firstSampleTime = System::currentTimeMillis();
	}
	count++;
	totalSize += bytes;

	// 4J Added	
	countSamples[samplesPos & (512 - 1)]++;
	sizeSamples[samplesPos & (512 - 1)] += (unsigned int) bytes;
}

int Packet::PacketStatistics::getCount() 
{
	return count;
}

double Packet::PacketStatistics::getAverageSize()
{
	if (count == 0)
	{
		return 0;
	}
	return (double) totalSize / count;
}

void Packet::PacketStatistics::renderStats( )
{
#ifndef _CONTENT_PACKAGE
#if PACKET_ENABLE_STAT_TRACKING
	samplesPos++;

	countSamples[samplesPos & (512 - 1)] = 0;
	sizeSamples[samplesPos & (512 - 1)] = 0;

	Minecraft *pMinecraft = Minecraft::GetInstance();
	pMinecraft->gui->renderGraph(512, samplesPos, countSamples, 1, 10, sizeSamples, 1, 50);
#endif
#endif
}

__int64 Packet::PacketStatistics::getCountSample(int samplePos)
{
	if(samplePos == 511)
	{
		samplesPos++;
		countSamples[samplesPos & (512 - 1)] = 0;
		sizeSamples[samplesPos & (512 - 1)] = 0;
	}

	return countSamples[samplePos] * 10;
}

wstring Packet::PacketStatistics::getLegendString()
{
	static wchar_t string[128];
	double bps = 0.0;
	if(firstSampleTime > 0)
	{
		float timeDiff = ((System::currentTimeMillis() - firstSampleTime)/1000);
		if(timeDiff > 0) bps = totalSize / timeDiff;
	}
	swprintf(string, 128, L"id: %d , packets: %d , total: %d , bytes: %d, total: %d, %f Bps", id, countSamples[(samplesPos - 1) & (512 - 1)], count, sizeSamples[(samplesPos - 1) & (512 - 1)], totalSize, bps );
	return string;
}

bool Packet::canBeInvalidated()
{
	return false;
}

bool Packet::isInvalidatedBy(shared_ptr<Packet> packet)
{
	return false;
}

bool Packet::isAync()
{
	return false;
}

// 4J Stu - Brought these functions forward for enchanting/game rules
shared_ptr<ItemInstance> Packet::readItem(DataInputStream *dis)
{
	shared_ptr<ItemInstance> item = nullptr;
	int id = dis->readShort();
	if (id >= 0)
	{
		int count = dis->readByte();
		int damage = dis->readShort();

		item = shared_ptr<ItemInstance>( new ItemInstance(id, count, damage) );
		// 4J Stu - Always read/write the tag
		//if (Item.items[id].canBeDepleted() || Item.items[id].shouldOverrideMultiplayerNBT())
		{
			item->tag = readNbt(dis);
		}
	}

	return item;
}

void Packet::writeItem(shared_ptr<ItemInstance> item, DataOutputStream *dos)
{
	if (item == NULL)
	{
		dos->writeShort(-1);
	}
	else
	{
		dos->writeShort(item->id);
		dos->writeByte(item->count);
		dos->writeShort(item->getAuxValue());
		// 4J Stu - Always read/write the tag
		//if (item.getItem().canBeDepleted() || item.getItem().shouldOverrideMultiplayerNBT())
		{
			writeNbt(item->tag, dos);
		}
	}
}

CompoundTag *Packet::readNbt(DataInputStream *dis)
{
	int size = dis->readShort();
	if (size < 0) return NULL;
	byteArray buff(size);
	dis->readFully(buff);
	CompoundTag *result = (CompoundTag *) NbtIo::decompress(buff);
	delete [] buff.data;
	return result;
}

void Packet::writeNbt(CompoundTag *tag, DataOutputStream *dos)
{
	if (tag == NULL)
	{
		dos->writeShort(-1);
	}
	else
	{
		byteArray buff = NbtIo::compress(tag);
		dos->writeShort((short) buff.length);
		dos->write(buff);
		delete [] buff.data;
	}
}
