#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "net.minecraft.world.entity.h"
#include "PacketListener.h"
#include "TextureChangePacket.h"



TextureChangePacket::TextureChangePacket()
{
	id = -1;
	action = e_TextureChange_Skin;
	path = L"";
}

TextureChangePacket::TextureChangePacket(shared_ptr<Entity> e, ETextureChangeType action, const wstring &path)
{
	id = e->entityId;
	this->action = action;
	this->path = path;
}

void TextureChangePacket::read(DataInputStream *dis) //throws IOException 
{
	id = dis->readInt();
	action = (ETextureChangeType)dis->readByte();
	path = dis->readUTF();
}

void TextureChangePacket::write(DataOutputStream *dos) //throws IOException 
{
	dos->writeInt(id);
	dos->writeByte(action);
	dos->writeUTF(path);
}

void TextureChangePacket::handle(PacketListener *listener) 
{
	listener->handleTextureChange(shared_from_this());
}

int TextureChangePacket::getEstimatedSize() 
{
	return 5 + (int)path.size();
}
