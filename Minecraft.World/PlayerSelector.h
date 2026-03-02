/*
package net.minecraft.commands;

import net.minecraft.Pos;
import net.minecraft.server.MinecraftServer;
import net.minecraft.server.level.ServerPlayer;
import net.minecraft.util.Mth;
import net.minecraft.world.level.Level;
import net.minecraft.world.level.LevelSettings;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class PlayerSelector {
    private static final Pattern PATTERN_TARGETS = Pattern.compile("^@([parf])(?:\\[([\\w=,!-]*)\\])?$");
    private static final Pattern PATTERN_SHORT_ARGUMENT = Pattern.compile("\\G([-!]?[\\w-]*)(?:$|,)");
    private static final Pattern PATTERN_LONG_ARGUMENT = Pattern.compile("\\G(\\w+)=([-!]?[\\w-]*)(?:$|,)");

    private static final int TARGETS_GROUP_TYPE = 1;
    private static final int TARGETS_GROUP_ARGS = 2; // Null if not specified

    private static final String TARGET_NEAREST = "p";
    private static final String TARGET_ALL = "a";
    private static final String TARGET_RANDOM = "r";

    private static final String ARGUMENT_RANGE_MAX = "r";
    private static final String ARGUMENT_RANGE_MIN = "rm";
    private static final String ARGUMENT_LEVEL_MAX = "l";
    private static final String ARGUMENT_LEVEL_MIN = "lm";
    private static final String ARGUMENT_COORDINATE_X = "x";
    private static final String ARGUMENT_COORDINATE_Y = "y";
    private static final String ARGUMENT_COORDINATE_Z = "z";
    private static final String ARGUMENT_COUNT = "c";
    private static final String ARGUMENT_MODE = "m";
    private static final String ARGUMENT_SCORE_PREFIX = "score_";
    private static final String ARGUMENT_TEAM_NAME = "team";
    private static final String ARGUMENT_PLAYER_NAME = "name";

    public static ServerPlayer getPlayer(CommandSender source, String input) {
        ServerPlayer[] result = getPlayers(source, input);

        if (result == null || result.length != 1) return null;

        return result[0];
    }

    public static String getPlayerNames(CommandSender source, String input) {
        ServerPlayer[] result = getPlayers(source, input);
        if (result == null || result.length == 0) return null;
        String[] names = new String[result.length];

        for (int i = 0; i < names.length; i++) {
            names[i] = result[i].getDisplayName();
        }

        return BaseCommand.joinStrings(names);
    }

    public static ServerPlayer[] getPlayers(CommandSender source, String input) {
        Matcher matcher = PATTERN_TARGETS.matcher(input);

        if (matcher.matches()) {
            Map<String, String> args = getArguments(matcher.group(TARGETS_GROUP_ARGS));
            String type = matcher.group(TARGETS_GROUP_TYPE);
            int rangeMin = getDefaultRangeMin(type);
            int rangeMax = getDefaultRangeMax(type);
            int levelMin = getDefaultLevelMin(type);
            int levelMax = getDefaultLevelMax(type);
            int count = getDefaultCount(type);
            int mode = LevelSettings.GameType.NOT_SET.getId();
            Pos pos = source.getCommandSenderWorldPosition();
            Map<String, Integer> scores = getScores(args);
            String name = null;
            String team = null;
            boolean requireLevel = false;

            if (args.containsKey(ARGUMENT_RANGE_MIN)) {
                rangeMin = Mth.getInt(args.get(ARGUMENT_RANGE_MIN), rangeMin);
                requireLevel = true;
            }
            if (args.containsKey(ARGUMENT_RANGE_MAX)) {
                rangeMax = Mth.getInt(args.get(ARGUMENT_RANGE_MAX), rangeMax);
                requireLevel = true;
            }
            if (args.containsKey(ARGUMENT_LEVEL_MIN)) {
                levelMin = Mth.getInt(args.get(ARGUMENT_LEVEL_MIN), levelMin);
            }
            if (args.containsKey(ARGUMENT_LEVEL_MAX)) {
                levelMax = Mth.getInt(args.get(ARGUMENT_LEVEL_MAX), levelMax);
            }
            if (args.containsKey(ARGUMENT_COORDINATE_X)) {
                pos.x = Mth.getInt(args.get(ARGUMENT_COORDINATE_X), pos.x);
                requireLevel = true;
            }
            if (args.containsKey(ARGUMENT_COORDINATE_Y)) {
                pos.y = Mth.getInt(args.get(ARGUMENT_COORDINATE_Y), pos.y);
                requireLevel = true;
            }
            if (args.containsKey(ARGUMENT_COORDINATE_Z)) {
                pos.z = Mth.getInt(args.get(ARGUMENT_COORDINATE_Z), pos.z);
                requireLevel = true;
            }
            if (args.containsKey(ARGUMENT_MODE)) {
                mode = Mth.getInt(args.get(ARGUMENT_MODE), mode);
            }
            if (args.containsKey(ARGUMENT_COUNT)) {
                count = Mth.getInt(args.get(ARGUMENT_COUNT), count);
            }
            if (args.containsKey(ARGUMENT_TEAM_NAME)) {
                team = args.get(ARGUMENT_TEAM_NAME);
            }
            if (args.containsKey(ARGUMENT_PLAYER_NAME)) {
                name = args.get(ARGUMENT_PLAYER_NAME);
            }

            Level level = requireLevel ? source.getCommandSenderWorld() : null;

            if (type.equals(TARGET_NEAREST) || type.equals(TARGET_ALL)) {
                List<ServerPlayer> players = MinecraftServer.getInstance().getPlayers().getPlayers(pos, rangeMin, rangeMax, count, mode, levelMin, levelMax, scores, name, team, level);
                return players == null || players.isEmpty() ? new ServerPlayer[0] : players.toArray(new ServerPlayer[0]);
            } else if (type.equals(TARGET_RANDOM)) {
                List<ServerPlayer> players = MinecraftServer.getInstance().getPlayers().getPlayers(pos, rangeMin, rangeMax, 0, mode, levelMin, levelMax, scores, name, team, level);
                Collections.shuffle(players);
                players = players.subList(0, Math.min(count, players.size()));
                return players == null || players.isEmpty() ? new ServerPlayer[0] : players.toArray(new ServerPlayer[0]);
            } else {
                return null;
            }
        } else {
            return null;
        }
    }

    public static Map<String, Integer> getScores(Map<String, String> input) {
        Map<String, Integer> result = new HashMap<String, Integer>();

        for (String key : input.keySet()) {
            if (key.startsWith(ARGUMENT_SCORE_PREFIX) && key.length() > ARGUMENT_SCORE_PREFIX.length()) {
                String name = key.substring(ARGUMENT_SCORE_PREFIX.length());
                result.put(name, Mth.getInt(input.get(key), 1));
            }
        }

        return result;
    }

    public static boolean isList(String input) {
        Matcher matcher = PATTERN_TARGETS.matcher(input);

        if (matcher.matches()) {
            Map<String, String> args = getArguments(matcher.group(TARGETS_GROUP_ARGS));
            String type = matcher.group(TARGETS_GROUP_TYPE);
            int count = getDefaultCount(type);
            if (args.containsKey(ARGUMENT_COUNT)) count = Mth.getInt(args.get(ARGUMENT_COUNT), count);
            return count != 1;
        }

        return false;
    }

    public static boolean isPattern(String input, String onlyType) {
        Matcher matcher = PATTERN_TARGETS.matcher(input);

        if (matcher.matches()) {
            String type = matcher.group(TARGETS_GROUP_TYPE);
            if (onlyType != null && !onlyType.equals(type)) return false;

            return true;
        }

        return false;
    }

    public static boolean isPattern(String input) {
        return isPattern(input, null);
    }

    private static final int getDefaultRangeMin(String type) {
        return 0;
    }

    private static final int getDefaultRangeMax(String type) {
        return 0;
    }

    private static final int getDefaultLevelMax(String type) {
        return Integer.MAX_VALUE;
    }

    private static final int getDefaultLevelMin(String type) {
        return 0;
    }

    private static final int getDefaultCount(String type) {
        if (type.equals(TARGET_ALL)) {
            return 0;
        } else {
            return 1;
        }
    }

    private static Map<String, String> getArguments(String input) {
        HashMap<String, String> result = new HashMap<String, String>();
        if (input == null) return result;
        Matcher matcher = PATTERN_SHORT_ARGUMENT.matcher(input);
        int count = 0;
        int last = -1;

        while (matcher.find()) {
            String name = null;

            switch (count++) {
                case 0:
                    name = ARGUMENT_COORDINATE_X;
                    break;
                case 1:
                    name = ARGUMENT_COORDINATE_Y;
                    break;
                case 2:
                    name = ARGUMENT_COORDINATE_Z;
                    break;
                case 3:
                    name = ARGUMENT_RANGE_MAX;
                    break;
            }

            if (name != null && matcher.group(1).length() > 0) result.put(name, matcher.group(1));
            last = matcher.end();
        }

        if (last < input.length()) {
            matcher = PATTERN_LONG_ARGUMENT.matcher(last == -1 ? input : input.substring(last));

            while (matcher.find()) {
                result.put(matcher.group(1), matcher.group(2));
            }
        }

        return result;
    }
}

*/