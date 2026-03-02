/*
package net.minecraft.commands.common;

import net.minecraft.commands.BaseCommand;
import net.minecraft.commands.CommandSender;
import net.minecraft.commands.exceptions.CommandException;
import net.minecraft.commands.exceptions.UsageException;
import net.minecraft.network.packet.LevelSoundPacket;
import net.minecraft.server.level.ServerPlayer;
import net.minecraft.world.entity.player.Player;

public class PlaySoundCommand extends BaseCommand {
    @Override
    public String getName() {
        return "playsound";
    }

    @Override
    public int getPermissionLevel() {
        return LEVEL_GAMEMASTERS;
    }

    @Override
    public String getUsage(CommandSender source) {
        return "commands.playsound.usage";
    }

    @Override
    public void execute(CommandSender source, String[] args) {
        if (args.length < 2) {
            throw new UsageException(getUsage(source));
        }

        int index = 0;
        String sound = args[index++];
        ServerPlayer player = convertToPlayer(source, args[index++]);
        double x = player.getCommandSenderWorldPosition().x;
        double y = player.getCommandSenderWorldPosition().y;
        double z = player.getCommandSenderWorldPosition().z;
        double volume = 1;
        double pitch = 1;
        double minVolume = 0;

        if (args.length > index) x = convertArgToCoordinate(source, x, args[index++]);
        if (args.length > index) y = convertArgToCoordinate(source, y, args[index++], 0, 0);
        if (args.length > index) z = convertArgToCoordinate(source, z, args[index++]);

        if (args.length > index) volume = convertArgToDouble(source, args[index++], 0, Float.MAX_VALUE);
        if (args.length > index) pitch = convertArgToDouble(source, args[index++], 0, 2);
        if (args.length > index) minVolume = convertArgToDouble(source, args[index++], 0, 1);

        double maxDist = volume > 1 ? volume * 16 : 16;
        double dist = player.distanceTo(x, y, z);

        if (dist > maxDist) {
            if (minVolume > 0) {
                double deltaX = x - player.x;
                double deltaY = y - player.y;
                double deltaZ = z - player.z;
                double length = Math.sqrt(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);
                double soundX = player.x;
                double soundY = player.y;
                double soundZ = player.z;

                if (length > 0) {
                    soundX += deltaX / length * 2;
                    soundY += deltaY / length * 2;
                    soundZ += deltaZ / length * 2;
                }

                player.connection.send(new LevelSoundPacket(sound, soundX, soundY, soundZ, (float) minVolume, (float) pitch));
            } else {
                throw new CommandException("commands.playsound.playerTooFar", player.getAName());
            }
        } else {
            player.connection.send(new LevelSoundPacket(sound, x, y, z, (float) volume, (float) pitch));
        }

        logAdminAction(source, "commands.playsound.success", sound, player.getAName());
    }

    @Override
    public boolean isValidWildcardPlayerArgument(String[] args, int argumentIndex) {
        return argumentIndex == 1;
    }
}

*/