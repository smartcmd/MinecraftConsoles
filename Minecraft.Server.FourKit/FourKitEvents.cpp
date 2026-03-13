#include "FourKitInternal.h"

void FourKit::FireEventOnLoad()
{
    try
    {
        ServerLoadEvent ^ event = gcnew ServerLoadEvent();
        EventManager::FireEvent(event);

        for each (ServerPlugin ^ plugin in pluginList)
        {
            try
            {
                EventManager::SetCurrentPlugin(plugin->GetName());

                plugin->OnEnable();

                EventManager::ClearCurrentPlugin();
            }
            catch (Exception ^ ex)
            {
                PluginLogger::LogError("fourkit",
                    String::Format("Error in OnEnable for plugin {0}: {1}",
                        plugin->GetName(), ex->Message));
                EventManager::ClearCurrentPlugin();
            }
        }
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnLoad event: {0}", ex->Message));
    }
}

void FourKit::FireEventOnExit()
{
    try
    {
        ServerShutdownEvent ^ event = gcnew ServerShutdownEvent();
        EventManager::FireEvent(event);

        for each (ServerPlugin ^ plugin in pluginList)
        {
            try
            {
                plugin->OnDisable();
            }
            catch (Exception ^ ex)
            {
                PluginLogger::LogError("fourkit",
                    String::Format("Error in OnDisable for plugin {0}: {1}",
                        plugin->GetName(), ex->Message));
            }
        }
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnExit event: {0}", ex->Message));
    }
}

void FourKit::FireEventOnPlayerJoin(const PlayerJoinData &playerData)
{
    try
    {
        String ^ name = gcnew String(playerData.playerName);
        if (!String::IsNullOrEmpty(name))
        {
            onlinePlayerNames->Add(name);
        }

        Player ^ player = gcnew Player(name);
        player->SetPlayerData(playerData.health, playerData.food, playerData.fallDistance,
                              playerData.yRot, playerData.xRot,
                              playerData.sneaking, playerData.sprinting, playerData.insideVehicle,
                              playerData.x, playerData.y, playerData.z, playerData.dimension);

        PlayerJoinEvent ^ event = gcnew PlayerJoinEvent();
        event->PlayerObject = player;
        EventManager::FireEvent(event);
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnPlayerJoin event: {0}", ex->Message));
    }
}

void FourKit::FireEventOnPlayerLeave(const PlayerLeaveData &playerData)
{
    try
    {
        String ^ name = gcnew String((playerData.playerName != nullptr) ? playerData.playerName : "");

        Player ^ player = FourKitInternal::ResolvePlayerByName(name);
        if (player == nullptr)
        {
            player = gcnew Player(name);
        }

        PlayerLeaveEvent ^ event = gcnew PlayerLeaveEvent();
        event->PlayerObject = player;
        EventManager::FireEvent(event);

        if (!String::IsNullOrEmpty(name))
        {
            onlinePlayerNames->Remove(name);
        }
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnPlayerLeave event: {0}", ex->Message));
    }
}

void FourKit::FireEventOnPlayerDeath(PlayerDeathData* deathData)
{
    try
    {
        if (deathData == nullptr) return;

        String^ name = gcnew String((deathData->playerName != nullptr) ? deathData->playerName : "");

        Player^ player = FourKitInternal::ResolvePlayerByName(name);
        if (player == nullptr)
        {
            player = gcnew Player(name);
        }

        PlayerDeathEvent^ event = gcnew PlayerDeathEvent();
        event->PlayerObject = player;
        event->DeathMessage = gcnew String(deathData->deathMessage);
        event->KeepInventory = deathData->keepInventory;
        event->KeepLevel = deathData->keepLevel;
        event->NewExp = deathData->newExp;
        event->NewLevel = deathData->newLevel;
        event->NewTotalExp = deathData->newTotalExp;

        EventManager::FireEvent(event);

        deathData->keepInventory = event->KeepInventory;
        deathData->keepLevel = event->KeepLevel;
        deathData->newExp = event->NewExp;
        deathData->newLevel = event->NewLevel;
        deathData->newTotalExp = event->NewTotalExp;

        if (event->DeathMessage != nullptr)
        {
            IntPtr ptr = Marshal::StringToHGlobalAnsi(event->DeathMessage);
            try
            {
                strncpy_s(deathData->deathMessage, sizeof(deathData->deathMessage),
                    (const char*)ptr.ToPointer(), _TRUNCATE);
            }
            finally
            {
                Marshal::FreeHGlobal(ptr);
            }
        }
    }
    catch (Exception^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnPlayerDeath event: {0}", ex->Message));
    }
}

