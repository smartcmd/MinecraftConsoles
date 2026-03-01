#include "stdafx.h"
#include "net.minecraft.commands.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.damagesource.h"
#include "BasicTypeContainers.h"
#include "KillCommand.h"

EGameCommand KillCommand::getId()
{
	return eGameCommand_Kill;
}

int KillCommand::getPermissionLevel()
{
	return LEVEL_ALL;
}

void KillCommand::execute(shared_ptr<CommandSender> source, byteArray commandData)
{
	shared_ptr<Player> player = dynamic_pointer_cast<Player>(source);

	player->hurt(DamageSource::outOfWorld, Float::MAX_VALUE);

	source->sendMessage(L"Ouch. That look like it hurt.");
//source.sendMessage(ChatMessageComponent.forTranslation("commands.kill.success"));
}