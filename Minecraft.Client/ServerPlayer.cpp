#include "stdafx.h"
#include "ServerPlayer.h"
#include "ServerPlayerGameMode.h"
#include "ServerLevel.h"
#include "MinecraftServer.h"
#include "EntityTracker.h"
#include "PlayerConnection.h"
#include "Settings.h"
#include "PlayerList.h"
#include "MultiPlayerLevel.h"

#include "..\Minecraft.World\net.minecraft.network.packet.h"
#include "..\Minecraft.World\net.minecraft.world.damagesource.h"
#include "..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.storage.h"
#include "..\Minecraft.World\net.minecraft.world.level.dimension.h"
#include "..\Minecraft.World\net.minecraft.world.entity.projectile.h"
#include "..\Minecraft.World\net.minecraft.world.entity.h"
#include "..\Minecraft.World\net.minecraft.world.entity.animal.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "..\Minecraft.World\net.minecraft.world.item.trading.h"
#include "..\Minecraft.World\net.minecraft.world.entity.item.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.entity.h"
#include "..\Minecraft.World\net.minecraft.world.scores.h"
#include "..\Minecraft.World\net.minecraft.world.scores.criteria.h"
#include "..\Minecraft.World\net.minecraft.stats.h"
#include "..\Minecraft.World\net.minecraft.locale.h"

#include "..\Minecraft.World\Pos.h"
#include "..\Minecraft.World\Random.h"

#include "..\Minecraft.World\LevelChunk.h"
#include "LevelRenderer.h"


ServerPlayer::ServerPlayer(MinecraftServer *server, Level *level, const wstring& name, ServerPlayerGameMode *gameMode) : Player(level, name)
{
	// 4J - added initialisers
	connection = nullptr;
	lastMoveX = lastMoveZ = 0;
	spewTimer = 0;
	lastRecordedHealthAndAbsorption = FLT_MIN;
	lastSentHealth = -99999999;
	lastSentFood = -99999999;
	lastFoodSaturationZero = true;
	lastSentExp = -99999999;
	invulnerableTime = 20 * 3;
	containerCounter = 0;
	ignoreSlotUpdateHack = false;
	latency = 0;
	wonGame = false;
	m_enteredEndExitPortal = false;
	// lastCarried = ItemInstanceArray(5);
	lastActionTime = 0;

	viewDistance = server->getPlayers()->getViewDistance();

	//    gameMode->player = this;		// 4J - removed to avoid use of shared_from_this in ctor, now set up externally
	this->gameMode = gameMode;

	Pos *spawnPos = level->getSharedSpawnPos();
	int xx = spawnPos->x;
	int zz = spawnPos->z;
	int yy = spawnPos->y;
	delete spawnPos;

	if (!level->dimension->hasCeiling && level->getLevelData()->getGameType() != GameType::ADVENTURE)
	{
		level->isFindingSpawn = true;

		int radius = max(5, server->getSpawnProtectionRadius() - 6);

		// 4J added - do additional checking that we aren't putting the player in deep water. Give up after 20 or goes just
		// in case the spawnPos is somehow in a really bad spot and we would just lock here.
		int waterDepth = 0;
		int attemptCount = 0;
		int xx2, yy2, zz2;

		int minXZ = - (level->dimension->getXZSize() * 16 ) / 2;
		int maxXZ = (level->dimension->getXZSize() * 16 ) / 2 - 1;

		bool playerNear = false;
		do
		{
			// Also check that we aren't straying outside of the map
			do
			{
				xx2 = xx + random->nextInt(radius * 2) - radius;
				zz2 = zz + random->nextInt(radius * 2) - radius;
			} while ( ( xx2 > maxXZ ) || ( xx2 < minXZ ) || ( zz2 > maxXZ ) || ( zz2 < minXZ ) );
			yy2 = level->getTopSolidBlock(xx2, zz2);

			waterDepth = 0;
			int yw = yy2;
			while( ( yw < 128 ) &&
				(( level->getTile(xx2,yw,zz2) == Tile::water_Id ) ||
				( level->getTile(xx2,yw,zz2) == Tile::calmWater_Id )) )
			{
				yw++;
				waterDepth++;
			}
			attemptCount++;
			playerNear = ( level->getNearestPlayer(xx + 0.5, yy, zz + 0.5,3) != nullptr );
		} while ( ( waterDepth > 1 ) && (!playerNear) && ( attemptCount < 20 ) );
		xx = xx2;
		yy = yy2;
		zz = zz2;

		level->isFindingSpawn = false;
	}

	this->server = server;
	footSize = 0;

	heightOffset = 0;	// 4J - this height used to be set up after moveTo, but that ends up with the y value being incorrect as it depends on this offset
	this->moveTo(xx + 0.5, yy, zz + 0.5, 0, 0);

	// 4J Handled later
	//while (!level->getCubes(this, bb).empty())
	//{
	//	setPos(x, y + 1, z);
	//}

	// m_UUID = name;

	// 4J Added
	lastBrupSendTickCount = 0;
}

ServerPlayer::~ServerPlayer()
{
	// delete [] lastCarried.data;
}

// 4J added - add bits to a flag array that is passed in, to represent those entities which have small Ids, and are in our vector of entitiesToRemove.
// If there aren't any entities to be flagged, this function does nothing. If there *are* entities to be added, uses the removedFound as an input to
// determine if the flag array has already been initialised at all - if it has been, then just adds flags to it; if it hasn't, then memsets the output
// flag array and adds to it for this ServerPlayer.
void ServerPlayer::flagEntitiesToBeRemoved(unsigned int *flags, bool *removedFound)
{
	if( entitiesToRemove.empty() )
	{
		return;
	}
	if( ( *removedFound ) == false )
	{
		*removedFound = true;
		// before this left 192 bytes uninitialized!!!!!
        memset(flags, 0, (2048 / 32) * sizeof(unsigned int));
	}

	for(int index : entitiesToRemove)
	{
		if( index < 2048 )
		{
			unsigned int i = index / 32;
			unsigned int j = index % 32;
			unsigned int uiMask = 0x80000000 >> j;

			flags[i] |= uiMask;
		}
	}
}

void ServerPlayer::readAdditionalSaveData(CompoundTag *entityTag)
{
	Player::readAdditionalSaveData(entityTag);

	if (entityTag->contains(L"playerGameType"))
	{
		// 4J Stu - We do not want to change the game mode for the player, instead we let the server override it globally
		//if (MinecraftServer::getInstance()->getForceGameType())
		//{
		//	gameMode->setGameModeForPlayer(MinecraftServer::getInstance()->getDefaultGameType());
		//}
		//else
		//{
		//	gameMode->setGameModeForPlayer(GameType::byId(entityTag->getInt(L"playerGameType")));
		//}
	}

	GameRulesInstance *grs = gameMode->getGameRules();
	if (entityTag->contains(L"GameRules") && grs != nullptr)
	{
		byteArray ba = entityTag->getByteArray(L"GameRules");
		ByteArrayInputStream bais(ba);
		DataInputStream dis(&bais);
		grs->read(&dis);
		dis.close();
		bais.close();
		//delete [] ba.data;
	}
}

void ServerPlayer::addAdditonalSaveData(CompoundTag *entityTag)
{
	Player::addAdditonalSaveData(entityTag);

	GameRulesInstance *grs = gameMode->getGameRules();
	if (grs != nullptr)
	{
		ByteArrayOutputStream baos;
		DataOutputStream dos(&baos);
		grs->write(&dos);
		entityTag->putByteArray(L"GameRules", baos.buf);
		baos.buf.data = nullptr;
		dos.close();
		baos.close();
	}

	// 4J Stu - We do not want to change the game mode for the player, instead we let the server override it globally
	//entityTag->putInt(L"playerGameType", gameMode->getGameModeForPlayer()->getId());
}

void ServerPlayer::giveExperienceLevels(int amount)
{
	Player::giveExperienceLevels(amount);
	lastSentExp = -1;
}

