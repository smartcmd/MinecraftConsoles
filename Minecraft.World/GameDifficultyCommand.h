/*
package net.minecraft.commands.common;

import java.util.List;

import net.minecraft.commands.*;
import net.minecraft.commands.exceptions.UsageException;
import net.minecraft.locale.I18n;
import net.minecraft.network.chat.ChatMessageComponent;
import net.minecraft.server.MinecraftServer;

public class GameDifficultyCommand extends BaseCommand {

    // note: copied from Options.java, move to shared location?
    private static final String[] DIFFICULTY_NAMES = {
            "options.difficulty.peaceful", "options.difficulty.easy", "options.difficulty.normal", "options.difficulty.hard"
    };

    @Override
    public String getName() {
        return "difficulty";
    }

    @Override
    public int getPermissionLevel() {
        return LEVEL_GAMEMASTERS;
    }


    @Override
    public String getUsage(CommandSender source) {
        return "commands.difficulty.usage";
    }

    @Override
    public void execute(CommandSender source, String[] args) {
        if (args.length > 0) {
            int newDiff = getDifficultyForString(source, args[0]);

            MinecraftServer.getInstance().setDifficulty(newDiff);

            logAdminAction(source, "commands.difficulty.success", ChatMessageComponent.forTranslation(DIFFICULTY_NAMES[newDiff]));

            return;
        }

        throw new UsageException("commands.difficulty.usage");
    }

    protected int getDifficultyForString(CommandSender source, String name) {
        if (name.equalsIgnoreCase("peaceful") || name.equalsIgnoreCase("p")) {
            return 0;
        } else if (name.equalsIgnoreCase("easy") || name.equalsIgnoreCase("e")) {
            return 1;
        } else if (name.equalsIgnoreCase("normal") || name.equalsIgnoreCase("n")) {
            return 2;
        } else if (name.equalsIgnoreCase("hard") || name.equalsIgnoreCase("h")) {
            return 3;
        } else {
            return convertArgToInt(source, name, 0, 3);
        }
    }

    @Override
    public List<String> matchArguments(CommandSender source, String[] args) {
        if (args.length == 1) {
            return matchArguments(args, "peaceful", "easy", "normal", "hard");
        }

        return null;
    }

}

*/