#pragma once

#include "Command.h"

class TimeCommand : public Command
{
public:
	virtual EGameCommand getId();
	virtual void execute(shared_ptr<CommandSender> source, byteArray commandData);

protected:
	void doSetTime(shared_ptr<CommandSender> source, int value);
	void doAddTime(shared_ptr<CommandSender> source, int value);

public:
	static shared_ptr<GameCommandPacket> preparePacket(bool night);
};