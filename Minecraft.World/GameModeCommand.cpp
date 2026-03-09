#include "stdafx.h"
#include "net.minecraft.commands.h"
#include "../Minecraft.Client/ServerPlayer.h"
#include "LevelSettings.h"
#include "GameModeCommand.h"

EGameCommand GameModeCommand::getId()
{
	return eGameCommand_GameMode;
}

int GameModeCommand::getPermissionLevel()
{
	return LEVEL_GAMEMASTERS;
}

 void GameModeCommand::execute(shared_ptr<CommandSender> source, byteArray commandData)
 {

    ByteArrayInputStream bais(commandData);
    DataInputStream dis(&bais);
 
    wstring gamemodeStr = dis.readUTF();
    PlayerUID uid = dis.readPlayerUID();

    GameType *gamemode = getModeForString(source, gamemodeStr);
    shared_ptr<ServerPlayer> player = getPlayer(uid);
 
	if (gamemode != NULL)
    {
        player->setGameMode(gamemode);
        player->fallDistance = 0;
 
        //ChatMessageComponent mode = ChatMessageComponent.forTranslation("gameMode."  newMode.getName());

        if (player != source)
        {
            logAdminAction(source, ChatPacket::e_ChatCustom, L"Set game mode of " + player->getName() + L" to " + gamemode->getName(), gamemode->getId(), player->getAName());
            //logAdminAction(source, AdminLogCommand::LOGTYPE_DONT_SHOW_TO_SELF, "commands.gamemode.success.other", player->getAName(), mode);
        }
        else
        {
            logAdminAction(source, ChatPacket::e_ChatCustom, L"Set own game mode to " + gamemode->getName(), gamemode->getId(), player->getAName());
            //logAdminAction(source, AdminLogCommand::LOGTYPE_DONT_SHOW_TO_SELF, "commands.gamemode.success.self", mode);
        }
	}
 }

GameType *GameModeCommand::getModeForString(shared_ptr<CommandSender> source, const wstring &name)
{
    if (equalsIgnoreCase(name, GameType::SURVIVAL->getName()) || equalsIgnoreCase(name, L"s"))
	{
		return GameType::SURVIVAL;
    }
    else if (equalsIgnoreCase(name, GameType::CREATIVE->getName()) || equalsIgnoreCase(name, L"c"))
    {
		return GameType::CREATIVE;
    }
    else if (equalsIgnoreCase(name, GameType::ADVENTURE->getName()) || equalsIgnoreCase(name, L"a"))
    {
		return GameType::ADVENTURE;
	}
	else
	{
        return nullptr;
	}
}