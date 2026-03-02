#pragma once
using namespace std;

#include "Packet.h"
#include "..\Minecraft.Client\Model.h"
#include "..\Minecraft.Client\SkinBox.h"

class DLCSkinFile;

class TextureAndGeometryPacket : public Packet, public enable_shared_from_this<TextureAndGeometryPacket>
{
public:
	wstring textureName;
	DWORD dwSkinID;
	PBYTE pbData;
	DWORD dwTextureBytes;
	SKIN_BOX *BoxDataA;
	DWORD dwBoxC;
	unsigned int uiAnimOverrideBitmask;

	TextureAndGeometryPacket();
	~TextureAndGeometryPacket();
	TextureAndGeometryPacket(const wstring &textureName, PBYTE pbData, DWORD dwBytes); 
	TextureAndGeometryPacket(const wstring &textureName, PBYTE pbData, DWORD dwBytes, DLCSkinFile *pDLCSkinFile); 
	TextureAndGeometryPacket(const wstring &textureName, PBYTE pbData, DWORD dwBytes, vector<SKIN_BOX *> *pvSkinBoxes, unsigned int uiAnimOverrideBitmask); 

	virtual void handle(PacketListener *listener);
	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new TextureAndGeometryPacket()); }
	virtual int getId() { return 160; }
};