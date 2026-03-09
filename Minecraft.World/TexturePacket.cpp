#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "PacketListener.h"
#include "TexturePacket.h"



TexturePacket::TexturePacket() 
{
	this->textureName = L"";
	this->dwBytes = 0;
	this->pbData = nullptr;
}

TexturePacket::~TexturePacket() 
{
	// can't free this - it's used elsewhere
// 	if(this->pbData!=nullptr)
// 	{
// 		delete [] this->pbData;
// 	}
}

TexturePacket::TexturePacket(const wstring &textureName, PBYTE pbData, DWORD dwBytes) 
{
	this->textureName = textureName;
	this->pbData = pbData;
	this->dwBytes = dwBytes;
}

void TexturePacket::handle(PacketListener *listener) 
{
	listener->handleTexture(shared_from_this());
}

void TexturePacket::read(DataInputStream *dis) //throws IOException
{
	textureName = dis->readUTF();
	dwBytes = static_cast<DWORD>(dis->readShort());

	if(dwBytes>0)
	{
		this->pbData= new BYTE [dwBytes];

		for(DWORD i=0;i<dwBytes;i++)
		{
			this->pbData[i] = dis->readByte();
		}
	}
}

void TexturePacket::write(DataOutputStream *dos) //throws IOException 
{
	dos->writeUTF(textureName);
	dos->writeShort(static_cast<short>(dwBytes));
	for(DWORD i=0;i<dwBytes;i++)
	{
		dos->writeByte(this->pbData[i]);
	}
}

int TexturePacket::getEstimatedSize() 
{
	return 4096;
}