void ServerPlayer::initMenu()
{
	containerMenu->addSlotListener(this);
}

void ServerPlayer::setDefaultHeadHeight()
{
	heightOffset = 0;
}

float ServerPlayer::getHeadHeight()
{
	return 1.62f;
}

void ServerPlayer::tick()
{
	gameMode->tick();

	if (invulnerableTime > 0) invulnerableTime--;
	containerMenu->broadcastChanges();

	// 4J-JEV, hook for Durango event 'EnteredNewBiome'.
	Biome *newBiome = level->getBiome(x,z);
	if (newBiome != currentBiome)
	{
		awardStat(
			GenericStats::enteredBiome(newBiome->id),
			GenericStats::param_enteredBiome(newBiome->id)
			);
		currentBiome = newBiome;
	}

	if (!level->isClientSide)
	{
		if (!containerMenu->stillValid(dynamic_pointer_cast<Player>(shared_from_this())))
		{
			closeContainer();
			containerMenu = inventoryMenu;
		}
	}

	flushEntitiesToRemove();
}

// 4J Stu - Split out here so that we can call this from other places
void ServerPlayer::flushEntitiesToRemove()
{
	while (!entitiesToRemove.empty())
	{
		int sz = entitiesToRemove.size();
		int amount = min(sz, RemoveEntitiesPacket::MAX_PER_PACKET);
		intArray ids(amount);
		int pos = 0;

        auto it = entitiesToRemove.begin();
        while (it != entitiesToRemove.end() && pos < amount)
		{
			ids[pos++] = *it;
			it = entitiesToRemove.erase(it);
		}

		connection->send(std::make_shared<RemoveEntitiesPacket>(ids));
	}
}

// 4J - have split doTick into 3 bits, so that we can call the doChunkSendingTick separately, but still do the equivalent of what calling a full doTick used to do, by calling this method
void ServerPlayer::doTick(bool sendChunks, bool dontDelayChunks/*=false*/, bool ignorePortal/*=false*/)
{
	m_ignorePortal = ignorePortal;
	if( sendChunks )
	{
		updateFrameTick();
	}
	doTickA();
	if( sendChunks )
	{
		doChunkSendingTick(dontDelayChunks);
	}
	doTickB();
	m_ignorePortal = false;
}

void ServerPlayer::doTickA()
{
	Player::tick();

	for (unsigned int i = 0; i < inventory->getContainerSize(); i++)
	{
		shared_ptr<ItemInstance> ie = inventory->getItem(i);
		if (ie != nullptr)
		{
			// 4J - removed condition. These were getting lower priority than tile update packets etc. on the slow outbound queue, and so were extremely slow to send sometimes,
			// particularly at the start of a game. They don't typically seem to be massive and shouldn't be send when there isn't actually any updating to do.
			if (Item::items[ie->id]->isComplex() ) // && connection->countDelayedPackets() <= 2)
			{
				shared_ptr<Packet> packet = (dynamic_cast<ComplexItem *>(Item::items[ie->id])->getUpdatePacket(ie, level, dynamic_pointer_cast<Player>( shared_from_this() ) ) );
				if (packet != nullptr)
				{
					connection->send(packet);
				}
			}
		}
	}
}

// 4J - split off the chunk sending bit of the tick here from ::doTick so we can do this exactly once per player per server tick
void ServerPlayer::doChunkSendingTick(bool dontDelayChunks)
{
	//	printf("[%d] %s: sendChunks: %d, empty: %d\n",tickCount, connection->getNetworkPlayer()->GetUID().getOnlineID(),sendChunks,chunksToSend.empty());
	if (!chunksToSend.empty())
	{
		ChunkPos nearest = chunksToSend.front();
		bool nearestValid = false;

		// 4J - reinstated and optimised some code that was commented out in the original, to make sure that we always
		// send the nearest chunk to the player. The original uses the bukkit sorting thing to try and avoid doing this, but
		// the player can quickly wander away from the centre of the spiral of chunks that that method creates, long before transmission
		// of them is complete.
		double dist = DBL_MAX;
		for(ChunkPos chunk : chunksToSend)
		{
			if( level->isChunkFinalised(chunk.x, chunk.z) )
			{
				double newDist = chunk.distanceToSqr(x, z);
				if ( (!nearestValid) || (newDist < dist) )
				{
					nearest = chunk;
					dist = chunk.distanceToSqr(x, z);
					nearestValid = true;
				}
			}
		}

		//        if (nearest != nullptr)		// 4J - removed as we don't have references here
		if( nearestValid )
		{
			bool okToSend = false;

			//                if (dist < 32 * 32) okToSend = true;
			if( connection->isLocal() )
			{
				if( !connection->done ) okToSend = true;
			}
			else
			{
				bool canSendToPlayer = MinecraftServer::chunkPacketManagement_CanSendTo(connection->getNetworkPlayer());

//				app.DebugPrintf(">>> %d\n", canSendToPlayer);
//				if( connection->getNetworkPlayer() )
//				{
//					app.DebugPrintf("%d: canSendToPlayer %d, countDelayedPackets %d GetSendQueueSizeBytes %d done: %d\n",
//						connection->getNetworkPlayer()->GetSmallId(),
//						canSendToPlayer, connection->countDelayedPackets(),
//						g_NetworkManager.GetHostPlayer()->GetSendQueueSizeMessages( nullptr, true ),
//						connection->done);
//				}

#ifdef MINECRAFT_SERVER_BUILD
				if (dontDelayChunks || (canSendToPlayer && !connection->done))
#else
				if( dontDelayChunks ||
					(canSendToPlayer &&
#ifdef _XBOX_ONE
					// The network manager on xbox one doesn't currently split data into slow & fast queues - since we can only measure
					// both together then bytes provides a better metric than count of data items to determine if we should avoid queueing too much up
					(g_NetworkManager.GetHostPlayer()->GetSendQueueSizeBytes( nullptr, true ) < 8192 )&&
#elif defined _XBOX
					(g_NetworkManager.GetHostPlayer()->GetSendQueueSizeMessages( nullptr, true ) < 4 )&&
#else
					(connection->countDelayedPackets() < 4 )&&
					(g_NetworkManager.GetHostPlayer()->GetSendQueueSizeMessages( nullptr, true ) < 4 )&&
#endif
					//(tickCount - lastBrupSendTickCount) > (connection->getNetworkPlayer()->GetCurrentRtt()>>4) &&
					!connection->done) )
#endif
                {
                    lastBrupSendTickCount = tickCount;
                    okToSend = true;
                    MinecraftServer::chunkPacketManagement_DidSendTo(connection->getNetworkPlayer());

                    //					static unordered_map<wstring,int64_t> mapLastTime;
                    //					int64_t thisTime = System::currentTimeMillis();
                    //					int64_t lastTime = mapLastTime[connection->getNetworkPlayer()->GetUID().toString()];
                    //					app.DebugPrintf(" - OK to send (%d ms since last)\n", thisTime - lastTime);
                    //					mapLastTime[connection->getNetworkPlayer()->GetUID().toString()] = thisTime;
                }
                else
                {
                    //					app.DebugPrintf(" - <NOT OK>\n");
                }
			}

			if (okToSend)
			{
				ServerLevel *level = server->getLevel(dimension);
				int flagIndex = getFlagIndexForChunk(nearest,this->level->dimension->id);
				chunksToSend.remove(nearest);

				bool chunkDataSent = false;

				// Don't send the chunk to the local machine - the chunks there are mapped directly to the server chunks. We could potentially stop this process earlier on by not adding
				// to the chunksToSend list, but that would stop the tile entities being broadcast too
				if( !connection->isLocal() )  // force here to disable sharing of data
				{
					// Don't send the chunk if we've set a flag to say that we've already sent it to this machine. This stops two things
					// (1) Sending a chunk to multiple players doing split screen on one machine
					// (2) Sending a chunk that we've already sent as the player moves around. The original version of the game resends these, since it maintains
					//     a region of active chunks round each player in the "infinite" world, but in our finite world, we don't ever request that chunks be
					//     unloaded on the client and so just gradually build up more and more of the finite set of chunks as the player moves
					if( !g_NetworkManager.SystemFlagGet(connection->getNetworkPlayer(),flagIndex) )
					{
						PIXBeginNamedEvent(0,"Creation BRUP for sending\n");
						int64_t before = System::currentTimeMillis();
						const auto packet = std::make_shared<BlockRegionUpdatePacket>(nearest.x * 16, 0, nearest.z * 16, 16, Level::maxBuildHeight, 16, level);
						int64_t after = System::currentTimeMillis();
//						app.DebugPrintf(">>><<< %d ms\n",after-before);
						PIXEndNamedEvent();
						if( dontDelayChunks ) packet->shouldDelay = false;

						if( packet->shouldDelay == true )
						{
							// Other than the first packet we always want these initial chunks to be sent over
							// QNet at a lower priority
							connection->queueSend( packet );
						}
						else
						{
							connection->send( packet );
						}
						// Set flag to say we have send this block already to this system
						g_NetworkManager.SystemFlagSet(connection->getNetworkPlayer(),flagIndex);

						chunkDataSent = true;
					}
				}
				else
				{
					// For local connections, we'll need to copy the lighting data over from server to client at this point. This is to try and keep lighting as similar as possible to the java version,
					// where client & server are individually responsible for maintaining their lighting (since 1.2.3). This is really an alternative to sending the lighting data over the fake local
					// network connection at this point.

					MultiPlayerLevel *clientLevel = Minecraft::GetInstance()->getLevel(level->dimension->id);
					if( clientLevel )
					{
						LevelChunk *lc = clientLevel->getChunk( nearest.x, nearest.z );
						lc->reSyncLighting();
						lc->recalcHeightmapOnly();
						clientLevel->setTilesDirty(nearest.x * 16 + 1, 1, nearest.z * 16 + 1,
							nearest.x * 16 + 14, Level::maxBuildHeight - 2, nearest.z * 16 + 14 );
					}
				}
				// Don't send TileEntity data until we have sent the block data
				if( connection->isLocal() || chunkDataSent)
				{
					vector<shared_ptr<TileEntity> > *tes = level->getTileEntitiesInRegion(nearest.x * 16, 0, nearest.z * 16, nearest.x * 16 + 16, Level::maxBuildHeight, nearest.z * 16 + 16);
					for (unsigned int i = 0; i < tes->size(); i++)
					{
						// 4J Stu - Added delay param to ensure that these arrive after the BRUPs from above
						// Fix for #9169 - ART : Sign text is replaced with the words �Awaiting approval�.
						broadcast(tes->at(i), !connection->isLocal() && !dontDelayChunks);
					}
					delete tes;
				}
			}
		}
	}
}