void FourKit::FireEventOnPlayerChat(const PlayerChatData &chatData, bool *cancelled)
{
    try
    {
        String ^ name = gcnew String((chatData.playerName != nullptr) ? chatData.playerName : "");
        String ^ message = gcnew String((chatData.message != nullptr) ? chatData.message : "");

        Player ^ player = FourKitInternal::ResolvePlayerByName(name);
        if (player == nullptr)
        {
            player = gcnew Player(name);
        }

        PlayerChatEvent ^ event = gcnew PlayerChatEvent();
        event->PlayerObject = player;
        event->Message = message;
        event->Cancelled = false;

        EventManager::FireEvent(event);

        if (cancelled != nullptr)
        {
            *cancelled = event->Cancelled;
        }
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnPlayerChat event: {0}", ex->Message));
        if (cancelled != nullptr)
        {
            *cancelled = false;
        }
    }
}

void FourKit::FireEventOnBlockBreak(const BlockBreakData &blockBreakData, bool *cancelled)
{
    try
    {
        String ^ name = gcnew String((blockBreakData.playerName != nullptr) ? blockBreakData.playerName : "");

        Player ^ player = FourKitInternal::ResolvePlayerByName(name);
        if (player == nullptr)
        {
            player = gcnew Player(name);
        }

        Block ^ block = gcnew Block(
            blockBreakData.x,
            blockBreakData.y,
            blockBreakData.z,
            0,
            blockBreakData.blockId,
            blockBreakData.blockData);

        BlockBreakEvent ^ event = gcnew BlockBreakEvent();
        event->PlayerObject = player;
        event->BlockObject = block;
        event->Cancelled = false;

        EventManager::FireEvent(event);

        if (cancelled != nullptr)
        {
            *cancelled = event->Cancelled;
        }
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnBlockBreak event: {0}", ex->Message));
        if (cancelled != nullptr)
        {
            *cancelled = false;
        }
    }
}

void FourKit::FireEventOnBlockPlace(const BlockPlaceData &blockPlaceData, bool *cancelled)
{
    try
    {
        String ^ name = gcnew String((blockPlaceData.playerName != nullptr) ? blockPlaceData.playerName : "");

        Player ^ player = FourKitInternal::ResolvePlayerByName(name);
        if (player == nullptr)
        {
            player = gcnew Player(name);
        }

        Block ^ block = gcnew Block(
            blockPlaceData.x,
            blockPlaceData.y,
            blockPlaceData.z,
            0,
            blockPlaceData.blockId,
            blockPlaceData.blockData);

        BlockPlaceEvent ^ event = gcnew BlockPlaceEvent();
        event->PlayerObject = player;
        event->BlockObject = block;
        event->Cancelled = false;

        EventManager::FireEvent(event);

        if (cancelled != nullptr)
        {
            *cancelled = event->Cancelled;
        }
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnBlockPlace event: {0}", ex->Message));
        if (cancelled != nullptr)
        {
            *cancelled = false;
        }
    }
}

void FourKit::FireEventOnPlayerMove(const PlayerMoveData &moveData, bool *cancelled)
{
    try
    {
        String ^ name = gcnew String((moveData.playerName != nullptr) ? moveData.playerName : "");

        Player ^ player = FourKitInternal::ResolvePlayerByName(name);
        if (player == nullptr)
        {
            player = gcnew Player(name);
        }

        PlayerMoveEvent^ event = gcnew PlayerMoveEvent();
        event->PlayerObject = player;
        event->From = gcnew Location(moveData.fromX, moveData.fromY, moveData.fromZ);
        event->To = gcnew Location(moveData.toX, moveData.toY, moveData.toZ);
        event->Cancelled = false;

        EventManager::FireEvent(event);

        if (cancelled != nullptr)
        {
            *cancelled = event->Cancelled;
        }
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnPlayerMove event: {0}", ex->Message));
        if (cancelled != nullptr)
        {
            *cancelled = false;
        }
    }
}

