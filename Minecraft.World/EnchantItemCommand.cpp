#include "stdafx.h"
#include "net.minecraft.network.packet.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.enchantment.h"
#include "../Minecraft.Client/ServerPlayer.h"
#include "EnchantItemCommand.h"

EGameCommand EnchantItemCommand::getId()
{
	return eGameCommand_EnchantItem;
}

int EnchantItemCommand::getPermissionLevel()
{
	return LEVEL_GAMEMASTERS;
}

void EnchantItemCommand::execute(shared_ptr<CommandSender> source, byteArray commandData)
{
	ByteArrayInputStream bais(commandData);
	DataInputStream dis(&bais);

	PlayerUID uid = dis.readPlayerUID();
	int enchantmentId = dis.readInt();
	int enchantmentLevel = dis.readInt();

	bais.reset();

	shared_ptr<ServerPlayer> player = getPlayer(uid);

	if(player == nullptr) return;

	shared_ptr<ItemInstance> selectedItem = player->getSelectedItem();

	if(selectedItem == nullptr) return;

	Enchantment *e = Enchantment::enchantments[enchantmentId];

	if(e == nullptr) return;
	if(!e->canEnchant(selectedItem)) return;

	if(enchantmentLevel < e->getMinLevel()) enchantmentLevel = e->getMinLevel();
	if(enchantmentLevel > e->getMaxLevel()) enchantmentLevel = e->getMaxLevel();

	if (selectedItem->hasTag())
	{
		ListTag<CompoundTag> *enchantmentTags = selectedItem->getEnchantmentTags();
		if (enchantmentTags != nullptr)
		{
			for (int i = 0; i < enchantmentTags->size(); i++)
			{
				int type = enchantmentTags->get(i)->getShort((wchar_t *)ItemInstance::TAG_ENCH_ID);

				if (Enchantment::enchantments[type] != nullptr)
				{
					Enchantment *other = Enchantment::enchantments[type];
					if (!other->isCompatibleWith(e))
					{
						return;
						//throw new CommandException("commands.enchant.cantCombine", e.getFullname(level), other.getFullname(enchantmentTags.get(i).getShort(ItemInstance.TAG_ENCH_LEVEL)));
					}
				}
			}
		}
	}

	selectedItem->enchant(e, enchantmentLevel);

	//logAdminAction(source, "commands.enchant.success");
	logAdminAction(source, ChatPacket::e_ChatCustom, L"commands.enchant.success");
}

shared_ptr<GameCommandPacket> EnchantItemCommand::preparePacket(shared_ptr<Player> player, int enchantmentId, int enchantmentLevel)
{
	if(player == nullptr) return nullptr;

	ByteArrayOutputStream baos;
	DataOutputStream dos(&baos);

	dos.writePlayerUID(player->getXuid());
	dos.writeInt(enchantmentId);
	dos.writeInt(enchantmentLevel);

	return std::make_shared<GameCommandPacket>(eGameCommand_EnchantItem, baos.toByteArray());
}