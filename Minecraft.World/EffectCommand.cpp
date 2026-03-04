#include "stdafx.h"
#include "net.minecraft.commands.common.h"
#include "..\Minecraft.Client\MinecraftServer.h"

EGameCommand EffectCommand::getId()
{
	return eGameCommand_Effect;
}

int EffectCommand::getPermissionLevel()
{
	return LEVEL_GAMEMASTERS;
}

wstring EffectCommand::getUsage(CommandSender *source)
{
	return L"commands.effect.usage";
}

void EffectCommand::execute(shared_ptr<CommandSender> source, byteArray commandData)
{
	//if (args.length >= 2)
	//{
	//	Player player = convertToPlayer(source, args[0]);

	//	if (args[1].equals("clear")) {
	//		if (player.getActiveEffects().isEmpty()) {
	//			throw new CommandException("commands.effect.failure.notActive.all", player.getAName());
	//		} else {
	//			player.removeAllEffects();
	//			logAdminAction(source, "commands.effect.success.removed.all", player.getAName());
	//		}
	//	} else {
	//		int effectId = convertArgToInt(source, args[1], 1);
	//		int duration = SharedConstants.TICKS_PER_SECOND * 30;
	//		int seconds = 30;
	//		int amplifier = 0;

	//		if (effectId < 0 || effectId >= MobEffect.effects.length || MobEffect.effects[effectId] == null) {
	//			throw new InvalidNumberException("commands.effect.notFound", effectId);
	//		}

	//		if (args.length >= 3) {
	//			seconds = convertArgToInt(source, args[2], 0, 1000000);
	//			if (MobEffect.effects[effectId].isInstantenous()) {
	//				duration = seconds;
	//			} else {
	//				duration = seconds * SharedConstants.TICKS_PER_SECOND;
	//			}
	//		} else if (MobEffect.effects[effectId].isInstantenous()) {
	//			duration = 1;
	//		}

	//		if (args.length >= 4) {
	//			amplifier = convertArgToInt(source, args[3], 0, 255);
	//		}

	//		if (seconds == 0) {
	//			if (player.hasEffect(effectId)) {
	//				player.removeEffect(effectId);
	//				logAdminAction(source, "commands.effect.success.removed", ChatMessageComponent.forTranslation(MobEffect.effects[effectId].getDescriptionId()), player.getAName());
	//			} else {
	//				throw new CommandException("commands.effect.failure.notActive", ChatMessageComponent.forTranslation(MobEffect.effects[effectId].getDescriptionId()), player.getAName());
	//			}
	//		} else {
	//			MobEffectInstance instance = new MobEffectInstance(effectId, duration, amplifier);
	//			player.addEffect(instance);
	//			logAdminAction(source, "commands.effect.success", ChatMessageComponent.forTranslation(instance.getDescriptionId()), effectId, amplifier, player.getAName(), seconds);
	//		}
	//	}

	//	return;
	//}

	//throw new UsageException("commands.effect.usage");
}

wstring EffectCommand::getPlayerNames()
{
	return L""; //MinecraftServer::getInstance()->getPlayerNames();
}

bool EffectCommand::isValidWildcardPlayerArgument(wstring args, int argumentIndex)
{
	return argumentIndex == 0;
}