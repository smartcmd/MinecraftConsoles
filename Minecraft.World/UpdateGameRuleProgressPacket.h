#pragma once
using namespace std;

#include "Packet.h"

class UpdateGameRuleProgressPacket : public Packet, public enable_shared_from_this<UpdateGameRuleProgressPacket>
{
public:
	ConsoleGameRules::EGameRuleType m_definitionType;
	wstring m_messageId;
	int m_icon, m_auxValue;
	int m_dataTag;
	byteArray m_data;

	UpdateGameRuleProgressPacket();
	UpdateGameRuleProgressPacket(ConsoleGameRules::EGameRuleType definitionType, const wstring &messageId, int icon, int auxValue, int dataTag, void *data, int dataLength);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new UpdateGameRuleProgressPacket()); }
	virtual int getId() { return 158; }
};