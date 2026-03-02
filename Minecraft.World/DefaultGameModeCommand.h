#pragma once

#include "GameModeCommand.h"

class GameType;

class DefaultGameModeCommand : public GameModeCommand
{
public:
	virtual EGameCommand getId();
	virtual void execute(shared_ptr<CommandSender> source, byteArray commandData);

protected:
	void doSetGameType(GameType *newGameType);
};