void ServerPlayer::doTickB()
{
#ifndef _CONTENT_PACKAGE
	// check if there's a debug dimension change requested
	//if(app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_GoToNether))
	//{
	//	if(level->dimension->id == 0 )
	//	{
	//		isInsidePortal=true;
	//		portalTime=1;
	//	}
	//	unsigned int uiVal=app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad());
	//	app.SetGameSettingsDebugMask(ProfileManager.GetPrimaryPad(),uiVal&~(1L<<eDebugSetting_GoToNether));
	//}
	// 	else if (app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_GoToEnd))
	// 	{
	// 		if(level->dimension->id == 0 )
	// 		{
	// 			server->players->toggleDimension( dynamic_pointer_cast<ServerPlayer>( shared_from_this() ), 1 );
	// 		}
	// 		unsigned int uiVal=app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad());
	// 		app.SetGameSettingsDebugMask(ProfileManager.GetPrimaryPad(),uiVal&~(1L<<eDebugSetting_GoToEnd));
	// 	}
	//else
		if (app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_GoToOverworld))
	{
		if(level->dimension->id != 0 )
		{
			isInsidePortal=true;
			portalTime=1;
		}
		unsigned int uiVal=app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad());
		app.SetGameSettingsDebugMask(ProfileManager.GetPrimaryPad(),uiVal&~(1L<<eDebugSetting_GoToOverworld));
	}
#endif

	if (getHealth() != lastSentHealth || lastSentFood != foodData.getFoodLevel() || ((foodData.getSaturationLevel() == 0) != lastFoodSaturationZero))
	{
		// 4J Stu - Added m_lastDamageSource for telemetry
		connection->send(std::make_shared<SetHealthPacket>(getHealth(), foodData.getFoodLevel(), foodData.getSaturationLevel(), m_lastDamageSource));
		lastSentHealth = getHealth();
		lastSentFood = foodData.getFoodLevel();
		lastFoodSaturationZero = foodData.getSaturationLevel() == 0;
	}

	if (getHealth() + getAbsorptionAmount() != lastRecordedHealthAndAbsorption)
	{
		lastRecordedHealthAndAbsorption = getHealth() + getAbsorptionAmount();

		vector<Objective *> *objectives = getScoreboard()->findObjectiveFor(ObjectiveCriteria::HEALTH);
		if(objectives)
		{
			vector< shared_ptr<Player> > players = vector< shared_ptr<Player> >();
			players.push_back(dynamic_pointer_cast<Player>(shared_from_this()));

			for (Objective *objective : *objectives)
			{
				getScoreboard()->getPlayerScore(getAName(), objective)->updateFor(&players);
			}
			delete objectives;
		}
	}

	if (totalExperience != lastSentExp)
	{
		lastSentExp = totalExperience;
		connection->send(std::make_shared<SetExperiencePacket>(experienceProgress, totalExperience, experienceLevel));
	}

}

shared_ptr<ItemInstance> ServerPlayer::getCarried(int slot)
{
	if (slot == 0) return inventory->getSelected();
	return inventory->armor[slot - 1];
}

void ServerPlayer::die(DamageSource *source)
{
	server->getPlayers()->broadcastAll(getCombatTracker()->getDeathMessagePacket());

	if (!level->getGameRules()->getBoolean(GameRules::RULE_KEEPINVENTORY))
	{
		inventory->dropAll();
	}

	vector<Objective *> *objectives = level->getScoreboard()->findObjectiveFor(ObjectiveCriteria::DEATH_COUNT);
	if(objectives)
	{
		for (int i = 0; i < objectives->size(); i++)
		{
			Objective *objective = objectives->at(i);

			Score *score = getScoreboard()->getPlayerScore(getAName(), objective);
			score->increment();
		}
		delete objectives;
	}

	shared_ptr<LivingEntity> killer = getKillCredit();
	if (killer != nullptr) killer->awardKillScore(shared_from_this(), deathScore);
	//awardStat(Stats::deaths, 1);
}

