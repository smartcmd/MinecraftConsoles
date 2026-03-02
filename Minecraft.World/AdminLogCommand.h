#pragma once

#include "ChatPacket.h"

class CommandSender;

class AdminLogCommand
{
public:
	static const int LOGTYPE_DONT_SHOW_TO_SELF = 1;

	virtual void logAdminCommand(shared_ptr<CommandSender> source, int type, ChatPacket::EChatPacketMessage messageType, const wstring& message = L"", int customData = -1, const wstring& additionalMessage = L"") = 0;
};