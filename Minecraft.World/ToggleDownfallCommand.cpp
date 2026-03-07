#include "stdafx.h"
#include "../Minecraft.Client/MinecraftServer.h"
#include "../Minecraft.Client/ServerLevel.h"
#include "net.minecraft.commands.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.storage.h"
#include "net.minecraft.network.packet.h"
#include "ToggleDownfallCommand.h"

EGameCommand ToggleDownfallCommand::getId()
{
	return eGameCommand_ToggleDownfall;
}

int ToggleDownfallCommand::getPermissionLevel()
{
	return LEVEL_GAMEMASTERS;
}

void ToggleDownfallCommand::execute(shared_ptr<CommandSender> source, byteArray commandData)
{
	doToggleDownfall();
	logAdminAction(source, ChatPacket::e_ChatCustom, L"commands.downfall.success");
}

void ToggleDownfallCommand::doToggleDownfall()
{
	MinecraftServer::getInstance()->levels[0]->toggleDownfall();
	MinecraftServer::getInstance()->levels[0]->getLevelData()->setThundering(true);
}

shared_ptr<GameCommandPacket> ToggleDownfallCommand::preparePacket()
{
	return shared_ptr<GameCommandPacket>( new GameCommandPacket(eGameCommand_ToggleDownfall, byteArray() ));
}