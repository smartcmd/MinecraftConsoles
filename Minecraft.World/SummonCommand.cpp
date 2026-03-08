#include "stdafx.h"
#include "..\Minecraft.Client\MinecraftServer.h"
#include "..\Minecraft.Client\ServerPlayer.h"
#include "..\Minecraft.Client\ServerLevel.h"
#include "..\Minecraft.World\net.minecraft.commands.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "GameCommandPacket.h"
#include "EntityIO.h"
#include "SummonCommand.h"

EGameCommand SummonCommand::getId()
{
    return eGameCommand_Summon;
}

int SummonCommand::getPermissionLevel()
{
    return LEVEL_GAMEMASTERS;
}

void SummonCommand::execute(shared_ptr<CommandSender> source, byteArray commandData)
{
    ByteArrayInputStream bais(commandData);
    DataInputStream dis(&bais);

    wstring entityType = dis.readUTF();
    int x = dis.readInt();
    int y = dis.readInt();
    int z = dis.readInt();
    
    bais.reset();

    MinecraftServer* server = MinecraftServer::getInstance();
    ServerLevel* level = NULL;
    
    shared_ptr<ServerPlayer> player = dynamic_pointer_cast<ServerPlayer>(source);
    
    if (player != NULL)
    {
        level = server->getLevel(player->dimension);
        
        if (x == INT_MIN || y == INT_MIN || z == INT_MIN)
        {
            x = (int)player->x;
            y = (int)player->y;
            z = (int)player->z;
        }
    }
    else
    {
        level = server->getLevel(0);
        
        if (x == INT_MIN || y == INT_MIN || z == INT_MIN)
        {
            source->sendMessage(L"Player Not Founded Coords Are Required: /summon <entity> <x> <y> <z>");
            return;
        }
    }
    
    if (level == NULL) return;

    int entityId = -1;
    wchar_t* endptr;
    long numId = wcstol(entityType.c_str(), &endptr, 10);
    if (*endptr == L'\0')
    {
        entityId = (int)numId;
    }
    else
    {
        if (entityType == L"creeper") entityId = 50;
        else if (entityType == L"skeleton") entityId = 51;
        else if (entityType == L"spider") entityId = 52;
        else if (entityType == L"giant") entityId = 53;
        else if (entityType == L"zombie") entityId = 54;
        else if (entityType == L"slime") entityId = 55;
        else if (entityType == L"ghast") entityId = 56;
        else if (entityType == L"pigzombie") entityId = 57;
        else if (entityType == L"enderman") entityId = 58;
        else if (entityType == L"cavespider") entityId = 59;
        else if (entityType == L"silverfish") entityId = 60;
        else if (entityType == L"blaze") entityId = 61;
        else if (entityType == L"lavaslime") entityId = 62;
        else if (entityType == L"enderdragon") entityId = 63;
        else if (entityType == L"wither") entityId = 64;
        else if (entityType == L"bat") entityId = 65;
        else if (entityType == L"witch") entityId = 66;
        else if (entityType == L"pig") entityId = 90;
        else if (entityType == L"sheep") entityId = 91;
        else if (entityType == L"cow") entityId = 92;
        else if (entityType == L"chicken") entityId = 93;
        else if (entityType == L"squid") entityId = 94;
        else if (entityType == L"wolf") entityId = 95;
        else if (entityType == L"mushroomcow") entityId = 96;
        else if (entityType == L"snowman") entityId = 97;
        else if (entityType == L"ocelot") entityId = 98;
        else if (entityType == L"villagergolem") entityId = 99;
        else if (entityType == L"horse") entityId = 100;
        else if (entityType == L"villager") entityId = 120;
        else if (entityType == L"item") entityId = 1;
        else if (entityType == L"xporb") entityId = 2;
        else if (entityType == L"painting") entityId = 9;
        else if (entityType == L"arrow") entityId = 10;
        else if (entityType == L"snowball") entityId = 11;
        else if (entityType == L"fireball") entityId = 12;
        else if (entityType == L"smallfireball") entityId = 13;
        else if (entityType == L"enderpearl") entityId = 14;
        else if (entityType == L"potion") entityId = 16;
        else if (entityType == L"experiencebottle") entityId = 17;
        else if (entityType == L"itemframe") entityId = 18;
        else if (entityType == L"witherskull") entityId = 19;
        else if (entityType == L"tnt") entityId = 20;
        else if (entityType == L"fallingsand") entityId = 21;
        else if (entityType == L"fireworks") entityId = 22;
        else if (entityType == L"boat") entityId = 41;
        else if (entityType == L"minecart") entityId = 42;
        else if (entityType == L"chestminecart") entityId = 43;
        else if (entityType == L"furnaceminecart") entityId = 44;
        else if (entityType == L"tntminecart") entityId = 45;
        else if (entityType == L"hopperminecart") entityId = 46;
        else if (entityType == L"spawnerminecart") entityId = 47;
        else
        {
            source->sendMessage(L"Unknown entity type: " + entityType);
            return;
        }
    }

    shared_ptr<Entity> entity = EntityIO::newById(entityId, level);
    
    if (entity == NULL)
    {
        source->sendMessage(L"Failed to summon entity: " + entityType);
        return;
    }

    entity->setPos(x + 0.5, y, z + 0.5);
    level->addEntity(entity);

    wstring successMsg = L"Summoned " + entityType + L" at (" + 
                         to_wstring(x) + L", " + to_wstring(y) + L", " + to_wstring(z) + L")";
    source->sendMessage(successMsg);
}

shared_ptr<GameCommandPacket> SummonCommand::preparePacket(const wstring& entityType, int x, int y, int z)
{
    ByteArrayOutputStream baos;
    DataOutputStream dos(&baos);

    dos.writeUTF(entityType);
    dos.writeInt(x);
    dos.writeInt(y);
    dos.writeInt(z);

    return shared_ptr<GameCommandPacket>(new GameCommandPacket(eGameCommand_Summon, baos.toByteArray()));
}