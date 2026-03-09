#include "stdafx.h"
#include "BlockReplacements.h"
#include "net.minecraft.world.level.tile.h"

byteArray BlockReplacements::replacements = byteArray(256);

void BlockReplacements::staticCtor()
{
    for (int i = 0; i < 256; i++)
	{
        byte b = static_cast<byte>(i);
        if (b != 0 && Tile::tiles[b & 0xff] == nullptr)
		{
            b = 0;
        }
        BlockReplacements::replacements[i] = b;
    }
}

void BlockReplacements::replace(byteArray blocks)
{
    for (unsigned int i = 0; i < blocks.length; i++)
	{
        blocks[i] = replacements[blocks[i] & 0xff];
    }
}