void FourKit::FireEventOnPlayerPortal(PlayerPortalData *portalData, bool *cancelled)
{
    try
    {
        if (portalData == nullptr)
        {
            if (cancelled != nullptr)
            {
                *cancelled = false;
            }
            return;
        }

        String ^ name = gcnew String((portalData->playerName != nullptr) ? portalData->playerName : "");

        Player ^ player = FourKitInternal::ResolvePlayerByName(name);
        if (player == nullptr)
        {
            player = gcnew Player(name);
        }

        PlayerPortalEvent^ event = gcnew PlayerPortalEvent();
        event->PlayerObject = player;

        TeleportCause cause = TeleportCause::UNKNOWN;
        if (portalData->cause == (int)TeleportCause::END_PORTAL) cause = TeleportCause::END_PORTAL;
        else if (portalData->cause == (int)TeleportCause::ENDER_PEARL) cause = TeleportCause::ENDER_PEARL;
        else if (portalData->cause == (int)TeleportCause::NETHER_PORTAL) cause = TeleportCause::NETHER_PORTAL;
        else if (portalData->cause == (int)TeleportCause::PLUGIN) cause = TeleportCause::PLUGIN;

        event->Cause = cause;
        event->From = gcnew Location(portalData->fromX, portalData->fromY, portalData->fromZ);
        event->To = gcnew Location(portalData->toX, portalData->toY, portalData->toZ);
        event->Cancelled = false;

        EventManager::FireEvent(event);

        if (event->From != nullptr)
        {
            portalData->fromX = event->From->getX();
            portalData->fromY = event->From->getY();
            portalData->fromZ = event->From->getZ();
        }
        if (event->To != nullptr)
        {
            portalData->toX = event->To->getX();
            portalData->toY = event->To->getY();
            portalData->toZ = event->To->getZ();
        }

        if (cancelled != nullptr)
        {
            *cancelled = event->Cancelled;
        }
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnPlayerPortal event: {0}", ex->Message));
        if (cancelled != nullptr)
        {
            *cancelled = false;
        }
    }
}

void FourKit::FireEventOnSignChange(SignChangeData *signData, bool *cancelled)
{
    try
    {
        if (signData == nullptr)
        {
            if (cancelled != nullptr)
            {
                *cancelled = false;
            }
            return;
        }

        String ^ name = gcnew String((signData->playerName != nullptr) ? signData->playerName : "");

        Player ^ player = FourKitInternal::ResolvePlayerByName(name);
        if (player == nullptr)
        {
            player = gcnew Player(name);
        }

        SignChangeEvent^ event = gcnew SignChangeEvent();
        event->PlayerObject = player;
        event->BlockObject = gcnew Block(
            signData->x,
            signData->y,
            signData->z,
            signData->dimension,
            NativeBlockCallbacks::GetBlockType(signData->x, signData->y, signData->z, signData->dimension),
            NativeBlockCallbacks::GetBlockData(signData->x, signData->y, signData->z, signData->dimension));

        event->Lines = gcnew cli::array<String^>(4);
        for (int i = 0; i < 4; ++i)
        {
            event->Lines[i] = gcnew String(signData->lines[i]);
        }
        event->Cancelled = false;

        EventManager::FireEvent(event);

        if (event->Lines != nullptr)
        {
            for (int i = 0; i < 4; ++i)
            {
                String^ managedLine = (i < event->Lines->Length && event->Lines[i] != nullptr) ? event->Lines[i] : String::Empty;
                IntPtr linePtr = Marshal::StringToHGlobalAnsi(managedLine);
                try
                {
                    const char* lineChars = (const char*)linePtr.ToPointer();
                    strncpy_s(signData->lines[i], sizeof(signData->lines[i]), lineChars, _TRUNCATE);
                }
                finally
                {
                    Marshal::FreeHGlobal(linePtr);
                }
            }
        }

        if (cancelled != nullptr)
        {
            *cancelled = event->Cancelled;
        }
    }
    catch (Exception ^ ex)
    {
        PluginLogger::LogError("fourkit", String::Format("Error firing OnSignChange event: {0}", ex->Message));
        if (cancelled != nullptr)
        {
            *cancelled = false;
        }
    }
}

