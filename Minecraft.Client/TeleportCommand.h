#pragma once

#include "..\Minecraft.World\Command.h"

class TeleportCommand : public Command
{
public:
	virtual EGameCommand getId();
	virtual void execute(shared_ptr<CommandSender> source, byteArray commandData);

	static shared_ptr<GameCommandPacket> preparePacket(PlayerUID subject, PlayerUID destination);
};