bool ServerPlayer::hurt(DamageSource *dmgSource, float dmg)
{
	if (isInvulnerable()) return false;

	// 4J: Not relevant to console servers
	// Allow falldamage on dedicated pvpservers -- so people cannot cheat their way out of 'fall traps'
	//bool allowFallDamage = server->isPvpAllowed() && server->isDedicatedServer() && server->isPvpAllowed() && (dmgSource->msgId.compare(L"fall") == 0);
	if (!server->isPvpAllowed() && invulnerableTime > 0 && dmgSource != DamageSource::outOfWorld) return false;

	if (dynamic_cast<EntityDamageSource *>(dmgSource) != nullptr)
	{
		// 4J Stu - Fix for #46422 - TU5: Crash: Gameplay: Crash when being hit by a trap using a dispenser
		// getEntity returns the owner of projectiles, and this would never be the arrow. The owner is sometimes nullptr.
		shared_ptr<Entity> source = dmgSource->getDirectEntity();

		if (source->instanceof(eTYPE_PLAYER) && !dynamic_pointer_cast<Player>(source)->canHarmPlayer(dynamic_pointer_cast<Player>(shared_from_this())))
		{
			return false;
		}

		if ( (source != nullptr) && source->instanceof(eTYPE_ARROW) )
		{
			shared_ptr<Arrow> arrow = dynamic_pointer_cast<Arrow>(source);
			if ( (arrow->owner != nullptr) && arrow->owner->instanceof(eTYPE_PLAYER) && !canHarmPlayer(dynamic_pointer_cast<Player>(arrow->owner))  )
			{
				return false;
			}
		}
	}
	bool returnVal = Player::hurt(dmgSource, dmg);

	if( returnVal )
	{
		// 4J Stu - Work out the source of this damage for telemetry
		m_lastDamageSource = eTelemetryChallenges_Unknown;

		if(dmgSource == DamageSource::fall) m_lastDamageSource = eTelemetryPlayerDeathSource_Fall;
		else if(dmgSource == DamageSource::onFire || dmgSource == DamageSource::inFire) m_lastDamageSource = eTelemetryPlayerDeathSource_Fire;
		else if(dmgSource == DamageSource::lava) m_lastDamageSource = eTelemetryPlayerDeathSource_Lava;
		else if(dmgSource == DamageSource::drown) m_lastDamageSource = eTelemetryPlayerDeathSource_Water;
		else if(dmgSource == DamageSource::inWall) m_lastDamageSource = eTelemetryPlayerDeathSource_Suffocate;
		else if(dmgSource == DamageSource::outOfWorld) m_lastDamageSource = eTelemetryPlayerDeathSource_OutOfWorld;
		else if(dmgSource == DamageSource::cactus) m_lastDamageSource = eTelemetryPlayerDeathSource_Cactus;
		else
		{
			shared_ptr<Entity> source = dmgSource->getEntity();
			if( source != nullptr )
			{
				switch(source->GetType())
				{
				case eTYPE_PLAYER:
				case eTYPE_SERVERPLAYER:
					m_lastDamageSource = eTelemetryPlayerDeathSource_Player_Weapon;
					break;
				case eTYPE_WOLF:
					m_lastDamageSource = eTelemetryPlayerDeathSource_Wolf;
					break;
				case eTYPE_CREEPER:
					m_lastDamageSource = eTelemetryPlayerDeathSource_Explosion_Creeper;
					break;
				case eTYPE_SKELETON:
					m_lastDamageSource = eTelemetryPlayerDeathSource_Skeleton;
					break;
				case eTYPE_SPIDER:
					m_lastDamageSource = eTelemetryPlayerDeathSource_Spider;
					break;
				case eTYPE_ZOMBIE:
					m_lastDamageSource = eTelemetryPlayerDeathSource_Zombie;
					break;
				case eTYPE_PIGZOMBIE:
					m_lastDamageSource = eTelemetryPlayerDeathSource_ZombiePigman;
					break;
				case eTYPE_GHAST:
					m_lastDamageSource = eTelemetryPlayerDeathSource_Ghast;
					break;
				case eTYPE_SLIME:
					m_lastDamageSource = eTelemetryPlayerDeathSource_Slime;
					break;
				case eTYPE_PRIMEDTNT:
					m_lastDamageSource = eTelemetryPlayerDeathSource_Explosion_Tnt;
					break;
				case eTYPE_ARROW:
					if ((dynamic_pointer_cast<Arrow>(source))->owner != nullptr)
					{
						shared_ptr<Entity> attacker = (dynamic_pointer_cast<Arrow>(source))->owner;
						if (attacker != nullptr)
						{
							switch(attacker->GetType())
							{
							case eTYPE_SKELETON:
								m_lastDamageSource = eTelemetryPlayerDeathSource_Skeleton;
								break;
							case eTYPE_PLAYER:
							case eTYPE_SERVERPLAYER:
								m_lastDamageSource = eTelemetryPlayerDeathSource_Player_Arrow;
								break;
							}
						}
					}
					break;
				case eTYPE_FIREBALL:
					m_lastDamageSource = eTelemetryPlayerDeathSource_Ghast;
					break;
				};
			}
		};
	}

	return returnVal;
}

bool ServerPlayer::canHarmPlayer(shared_ptr<Player> target)
{
	if (!server->isPvpAllowed()) return false;
	if(!isAllowedToAttackPlayers()) return false;
	return Player::canHarmPlayer(target);
}

// 4J: Added for checking when only player name is provided (possible player isn't on server), e.g. can harm owned animals
bool ServerPlayer::canHarmPlayer(wstring targetName)
{
	bool canHarm = true;

	shared_ptr<ServerPlayer> owner = server->getPlayers()->getPlayer(targetName);
	if (owner != nullptr)
	{
		if ((shared_from_this() != owner) && canHarmPlayer(owner)) canHarm = false;
	}
	else
	{
		if (this->name != targetName && (!isAllowedToAttackPlayers() || !server->isPvpAllowed())) canHarm = false;
	}

	return canHarm;
}

void ServerPlayer::changeDimension(int i)
{
	if(!connection->hasClientTickedOnce()) return;

	if (dimension == 1 && i == 1)
	{
		app.DebugPrintf("Start win game\n");
		awardStat(GenericStats::winGame(), GenericStats::param_winGame());

		// All players on the same system as this player should also be removed from the game while the Win screen is shown
		INetworkPlayer *thisPlayer = connection->getNetworkPlayer();

		if(!wonGame)
		{
			level->removeEntity(shared_from_this());
			wonGame = true;
			m_enteredEndExitPortal = true; // We only flag this for the player in the portal
			connection->send(std::make_shared<GameEventPacket>(GameEventPacket::WIN_GAME, thisPlayer->GetUserIndex()));
			app.DebugPrintf("Sending packet to %d\n", thisPlayer->GetUserIndex());
		}
		if(thisPlayer)
		{
			for(auto& servPlayer : MinecraftServer::getInstance()->getPlayers()->players)
			{
				INetworkPlayer *checkPlayer = servPlayer->connection->getNetworkPlayer();
				if(thisPlayer != checkPlayer && checkPlayer != nullptr && thisPlayer->IsSameSystem( checkPlayer ) && !servPlayer->wonGame )
				{
					servPlayer->wonGame = true;
					servPlayer->connection->send(std::make_shared<GameEventPacket>(GameEventPacket::WIN_GAME, thisPlayer->GetUserIndex()));
					app.DebugPrintf("Sending packet to %d\n", thisPlayer->GetUserIndex());
				}
			}
		}
		app.DebugPrintf("End win game\n");
	}
	else
	{
		if (dimension == 0 && i == 1)
		{
			awardStat(GenericStats::theEnd(), GenericStats::param_theEnd());

			Pos *pos = server->getLevel(i)->getDimensionSpecificSpawn();
			if (pos != nullptr)
			{
				connection->teleport(pos->x, pos->y, pos->z, 0, 0);
				delete pos;
			}

			i = 1;
		}
		else
		{
			// 4J: Removed on the advice of the mighty King of Achievments (JV)
			// awardStat(GenericStats::portal(), GenericStats::param_portal());
		}
		server->getPlayers()->toggleDimension( dynamic_pointer_cast<ServerPlayer>(shared_from_this()), i);
		lastSentExp = -1;
		lastSentHealth = -1;
		lastSentFood = -1;
	}
}

// 4J Added delay param
void ServerPlayer::broadcast(shared_ptr<TileEntity> te, bool delay /*= false*/)
{
	if (te != nullptr)
	{
		shared_ptr<Packet> p = te->getUpdatePacket();
		if (p != nullptr)
		{
			p->shouldDelay = delay;
			if(delay) connection->queueSend(p);
			else connection->send(p);
		}
	}
}

void ServerPlayer::take(shared_ptr<Entity> e, int orgCount)
{
	Player::take(e, orgCount);
	containerMenu->broadcastChanges();
}

Player::BedSleepingResult ServerPlayer::startSleepInBed(int x, int y, int z, bool bTestUse)
{
	BedSleepingResult result = Player::startSleepInBed(x, y, z, bTestUse);
	if (result == OK)
	{
		shared_ptr<Packet> p = std::make_shared<EntityActionAtPositionPacket>(shared_from_this(), EntityActionAtPositionPacket::START_SLEEP, x, y, z);
		getLevel()->getTracker()->broadcast(shared_from_this(), p);
		connection->teleport(this->x, this->y, this->z, yRot, xRot);
		connection->send(p);
	}
	return result;
}

