#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "net.minecraft.world.entity.h"
#include "PacketListener.h"
#include "TextureAndGeometryChangePacket.h"




TextureAndGeometryChangePacket::TextureAndGeometryChangePacket()
{
	id = -1;
	path = L"";
	dwSkinID = 0;
}

TextureAndGeometryChangePacket::TextureAndGeometryChangePacket(shared_ptr<Entity> e, const wstring &path)
{
	id = e->entityId;
	this->path = path;
	wstring skinValue = path.substr(7,path.size());
	skinValue = skinValue.substr(0,skinValue.find_first_of(L'.'));
	std::wstringstream ss;
	ss << std::dec << skinValue.c_str();
	ss >> dwSkinID;
	dwSkinID = MAKE_SKIN_BITMASK(true, dwSkinID);

}

void TextureAndGeometryChangePacket::read(DataInputStream *dis) //throws IOException 
{
	id = dis->readInt();
	dwSkinID = dis->readInt();
	path = dis->readUTF();
}

void TextureAndGeometryChangePacket::write(DataOutputStream *dos) //throws IOException 
{
	dos->writeInt(id);
	dos->writeInt(dwSkinID);
	dos->writeUTF(path);
}

void TextureAndGeometryChangePacket::handle(PacketListener *listener) 
{
	listener->handleTextureAndGeometryChange(shared_from_this());
}

int TextureAndGeometryChangePacket::getEstimatedSize() 
{
	return 8 + (int)path.size();
}
