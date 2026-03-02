/*
package net.minecraft.commands.common;

import java.util.*;

import net.minecraft.SharedConstants;
import net.minecraft.commands.*;
import net.minecraft.commands.exceptions.UsageException;
import net.minecraft.server.MinecraftServer;
import net.minecraft.world.level.Level;
import net.minecraft.world.level.storage.LevelData;

public class WeatherCommand extends BaseCommand {
    @Override
    public String getName() {
        return "weather";
    }

    @Override
    public int getPermissionLevel() {
        return LEVEL_GAMEMASTERS;
    }

    @Override
    public String getUsage(CommandSender source) {
        return "commands.weather.usage";
    }

    @Override
    public void execute(CommandSender source, String[] args) {
        if (args.length < 1 || args.length > 2) {
            throw new UsageException("commands.weather.usage");
        }

        int duration = (300 + new Random().nextInt(600)) * SharedConstants.TICKS_PER_SECOND;
        if (args.length >= 2) {
            duration = convertArgToInt(source, args[1], 1, 1000000) * SharedConstants.TICKS_PER_SECOND;
        }

        Level level = MinecraftServer.getInstance().levels[0];
        LevelData levelData = level.getLevelData();
        levelData.setRainTime(duration);
        levelData.setThunderTime(duration);

        if ("clear".equalsIgnoreCase(args[0])) {
            levelData.setRaining(false);
            levelData.setThundering(false);
            logAdminAction(source, "commands.weather.clear");
        } else if ("rain".equalsIgnoreCase(args[0])) {
            levelData.setRaining(true);
            levelData.setThundering(false);
            logAdminAction(source, "commands.weather.rain");
        } else if ("thunder".equalsIgnoreCase(args[0])) {
            levelData.setRaining(true);
            levelData.setThundering(true);
            logAdminAction(source, "commands.weather.thunder");
        } else {
            throw new UsageException("commands.weather.usage");
        }
    }

    @Override
    public List<String> matchArguments(CommandSender source, String[] args) {
        if (args.length == 1) {
            return matchArguments(args, "clear", "rain", "thunder");
        }

        return null;
    }

}

*/