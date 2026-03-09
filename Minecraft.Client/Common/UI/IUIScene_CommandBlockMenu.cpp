#include "stdafx.h"
#include "../../../Minecraft.World/CustomPayloadPacket.h"
#include "MultiPlayerLocalPlayer.h"
#include "ClientConnection.h"
#include "IUIScene_CommandBlockMenu.h"

void IUIScene_CommandBlockMenu::Initialise(CommandBlockEntity *commandBlock)
{
	m_commandBlock = commandBlock;
	SetCommand(m_commandBlock->getCommand());
}

void IUIScene_CommandBlockMenu::ConfirmButtonClicked()
{
	ByteArrayOutputStream baos;
	DataOutputStream dos(&baos);

	dos.writeInt(m_commandBlock->x);
	dos.writeInt(m_commandBlock->y);
	dos.writeInt(m_commandBlock->z);
	dos.writeUTF(GetCommand());

	Minecraft::GetInstance()->localplayers[GetPad()]->connection->send(std::make_shared<CustomPayloadPacket>(CustomPayloadPacket::SET_ADVENTURE_COMMAND_PACKET, baos.toByteArray()));
}

