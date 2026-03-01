/*
package net.minecraft.commands.common;

import net.minecraft.commands.*;
import net.minecraft.network.chat.ChatMessageComponent;
import net.minecraft.server.MinecraftServer;
import net.minecraft.world.entity.player.Player;
import net.minecraft.world.level.Level;

public class ShowSeedCommand extends BaseCommand {
    @Override
    public boolean canExecute(CommandSender source) {
        return MinecraftServer.getInstance().isSingleplayer() || super.canExecute(source);
    }

    @Override
    public String getName() {
        return "seed";
    }

    @Override
    public int getPermissionLevel() {
        return LEVEL_GAMEMASTERS;
    }

    @Override
    public String getUsage(CommandSender source) {
        return "commands.seed.usage";
    }

    @Override
    public void execute(CommandSender source, String[] args) {
        Level level = source instanceof Player ? ((Player) source).level : MinecraftServer.getInstance().getLevel(0);
        source.sendMessage(ChatMessageComponent.forTranslation("commands.seed.success", level.getSeed()));
    }
}

*/