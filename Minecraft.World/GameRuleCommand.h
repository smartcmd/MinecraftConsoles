/*
package net.minecraft.commands.common;

import net.minecraft.commands.BaseCommand;
import net.minecraft.commands.CommandSender;
import net.minecraft.commands.exceptions.UsageException;
import net.minecraft.network.chat.ChatMessageComponent;
import net.minecraft.server.MinecraftServer;
import net.minecraft.world.level.GameRules;

import java.util.List;

public class GameRuleCommand extends BaseCommand {
    @Override
    public String getName() {
        return "gamerule";
    }

    @Override
    public int getPermissionLevel() {
        return LEVEL_GAMEMASTERS;
    }


    @Override
    public String getUsage(CommandSender source) {
        return "commands.gamerule.usage";
    }

    @Override
    public void execute(CommandSender source, String[] args) {
        if (args.length == 2) {
            String rule = args[0];
            String value = args[1];

            GameRules rules = getRules();

            if (rules.contains(rule)) {
                rules.set(rule, value);
                logAdminAction(source, "commands.gamerule.success");
            } else {
                logAdminAction(source, "commands.gamerule.norule", rule);
            }

            return;
        } else if (args.length == 1) {
            String rule = args[0];
            GameRules rules = getRules();

            if (rules.contains(rule)) {
                String value = rules.get(rule);
                source.sendMessage(ChatMessageComponent.forPlainText(rule).addPlainText(" = ").addPlainText(value));
            } else {
                logAdminAction(source, "commands.gamerule.norule", rule);
            }

            return;
        } else if (args.length == 0) {
            GameRules rules = getRules();
            source.sendMessage(ChatMessageComponent.forPlainText(joinStrings(rules.getRuleNames())));
            return;
        }

        throw new UsageException("commands.gamerule.usage");
    }

    @Override
    public List<String> matchArguments(CommandSender source, String[] args) {
        if (args.length == 1) {
            return matchArguments(args, getRules().getRuleNames());
        } else if (args.length == 2) {
            return matchArguments(args, "true", "false");
        }

        return null;
    }

    private GameRules getRules() {
        return MinecraftServer.getInstance().getLevel(0).getGameRules();
    }
}

*/