void ServerPlayer::stopSleepInBed(bool forcefulWakeUp, bool updateLevelList, bool saveRespawnPoint)
{
	if (isSleeping())
	{
		getLevel()->getTracker()->broadcastAndSend(shared_from_this(), std::make_shared<AnimatePacket>(shared_from_this(), AnimatePacket::WAKE_UP));
	}
	Player::stopSleepInBed(forcefulWakeUp, updateLevelList, saveRespawnPoint);
	if (connection != nullptr) connection->teleport(x, y, z, yRot, xRot);
}

void ServerPlayer::ride(shared_ptr<Entity> e)
{
	Player::ride(e);
	connection->send(std::make_shared<SetEntityLinkPacket>(SetEntityLinkPacket::RIDING, shared_from_this(), riding));

	// 4J Removed this - The act of riding will be handled on the client and will change the position
	// of the player. If we also teleport it then we can end up with a repeating movements, e.g. bouncing
	// up and down after exiting a boat due to slight differences in position on the client and server
	//connection->teleport(x, y, z, yRot, xRot);
}

void ServerPlayer::checkFallDamage(double ya, bool onGround)
{
}

void ServerPlayer::doCheckFallDamage(double ya, bool onGround)
{
	Player::checkFallDamage(ya, onGround);
}

void ServerPlayer::openTextEdit(shared_ptr<TileEntity> sign)
{
	shared_ptr<SignTileEntity> signTE = dynamic_pointer_cast<SignTileEntity>(sign);
	if (signTE != nullptr)
	{
		signTE->setAllowedPlayerEditor(dynamic_pointer_cast<Player>(shared_from_this()));
		connection->send(std::make_shared<TileEditorOpenPacket>(TileEditorOpenPacket::SIGN, sign->x, sign->y, sign->z));
	}
}

void ServerPlayer::nextContainerCounter()
{
	containerCounter = (containerCounter % 100) + 1;
}

bool ServerPlayer::startCrafting(int x, int y, int z)
{
	if(containerMenu == inventoryMenu)
	{
		nextContainerCounter();
		connection->send(std::make_shared<ContainerOpenPacket>(containerCounter, ContainerOpenPacket::WORKBENCH, L"", 9, false));
		containerMenu = new CraftingMenu(inventory, level, x, y, z);
		containerMenu->containerId = containerCounter;
		containerMenu->addSlotListener(this);
	}
	else
	{
		app.DebugPrintf("ServerPlayer tried to open crafting container when one was already open\n");
	}

	return true;
}

bool ServerPlayer::openFireworks(int x, int y, int z)
{
	if(containerMenu == inventoryMenu)
	{
		nextContainerCounter();
		connection->send(std::make_shared<ContainerOpenPacket>(containerCounter, ContainerOpenPacket::FIREWORKS, L"", 9, false));
		containerMenu = new FireworksMenu(inventory, level, x, y, z);
		containerMenu->containerId = containerCounter;
		containerMenu->addSlotListener(this);
	}
	else if(dynamic_cast<CraftingMenu *>(containerMenu) != nullptr)
	{
		closeContainer();

		nextContainerCounter();
		connection->send(std::make_shared<ContainerOpenPacket>(containerCounter, ContainerOpenPacket::FIREWORKS, L"", 9, false));
		containerMenu = new FireworksMenu(inventory, level, x, y, z);
		containerMenu->containerId = containerCounter;
		containerMenu->addSlotListener(this);
	}
	else
	{
		app.DebugPrintf("ServerPlayer tried to open crafting container when one was already open\n");
	}

	return true;
}

bool ServerPlayer::startEnchanting(int x, int y, int z, const wstring &name)
{
	if(containerMenu == inventoryMenu)
	{
		nextContainerCounter();
		connection->send(std::make_shared<ContainerOpenPacket>(containerCounter, ContainerOpenPacket::ENCHANTMENT, name.empty() ? L"" : name, 9, !name.empty()));
		containerMenu = new EnchantmentMenu(inventory, level, x, y, z);
		containerMenu->containerId = containerCounter;
		containerMenu->addSlotListener(this);
	}
	else
	{
		app.DebugPrintf("ServerPlayer tried to open enchanting container when one was already open\n");
	}

	return true;
}

bool ServerPlayer::startRepairing(int x, int y, int z)
{
	if(containerMenu == inventoryMenu)
	{
		nextContainerCounter();
		connection->send(std::make_shared<ContainerOpenPacket>(containerCounter, ContainerOpenPacket::REPAIR_TABLE, L"", 9, false));
		containerMenu = new AnvilMenu(inventory, level, x, y, z, dynamic_pointer_cast<Player>(shared_from_this()));
		containerMenu->containerId = containerCounter;
		containerMenu->addSlotListener(this);
	}
	else
	{
		app.DebugPrintf("ServerPlayer tried to open enchanting container when one was already open\n");
	}

	return true;
}

bool ServerPlayer::openContainer(shared_ptr<Container> container)
{
	if(containerMenu == inventoryMenu)
	{
		nextContainerCounter();

		// 4J-JEV: Added to distinguish between ender, bonus, large and small chests (for displaying the name of the chest).
		int containerType = container->getContainerType();
		assert(containerType >= 0);

		connection->send(std::make_shared<ContainerOpenPacket>(containerCounter, containerType, container->getCustomName(), container->getContainerSize(), container->hasCustomName()));

		containerMenu = new ContainerMenu(inventory, container);
		containerMenu->containerId = containerCounter;
		containerMenu->addSlotListener(this);
	}
	else
	{
		app.DebugPrintf("ServerPlayer tried to open container when one was already open\n");
	}

	return true;
}

bool ServerPlayer::openHopper(shared_ptr<HopperTileEntity> container)
{
	if(containerMenu == inventoryMenu)
	{
		nextContainerCounter();
		connection->send(std::make_shared<ContainerOpenPacket>(containerCounter, ContainerOpenPacket::HOPPER, container->getCustomName(), container->getContainerSize(), container->hasCustomName()));
		containerMenu = new HopperMenu(inventory, container);
		containerMenu->containerId = containerCounter;
		containerMenu->addSlotListener(this);
	}
	else
	{
		app.DebugPrintf("ServerPlayer tried to open hopper container when one was already open\n");
	}

	return true;
}

bool ServerPlayer::openHopper(shared_ptr<MinecartHopper> container)
{
	if(containerMenu == inventoryMenu)
	{
		nextContainerCounter();
		connection->send(std::make_shared<ContainerOpenPacket>(containerCounter, ContainerOpenPacket::HOPPER, container->getCustomName(), container->getContainerSize(), container->hasCustomName()));
		containerMenu = new HopperMenu(inventory, container);
		containerMenu->containerId = containerCounter;
		containerMenu->addSlotListener(this);
	}
	else
	{
		app.DebugPrintf("ServerPlayer tried to open minecart hopper container when one was already open\n");
	}

	return true;
}

bool ServerPlayer::openFurnace(shared_ptr<FurnaceTileEntity> furnace)
{
	if(containerMenu == inventoryMenu)
	{
		nextContainerCounter();
		connection->send(std::make_shared<ContainerOpenPacket>(containerCounter, ContainerOpenPacket::FURNACE, furnace->getCustomName(), furnace->getContainerSize(), furnace->hasCustomName()));
		containerMenu = new FurnaceMenu(inventory, furnace);
		containerMenu->containerId = containerCounter;
		containerMenu->addSlotListener(this);
	}
	else
	{
		app.DebugPrintf("ServerPlayer tried to open furnace when one was already open\n");
	}

	return true;
}

