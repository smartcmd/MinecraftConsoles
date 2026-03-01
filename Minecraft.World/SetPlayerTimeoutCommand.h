/*
package net.minecraft.commands.common;

import net.minecraft.commands.BaseCommand;
import net.minecraft.commands.CommandSender;
import net.minecraft.commands.exceptions.UsageException;
import net.minecraft.server.MinecraftServer;

public class SetPlayerTimeoutCommand extends BaseCommand {
    public String getName() {
        return "setidletimeout";
    }

    @Override
    public int getPermissionLevel() {
        return LEVEL_ADMINS;
    }

    @Override
    public String getUsage(CommandSender source) {
        return "commands.setidletimeout.usage";
    }

    public void execute(CommandSender source, String[] args) {
        if (args.length == 1) {
            int timeout = convertArgToInt(source, args[0], 0);
            MinecraftServer.getInstance().setPlayerIdleTimeout(timeout);
            logAdminAction(source, "commands.setidletimeout.success", timeout);
            return;
        }

        throw new UsageException("commands.setidletimeout.usage");
    }
}

*/