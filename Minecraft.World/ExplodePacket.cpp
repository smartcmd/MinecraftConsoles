#include "stdafx.h"
#include <iostream>
#include "InputOutputStream.h"
#include "net.minecraft.world.level.h"
#include "PacketListener.h"
#include "ExplodePacket.h"



ExplodePacket::ExplodePacket() 
{
	x = 0;
	y = 0;
	z = 0;
	r = 0.0f;
	m_bKnockbackOnly = false;
	knockbackX = 0.0f;
	knockbackY = 0.0f;
	knockbackZ = 0.0f;
}

ExplodePacket::ExplodePacket(double x, double y, double z, float r, unordered_set<TilePos, TilePosKeyHash, TilePosKeyEq> *toBlow, Vec3 *knockback, bool knockBackOnly)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->r = r;
	m_bKnockbackOnly = knockBackOnly;

	if(toBlow != nullptr)
	{
		this->toBlow.assign(toBlow->begin(),toBlow->end());
	}

	if (knockback != nullptr)
	{
		knockbackX = static_cast<float>(knockback->x);
		knockbackY = static_cast<float>(knockback->y);
		knockbackZ = static_cast<float>(knockback->z);
	}
}

void ExplodePacket::read(DataInputStream *dis) //throws IOException 
{
	m_bKnockbackOnly = dis->readBoolean();

	if(!m_bKnockbackOnly)
	{
		x = dis->readDouble();
		y = dis->readDouble();
		z = dis->readDouble();
		r = dis->readFloat();
		int count = dis->readInt();

		int xp = static_cast<int>(x);
		int yp = static_cast<int>(y);
		int zp = static_cast<int>(z);
		for (int i=0; i<count; i++) 
		{
			int xx = static_cast<signed char>(dis->readByte())+xp;
			int yy = static_cast<signed char>(dis->readByte())+yp;
			int zz = static_cast<signed char>(dis->readByte())+zp;
			toBlow.push_back( TilePos(xx, yy, zz) );
		}
	}

	knockbackX = dis->readFloat();
	knockbackY = dis->readFloat();
	knockbackZ = dis->readFloat();
}

void ExplodePacket::write(DataOutputStream *dos) //throws IOException 
{
	dos->writeBoolean(m_bKnockbackOnly);

	if(!m_bKnockbackOnly)
	{
		dos->writeDouble(x);
		dos->writeDouble(y);
		dos->writeDouble(z);
		dos->writeFloat(r);
		dos->writeInt(static_cast<int>(toBlow.size()));

		int xp = static_cast<int>(x);
		int yp = static_cast<int>(y);
		int zp = static_cast<int>(z);

		for ( const TilePos& tp : toBlow )
		{
			int xx = tp.x-xp;
			int yy = tp.y-yp;
			int zz = tp.z-zp;
			dos->writeByte(xx);
			dos->writeByte(yy);
			dos->writeByte(zz);
		}
	}

	dos->writeFloat(knockbackX);
	dos->writeFloat(knockbackY);
	dos->writeFloat(knockbackZ);
}

void ExplodePacket::handle(PacketListener *listener)
{
	listener->handleExplosion(shared_from_this());
}

int ExplodePacket::getEstimatedSize() 
{
	return 8*3+4+4+static_cast<int>(toBlow.size())*3+12;
}

float ExplodePacket::getKnockbackX()
{
	return knockbackX;
}

float ExplodePacket::getKnockbackY()
{
	return knockbackY;
}

float ExplodePacket::getKnockbackZ()
{
	return knockbackZ;
}