bool ServerPlayer::openTrap(shared_ptr<DispenserTileEntity> trap)
{
	if(containerMenu == inventoryMenu)
	{
		nextContainerCounter();
		connection->send(std::make_shared<ContainerOpenPacket>(containerCounter, trap->GetType() == eTYPE_DROPPERTILEENTITY ? ContainerOpenPacket::DROPPER : ContainerOpenPacket::TRAP, trap->getCustomName(), trap->getContainerSize(), trap->hasCustomName()));
		containerMenu = new TrapMenu(inventory, trap);
		containerMenu->containerId = containerCounter;
		containerMenu->addSlotListener(this);
	}
	else
	{
		app.DebugPrintf("ServerPlayer tried to open dispenser when one was already open\n");
	}

	return true;
}

bool ServerPlayer::openBrewingStand(shared_ptr<BrewingStandTileEntity> brewingStand)
{
	if(containerMenu == inventoryMenu)
	{
		nextContainerCounter();
		connection->send(std::make_shared<ContainerOpenPacket>(containerCounter, ContainerOpenPacket::BREWING_STAND, brewingStand->getCustomName(), brewingStand->getContainerSize(), brewingStand->hasCustomName()));
		containerMenu = new BrewingStandMenu(inventory, brewingStand);
		containerMenu->containerId = containerCounter;
		containerMenu->addSlotListener(this);
	}
	else
	{
		app.DebugPrintf("ServerPlayer tried to open brewing stand when one was already open\n");
	}

	return true;
}

bool ServerPlayer::openBeacon(shared_ptr<BeaconTileEntity> beacon)
{
	if(containerMenu == inventoryMenu)
	{
		nextContainerCounter();
		connection->send(std::make_shared<ContainerOpenPacket>(containerCounter, ContainerOpenPacket::BEACON, beacon->getCustomName(), beacon->getContainerSize(), beacon->hasCustomName()));
		containerMenu = new BeaconMenu(inventory, beacon);
		containerMenu->containerId = containerCounter;
		containerMenu->addSlotListener(this);
	}
	else
	{
		app.DebugPrintf("ServerPlayer tried to open beacon when one was already open\n");
	}

	return true;
}

bool ServerPlayer::openTrading(shared_ptr<Merchant> traderTarget, const wstring &name)
{
	if(containerMenu == inventoryMenu)
	{
		nextContainerCounter();
		containerMenu = new MerchantMenu(inventory, traderTarget, level);
		containerMenu->containerId = containerCounter;
		containerMenu->addSlotListener(this);
		shared_ptr<Container> container = static_cast<MerchantMenu *>(containerMenu)->getTradeContainer();

		connection->send(std::make_shared<ContainerOpenPacket>(containerCounter, ContainerOpenPacket::TRADER_NPC, name.empty() ? L"" : name, container->getContainerSize(), !name.empty()));

		MerchantRecipeList *offers = traderTarget->getOffers(dynamic_pointer_cast<Player>(shared_from_this()));
		if (offers != nullptr)
		{
			ByteArrayOutputStream rawOutput;
			DataOutputStream output(&rawOutput);

			// just to make sure the offers are matched to the container
			output.writeInt(containerCounter);
			offers->writeToStream(&output);

			connection->send(std::make_shared<CustomPayloadPacket>(CustomPayloadPacket::TRADER_LIST_PACKET, rawOutput.toByteArray()));
		}
	}
	else
	{
		app.DebugPrintf("ServerPlayer tried to open trading menu when one was already open\n");
	}

	return true;
}

bool ServerPlayer::openHorseInventory(shared_ptr<EntityHorse> horse, shared_ptr<Container> container)
{
	if (containerMenu != inventoryMenu)
	{
		closeContainer();
	}
	nextContainerCounter();
	connection->send(std::make_shared<ContainerOpenPacket>(containerCounter, ContainerOpenPacket::HORSE, horse->getCustomName(), container->getContainerSize(), container->hasCustomName(), horse->entityId));
	containerMenu = new HorseInventoryMenu(inventory, container, horse);
	containerMenu->containerId = containerCounter;
	containerMenu->addSlotListener(this);

	return true;
}

void ServerPlayer::slotChanged(AbstractContainerMenu *container, int slotIndex, shared_ptr<ItemInstance> item)
{
	if (dynamic_cast<ResultSlot *>(container->getSlot(slotIndex)))
	{
		return;
	}

	if (ignoreSlotUpdateHack)
	{
		// Do not send this packet!
		//
		// This is a horrible hack that makes sure that inventory clicks
		// that the client correctly predicted don't get sent out to the
		// client again.
		return;
	}

	connection->send(std::make_shared<ContainerSetSlotPacket>(container->containerId, slotIndex, item));

}

void ServerPlayer::refreshContainer(AbstractContainerMenu *menu)
{
	vector<shared_ptr<ItemInstance> > *items = menu->getItems();
	refreshContainer(menu, items);
	delete items;
}

void ServerPlayer::refreshContainer(AbstractContainerMenu *container, vector<shared_ptr<ItemInstance> > *items)
{
	connection->send(std::make_shared<ContainerSetContentPacket>(container->containerId, items));
	connection->send(std::make_shared<ContainerSetSlotPacket>(-1, -1, inventory->getCarried()));
}

void ServerPlayer::setContainerData(AbstractContainerMenu *container, int id, int value)
{
	// 4J - added, so that furnace updates also have this hack
	if (ignoreSlotUpdateHack)
	{
		// Do not send this packet!
		//
		// This is a horrible hack that makes sure that inventory clicks
		// that the client correctly predicted don't get sent out to the
		// client again.
		return;
	}
	connection->send(std::make_shared<ContainerSetDataPacket>(container->containerId, id, value));
}

void ServerPlayer::closeContainer()
{
	connection->send(std::make_shared<ContainerClosePacket>(containerMenu->containerId));
	doCloseContainer();
}

void ServerPlayer::broadcastCarriedItem()
{
	if (ignoreSlotUpdateHack)
	{
		// Do not send this packet!
		// This is a horrible hack that makes sure that inventory clicks
		// that the client correctly predicted don't get sent out to the
		// client again.
		return;
	}
	connection->send(std::make_shared<ContainerSetSlotPacket>(-1, -1, inventory->getCarried()));
}

void ServerPlayer::doCloseContainer()
{
	containerMenu->removed( dynamic_pointer_cast<Player>( shared_from_this() ) );
	containerMenu = inventoryMenu;
}

void ServerPlayer::setPlayerInput(float xxa, float yya, bool jumping, bool sneaking)
{
	if(riding != nullptr)
	{
		if (xxa >= -1 && xxa <= 1) this->xxa = xxa;
		if (yya >= -1 && yya <= 1) this->yya = yya;
		this->jumping = jumping;
		this->setSneaking(sneaking);
	}
}

void ServerPlayer::awardStat(Stat *stat, byteArray param)
{
	if (stat == nullptr)
	{
		delete [] param.data;
		return;
	}

	if (!stat->awardLocallyOnly)
	{
#ifndef _DURANGO
		int count = *((int*)param.data);
		delete [] param.data;

		connection->send(std::make_shared<AwardStatPacket>(stat->id, count));
#else
		connection->send( shared_ptr<AwardStatPacket>( new AwardStatPacket(stat->id, param) ) );
		// byteArray deleted in AwardStatPacket destructor.
#endif
	}
	else delete [] param.data;
}

void ServerPlayer::disconnect()
{
	if (rider.lock() != nullptr) rider.lock()->ride(shared_from_this() );
	if (m_isSleeping)
	{
		stopSleepInBed(true, false, false);
	}
}

void ServerPlayer::resetSentInfo()
{
	lastSentHealth = -99999999;
}

