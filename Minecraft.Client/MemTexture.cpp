#include "stdafx.h"
#include "MemTexture.h"

MemTexture::MemTexture(const wstring& _url, PBYTE pbData,DWORD dwBytes, MemTextureProcessor *processor)
{
	// 4J - added
    count = 1;
    id = -1;
    isLoaded = false;
	ticksSinceLastUse = 0;

	// 4J - TODO - actually implement

	// load the texture, and process it
	//loadedImage=Textures::getTexture()
	// 4J - remember to add deletes in here for any created BufferedImages when implemented
	loadedImage = new BufferedImage(pbData,dwBytes);
	if(processor==nullptr)
	{

	}
	else
	{
		//loadedImage=processor.process(ImageIO.read(huc.getInputStream()));
	}
	

}

MemTexture::~MemTexture()
{
	delete loadedImage;
}