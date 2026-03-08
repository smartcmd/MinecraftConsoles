#pragma once

#include "Command.h"
#include "level.h"

class GameCommandPacket;

class SummonCommand : public Command
{
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(shared_ptr<CommandSender> source, byteArray commandData);

    static shared_ptr<GameCommandPacket> preparePacket(const wstring& entityType, int x, int y, int z);
};