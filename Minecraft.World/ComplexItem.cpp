#include "stdafx.h"
#include "net.minecraft.network.packet.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "ComplexItem.h"

ComplexItem::ComplexItem(int id) : Item(id)
{
}

bool ComplexItem::isComplex()
{
    return true;
}

shared_ptr<Packet> ComplexItem::getUpdatePacket(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
    return nullptr;
}