void FourKit::FireEventOnPlayerInteract(PlayerInteractData *interactData, bool *cancelled)
{
	try
	{
		if (interactData == nullptr)
		{
			if (cancelled != nullptr)
			{
				*cancelled = false;
			}
			return;
		}

		String ^ name = gcnew String((interactData->playerName != nullptr) ? interactData->playerName : "");
		Player ^ player = FourKitInternal::ResolvePlayerByName(name);
		if (player == nullptr)
		{
			player = gcnew Player(name);
		}

		PlayerInteractEvent^ event = gcnew PlayerInteractEvent();
		event->PlayerObject = player;
		event->Action = FourKitInternal::ToInteractAction(interactData->action);
        event->Face = FourKitInternal::ToBlockFace(interactData->blockFace);
        event->HasBlock = interactData->hasBlock;
        event->HasItem = interactData->hasItem;
		event->Cancelled = false;

		if (interactData->hasBlock)
		{
			event->ClickedBlock = gcnew Block(
				interactData->x,
				interactData->y,
				interactData->z,
				interactData->dimension,
				interactData->blockId,
				interactData->blockData);
		}
		else
		{
			event->ClickedBlock = nullptr;
		}

		EventManager::FireEvent(event);

		if (cancelled != nullptr)
		{
			*cancelled = event->Cancelled;
		}
	}
	catch (Exception ^ ex)
	{
		PluginLogger::LogError("fourkit", String::Format("Error firing OnPlayerInteract event: {0}", ex->Message));
		if (cancelled != nullptr)
		{
			*cancelled = false;
		}
	}
}

void FourKit::FireEventOnPlayerDropItem(PlayerDropItemData* dropData, bool* cancelled)
{
	try
	{
		if (dropData == nullptr)
		{
			if (cancelled != nullptr)
			{
				*cancelled = false;
			}
			return;
		}

		String^ name = gcnew String((dropData->playerName != nullptr) ? dropData->playerName : "");

		Player^ player = FourKitInternal::ResolvePlayerByName(name);
		if (player == nullptr)
		{
			player = gcnew Player(name);
		}

		ItemStack^ itemStack = gcnew ItemStack(dropData->itemId, dropData->itemCount, dropData->itemData);
		Item^ itemDrop = gcnew Item(gcnew Location(player->getX(), player->getY(), player->getZ()), itemStack);
		itemDrop->setPickupDelay(dropData->pickupDelay);

		PlayerDropItemEvent^ event = gcnew PlayerDropItemEvent();
		event->PlayerObject = player;
		event->ItemDrop = itemDrop;
		event->Cancelled = false;

		EventManager::FireEvent(event);

		if (event->ItemDrop != nullptr)
		{
			if (event->ItemDrop->getItemStack() != nullptr)
			{
				ItemStack^ modifiedStack = event->ItemDrop->getItemStack();
				dropData->itemId = modifiedStack->getTypeId();
				dropData->itemCount = modifiedStack->getAmount();
				dropData->itemData = modifiedStack->getData();
			}
			dropData->pickupDelay = event->ItemDrop->getPickupDelay();
		}

		if (cancelled != nullptr)
		{
			*cancelled = event->Cancelled;
		}
	}
	catch (Exception^ ex)
	{
		PluginLogger::LogError("fourkit", String::Format("Error firing OnPlayerDropItem event: {0}", ex->Message));
		if (cancelled != nullptr)
		{
			*cancelled = false;
		}
	}
}

static DamageCause ToDamageCause(int cause)
{
	switch (cause)
	{
	case 0: return DamageCause::BLOCK_EXPLOSION;
	case 1: return DamageCause::CONTACT;
	case 2: return DamageCause::CUSTOM;
	case 3: return DamageCause::DROWNING;
	case 4: return DamageCause::ENTITY_ATTACK;
	case 5: return DamageCause::ENTITY_EXPLOSION;
	case 6: return DamageCause::FALL;
	case 7: return DamageCause::FALLING_BLOCK;
	case 8: return DamageCause::FIRE;
	case 9: return DamageCause::FIRE_TICK;
	case 10: return DamageCause::LAVA;
	case 11: return DamageCause::LIGHTNING;
	case 12: return DamageCause::MAGIC;
	case 13: return DamageCause::MELTING;
	case 14: return DamageCause::POISON;
	case 15: return DamageCause::PROJECTILE;
	case 16: return DamageCause::STARVATION;
	case 17: return DamageCause::SUFFOCATION;
	case 18: return DamageCause::SUICIDE;
	case 19: return DamageCause::THORNS;
	case 20: return DamageCause::VOID_DAMAGE;
	case 21: return DamageCause::WITHER;
	default: return DamageCause::CUSTOM;
	}
}