void ServerPlayer::displayClientMessage(int messageId)
{
	ChatPacket::EChatPacketMessage messageType = ChatPacket::e_ChatCustom;
	// Convert the message id to an enum that will not change between game versions
	switch(messageId)
	{
	case IDS_TILE_BED_OCCUPIED:
		messageType = ChatPacket::e_ChatBedOccupied;
		connection->send(std::make_shared<ChatPacket>(L"", messageType));
		break;
	case IDS_TILE_BED_NO_SLEEP:
		messageType = ChatPacket::e_ChatBedNoSleep;
		connection->send(std::make_shared<ChatPacket>(L"", messageType));
		break;
	case IDS_TILE_BED_NOT_VALID:
		messageType = ChatPacket::e_ChatBedNotValid;
		connection->send(std::make_shared<ChatPacket>(L"", messageType));
		break;
	case IDS_TILE_BED_NOTSAFE:
		messageType = ChatPacket::e_ChatBedNotSafe;
		connection->send(std::make_shared<ChatPacket>(L"", messageType));
		break;
	case IDS_TILE_BED_PLAYERSLEEP:
		messageType = ChatPacket::e_ChatBedPlayerSleep;
		// broadcast to all the other players in the game
		for (unsigned int i = 0; i < server->getPlayers()->players.size(); i++)
		{
			shared_ptr<ServerPlayer> player = server->getPlayers()->players[i];
			if(shared_from_this()!=player)
			{
				player->connection->send(std::make_shared<ChatPacket>(name, ChatPacket::e_ChatBedPlayerSleep));
			}
			else
			{
				player->connection->send(std::make_shared<ChatPacket>(name, ChatPacket::e_ChatBedMeSleep));
			}
		}
		return;
		break;
	case IDS_PLAYER_ENTERED_END:
		for (unsigned int i = 0; i < server->getPlayers()->players.size(); i++)
		{
			shared_ptr<ServerPlayer> player = server->getPlayers()->players[i];
			if(shared_from_this()!=player)
			{
				player->connection->send(std::make_shared<ChatPacket>(name, ChatPacket::e_ChatPlayerEnteredEnd));
			}
		}
		break;
	case IDS_PLAYER_LEFT_END:
		for (unsigned int i = 0; i < server->getPlayers()->players.size(); i++)
		{
			shared_ptr<ServerPlayer> player = server->getPlayers()->players[i];
			if(shared_from_this()!=player)
			{
				player->connection->send(std::make_shared<ChatPacket>(name, ChatPacket::e_ChatPlayerLeftEnd));
			}
		}
		break;
	case IDS_TILE_BED_MESLEEP:
		messageType = ChatPacket::e_ChatBedMeSleep;
		connection->send(std::make_shared<ChatPacket>(L"", messageType));
		break;

	case IDS_MAX_PIGS_SHEEP_COWS_CATS_SPAWNED:
		for (unsigned int i = 0; i < server->getPlayers()->players.size(); i++)
		{
			shared_ptr<ServerPlayer> player = server->getPlayers()->players[i];
			if(shared_from_this()==player)
			{
				player->connection->send(std::make_shared<ChatPacket>(name, ChatPacket::e_ChatPlayerMaxPigsSheepCows));
			}
		}
		break;
	case IDS_MAX_CHICKENS_SPAWNED:
		for (unsigned int i = 0; i < server->getPlayers()->players.size(); i++)
		{
			shared_ptr<ServerPlayer> player = server->getPlayers()->players[i];
			if(shared_from_this()==player)
			{
				player->connection->send(std::make_shared<ChatPacket>(name, ChatPacket::e_ChatPlayerMaxChickens));
			}
		}
		break;
	case IDS_MAX_SQUID_SPAWNED:
		for (unsigned int i = 0; i < server->getPlayers()->players.size(); i++)
		{
			shared_ptr<ServerPlayer> player = server->getPlayers()->players[i];
			if(shared_from_this()==player)
			{
				player->connection->send(std::make_shared<ChatPacket>(name, ChatPacket::e_ChatPlayerMaxSquid));
			}
		}
		break;
	case IDS_MAX_BATS_SPAWNED:
		for (unsigned int i = 0; i < server->getPlayers()->players.size(); i++)
		{
			shared_ptr<ServerPlayer> player = server->getPlayers()->players[i];
			if(shared_from_this()==player)
			{
				player->connection->send(std::make_shared<ChatPacket>(name, ChatPacket::e_ChatPlayerMaxBats));
			}
		}
		break;
	case IDS_MAX_WOLVES_SPAWNED:
		for (unsigned int i = 0; i < server->getPlayers()->players.size(); i++)
		{
			shared_ptr<ServerPlayer> player = server->getPlayers()->players[i];
			if(shared_from_this()==player)
			{
				player->connection->send(std::make_shared<ChatPacket>(name, ChatPacket::e_ChatPlayerMaxWolves));
			}
		}
		break;
	case IDS_MAX_MOOSHROOMS_SPAWNED:
		for (unsigned int i = 0; i < server->getPlayers()->players.size(); i++)
		{
			shared_ptr<ServerPlayer> player = server->getPlayers()->players[i];
			if(shared_from_this()==player)
			{
				player->connection->send(std::make_shared<ChatPacket>(name, ChatPacket::e_ChatPlayerMaxMooshrooms));
			}
		}
		break;
	case IDS_MAX_ENEMIES_SPAWNED:
		for (unsigned int i = 0; i < server->getPlayers()->players.size(); i++)
		{
			shared_ptr<ServerPlayer> player = server->getPlayers()->players[i];
			if(shared_from_this()==player)
			{
				player->connection->send(std::make_shared<ChatPacket>(name, ChatPacket::e_ChatPlayerMaxEnemies));
			}
		}
		break;

	case IDS_MAX_VILLAGERS_SPAWNED:
		for (unsigned int i = 0; i < server->getPlayers()->players.size(); i++)
		{
			shared_ptr<ServerPlayer> player = server->getPlayers()->players[i];
			if(shared_from_this()==player)
			{
				player->connection->send(std::make_shared<ChatPacket>(name, ChatPacket::e_ChatPlayerMaxVillagers));
			}
		}
		break;
	case IDS_MAX_PIGS_SHEEP_COWS_CATS_BRED:
		for (unsigned int i = 0; i < server->getPlayers()->players.size(); i++)
		{
			shared_ptr<ServerPlayer> player = server->getPlayers()->players[i];
			if(shared_from_this()==player)
			{
				player->connection->send(std::make_shared<ChatPacket>(name, ChatPacket::e_ChatPlayerMaxBredPigsSheepCows));
			}
		}
		break;
	case IDS_MAX_CHICKENS_BRED:
		for (unsigned int i = 0; i < server->getPlayers()->players.size(); i++)
		{
			shared_ptr<ServerPlayer> player = server->getPlayers()->players[i];
			if(shared_from_this()==player)
			{
				player->connection->send(std::make_shared<ChatPacket>(name, ChatPacket::e_ChatPlayerMaxBredChickens));
			}
		}
		break;
	case IDS_MAX_MUSHROOMCOWS_BRED:
		for (unsigned int i = 0; i < server->getPlayers()->players.size(); i++)
		{
			shared_ptr<ServerPlayer> player = server->getPlayers()->players[i];
			if(shared_from_this()==player)
			{
				player->connection->send(std::make_shared<ChatPacket>(name, ChatPacket::e_ChatPlayerMaxBredMooshrooms));
			}
		}
		break;

	case IDS_MAX_WOLVES_BRED:
		for (unsigned int i = 0; i < server->getPlayers()->players.size(); i++)
		{
			shared_ptr<ServerPlayer> player = server->getPlayers()->players[i];
			if(shared_from_this()==player)
			{
				player->connection->send(std::make_shared<ChatPacket>(name, ChatPacket::e_ChatPlayerMaxBredWolves));
			}
		}
		break;

	case IDS_CANT_SHEAR_MOOSHROOM:
		for (unsigned int i = 0; i < server->getPlayers()->players.size(); i++)
		{
			shared_ptr<ServerPlayer> player = server->getPlayers()->players[i];
			if(shared_from_this()==player)
			{
				player->connection->send(std::make_shared<ChatPacket>(name, ChatPacket::e_ChatPlayerCantShearMooshroom));
			}
		}
		break;


	case IDS_MAX_HANGINGENTITIES:
		for (unsigned int i = 0; i < server->getPlayers()->players.size(); i++)
		{
			shared_ptr<ServerPlayer> player = server->getPlayers()->players[i];
			if(shared_from_this()==player)
			{
				player->connection->send(std::make_shared<ChatPacket>(name, ChatPacket::e_ChatPlayerMaxHangingEntities));
			}
		}
		break;
	case IDS_CANT_SPAWN_IN_PEACEFUL:
		for (unsigned int i = 0; i < server->getPlayers()->players.size(); i++)
		{
			shared_ptr<ServerPlayer> player = server->getPlayers()->players[i];
			if(shared_from_this()==player)
			{
				player->connection->send(std::make_shared<ChatPacket>(name, ChatPacket::e_ChatPlayerCantSpawnInPeaceful));
			}
		}
		break;

	case IDS_MAX_BOATS:
		for (unsigned int i = 0; i < server->getPlayers()->players.size(); i++)
		{
			shared_ptr<ServerPlayer> player = server->getPlayers()->players[i];
			if(shared_from_this()==player)
			{
				player->connection->send(std::make_shared<ChatPacket>(name, ChatPacket::e_ChatPlayerMaxBoats));
			}
		}
		break;

	default:
		app.DebugPrintf("Tried to send a chat packet to the player with an unhandled messageId\n");
		assert( false );
		break;
	}

	//Language *language = Language::getInstance();
	//wstring languageString = app.GetString(messageId);//language->getElement(messageId);
	//connection->send( shared_ptr<ChatPacket>( new ChatPacket(L"", messageType) ) );
}