static EntityType ToEntityType(int type)
{
	return (EntityType)type;
}

static Entity^ BuildEntityFromDamageData(
	const char* name, int entityType, int entityId,
	double x, double y, double z, int dimension,
	bool isPlayer, float fallDistance, bool onGround)
{
	if (isPlayer)
	{
		String^ playerName = gcnew String((name != nullptr) ? name : "");
		Player^ player = FourKitInternal::ResolvePlayerByName(playerName);
		if (player == nullptr)
		{
			player = gcnew Player(playerName);
		}
		player->SetEntityData(entityId, EntityType::PLAYER, x, y, z, dimension, fallDistance, onGround, false);
		return player;
	}
	else
	{
		Entity^ entity = gcnew Entity(entityId, ToEntityType(entityType), x, y, z, dimension);
		entity->SetEntityData(entityId, ToEntityType(entityType), x, y, z, dimension, fallDistance, onGround, false);
		return entity;
	}
}

void FourKit::FireEventOnEntityDamage(EntityDamageData* damageData, bool* cancelled)
{
	try
	{
		if (damageData == nullptr)
		{
			if (cancelled != nullptr)
			{
				*cancelled = false;
			}
			return;
		}

		Entity^ entity = BuildEntityFromDamageData(
			damageData->entityName, damageData->entityType, damageData->entityId,
			damageData->entityX, damageData->entityY, damageData->entityZ,
			damageData->entityDimension, damageData->entityIsPlayer,
			damageData->entityFallDistance, damageData->entityOnGround);

		EntityDamageEvent^ event = gcnew EntityDamageEvent();
		event->EntityObject = entity;
		event->Cause = ToDamageCause(damageData->cause);
		event->Damage = damageData->damage;
		event->FinalDamage = damageData->finalDamage;
		event->OriginalDamage = damageData->damage;
		event->Cancelled = false;

		EventManager::FireEvent(event);

		damageData->damage = event->Damage;
		damageData->finalDamage = event->FinalDamage;

		if (cancelled != nullptr)
		{
			*cancelled = event->Cancelled;
		}
	}
	catch (Exception^ ex)
	{
		PluginLogger::LogError("fourkit", String::Format("Error firing OnEntityDamage event: {0}", ex->Message));
		if (cancelled != nullptr)
		{
			*cancelled = false;
		}
	}
}

void FourKit::FireEventOnEntityDamageByEntity(EntityDamageData* damageData, bool* cancelled)
{
	try
	{
		if (damageData == nullptr)
		{
			if (cancelled != nullptr)
			{
				*cancelled = false;
			}
			return;
		}

		Entity^ entity = BuildEntityFromDamageData(
			damageData->entityName, damageData->entityType, damageData->entityId,
			damageData->entityX, damageData->entityY, damageData->entityZ,
			damageData->entityDimension, damageData->entityIsPlayer,
			damageData->entityFallDistance, damageData->entityOnGround);

		Entity^ damager = BuildEntityFromDamageData(
			damageData->damagerName, damageData->damagerEntityType, damageData->damagerEntityId,
			damageData->damagerX, damageData->damagerY, damageData->damagerZ,
			damageData->damagerDimension, damageData->damagerIsPlayer,
			damageData->damagerFallDistance, damageData->damagerOnGround);

		EntityDamageByEntityEvent^ event = gcnew EntityDamageByEntityEvent();
		event->EntityObject = entity;
		event->DamagerObject = damager;
		event->Cause = ToDamageCause(damageData->cause);
		event->Damage = damageData->damage;
		event->FinalDamage = damageData->finalDamage;
		event->OriginalDamage = damageData->damage;
		event->Cancelled = false;

		EventManager::FireEvent(event);

		damageData->damage = event->Damage;
		damageData->finalDamage = event->FinalDamage;

		if (cancelled != nullptr)
		{
			*cancelled = event->Cancelled;
		}
	}
	catch (Exception^ ex)
	{
		PluginLogger::LogError("fourkit", String::Format("Error firing OnEntityDamageByEntity event: {0}", ex->Message));
		if (cancelled != nullptr)
		{
			*cancelled = false;
		}
	}
}