void ServerPlayer::completeUsingItem()
{
	connection->send(std::make_shared<EntityEventPacket>(entityId, EntityEvent::USE_ITEM_COMPLETE));
	Player::completeUsingItem();
}

void ServerPlayer::startUsingItem(shared_ptr<ItemInstance> instance, int duration)
{
	Player::startUsingItem(instance, duration);

	if (instance != nullptr && instance->getItem() != nullptr && instance->getItem()->getUseAnimation(instance) == UseAnim_eat)
	{
		getLevel()->getTracker()->broadcastAndSend(shared_from_this(), std::make_shared<AnimatePacket>(shared_from_this(), AnimatePacket::EAT));
	}
}

void ServerPlayer::restoreFrom(shared_ptr<Player> oldPlayer, bool restoreAll)
{
	Player::restoreFrom(oldPlayer, restoreAll);
	lastSentExp = -1;
	lastSentHealth = -1;
	lastSentFood = -1;
	entitiesToRemove = dynamic_pointer_cast<ServerPlayer>(oldPlayer)->entitiesToRemove;
}

void ServerPlayer::onEffectAdded(MobEffectInstance *effect)
{
	Player::onEffectAdded(effect);
	connection->send(std::make_shared<UpdateMobEffectPacket>(entityId, effect));
}


void ServerPlayer::onEffectUpdated(MobEffectInstance *effect, bool doRefreshAttributes)
{
	Player::onEffectUpdated(effect, doRefreshAttributes);
	connection->send(std::make_shared<UpdateMobEffectPacket>(entityId, effect));
}


void ServerPlayer::onEffectRemoved(MobEffectInstance *effect)
{
	Player::onEffectRemoved(effect);
	connection->send(std::make_shared<RemoveMobEffectPacket>(entityId, effect));
}

void ServerPlayer::teleportTo(double x, double y, double z)
{
	connection->teleport(x, y, z, yRot, xRot);
}

void ServerPlayer::crit(shared_ptr<Entity> entity)
{
	getLevel()->getTracker()->broadcastAndSend(shared_from_this(), std::make_shared<AnimatePacket>(entity, AnimatePacket::CRITICAL_HIT));
}

void ServerPlayer::magicCrit(shared_ptr<Entity> entity)
{
	getLevel()->getTracker()->broadcastAndSend(shared_from_this(), std::make_shared<AnimatePacket>(entity, AnimatePacket::MAGIC_CRITICAL_HIT));
}

void ServerPlayer::onUpdateAbilities()
{
	if (connection == nullptr) return;
	connection->send(std::make_shared<PlayerAbilitiesPacket>(&abilities));
}

ServerLevel *ServerPlayer::getLevel()
{
	return static_cast<ServerLevel *>(level);
}

void ServerPlayer::setGameMode(GameType *mode)
{
	gameMode->setGameModeForPlayer(mode);
	connection->send(std::make_shared<GameEventPacket>(GameEventPacket::CHANGE_GAME_MODE, mode->getId()));
}

void ServerPlayer::sendMessage(const wstring& message, ChatPacket::EChatPacketMessage type /*= e_ChatCustom*/, int customData /*= -1*/, const wstring& additionalMessage /*= L""*/)
{
	connection->send(std::make_shared<ChatPacket>(message, type, customData, additionalMessage));
}

bool ServerPlayer::hasPermission(EGameCommand command)
{
	return server->getPlayers()->isOp(dynamic_pointer_cast<ServerPlayer>(shared_from_this()));

	// 4J: Removed permission level
	/*if( server->getPlayers()->isOp(dynamic_pointer_cast<ServerPlayer>(shared_from_this())) )
	{
		return server->getOperatorUserPermissionLevel() >= permissionLevel;
	}
	return false;*/
}

// 4J - Don't use
//void ServerPlayer::updateOptions(shared_ptr<ClientInformationPacket> packet)
//{
//	// 4J - Don't need
//	//if (language.getLanguageList().containsKey(packet.getLanguage()))
//	//{
//	//	language.loadLanguage(packet->getLanguage());
//	//}
//
//	int dist = 16 * 16 >> packet->getViewDistance();
//	if (dist > PlayerChunkMap::MIN_VIEW_DISTANCE && dist < PlayerChunkMap::MAX_VIEW_DISTANCE)
//	{
//		this->viewDistance = dist;
//	}
//
//	chatVisibility = packet->getChatVisibility();
//	canChatColor = packet->getChatColors();
//
//	// 4J - Don't need
//	//if (server.isSingleplayer() && server.getSingleplayerName().equals(name))
//	//{
//	//	server.setDifficulty(packet.getDifficulty());
//	//}
//}

int ServerPlayer::getViewDistance()
{
	return viewDistance;
}

//bool ServerPlayer::canChatInColor()
//{
//	return canChatColor;
//}
//
//int ServerPlayer::getChatVisibility()
//{
//	return chatVisibility;
//}

Pos *ServerPlayer::getCommandSenderWorldPosition()
{
	return new Pos(Mth::floor(x), Mth::floor(y + .5), Mth::floor(z));
}

void ServerPlayer::resetLastActionTime()
{
	this->lastActionTime = MinecraftServer::getCurrentTimeMillis();
}

// Get an index that can be used to uniquely reference this chunk from either dimension
int ServerPlayer::getFlagIndexForChunk(const ChunkPos& pos, int dimension)
{
	// Scale pos x & z up by 16 as getGlobalIndexForChunk is expecting tile rather than chunk coords
	return LevelRenderer::getGlobalIndexForChunk(pos.x * 16 , 0, pos.z * 16, dimension ) / (Level::maxBuildHeight / 16);		// dividing here by number of renderer chunks in one column;
}

// 4J Added, returns a number which is subtracted from the default view distance
int ServerPlayer::getPlayerViewDistanceModifier()
{
	int value = 0;

	if( !connection->isLocal() )
	{
		INetworkPlayer *player = connection->getNetworkPlayer();

		if( player != nullptr )
		{
			DWORD rtt = player->GetCurrentRtt();

			value = rtt >> 6;

			if(value > 4) value = 4;
		}
	}

	return value;
}

void ServerPlayer::handleCollectItem(shared_ptr<ItemInstance> item)
{
	if(gameMode->getGameRules() != nullptr) gameMode->getGameRules()->onCollectItem(item);
}

#ifndef _CONTENT_PACKAGE
void ServerPlayer::debug_setPosition(double x, double y, double z, double nYRot, double nXRot)
{
	connection->teleport(x, y, z, nYRot, nXRot);
}
#endif
