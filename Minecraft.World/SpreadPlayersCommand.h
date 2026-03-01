/*
package net.minecraft.commands.common;

import java.util.*;

import net.minecraft.commands.*;
import net.minecraft.commands.exceptions.*;
import net.minecraft.network.chat.ChatMessageComponent;
import net.minecraft.server.MinecraftServer;
import net.minecraft.server.level.ServerPlayer;
import net.minecraft.util.Mth;
import net.minecraft.world.entity.LivingEntity;
import net.minecraft.world.entity.player.Player;
import net.minecraft.world.level.Level;
import net.minecraft.world.level.material.Material;
import net.minecraft.world.level.tile.Tile;
import net.minecraft.world.scores.Team;

import com.google.common.collect.*;

public class SpreadPlayersCommand extends BaseCommand {
    private static final int MAX_ITERATION_COUNT = 10000;

    @Override
    public String getName() {
        return "spreadplayers";
    }

    @Override
    public int getPermissionLevel() {
        return LEVEL_GAMEMASTERS;
    }

    @Override
    public String getUsage(CommandSender source) {
        return "commands.spreadplayers.usage";
    }

    @Override
    public void execute(CommandSender source, String[] args) {
        if (args.length < 6) throw new UsageException("commands.spreadplayers.usage");
        int index = 0;
        double x = convertArgToCoordinate(source, Double.NaN, args[index++]);
        double z = convertArgToCoordinate(source, Double.NaN, args[index++]);
        double minDist = convertArgToDouble(source, args[index++], 0);
        double maxDist = convertArgToDouble(source, args[index++], minDist + 1);
        boolean respectTeams = convertArgToBoolean(source, args[index++]);

        List<LivingEntity> players = Lists.newArrayList();

        while (index < args.length) {
            String arg = args[index++];

            if (PlayerSelector.isPattern(arg)) {
                ServerPlayer[] result = PlayerSelector.getPlayers(source, arg);

                if (result != null && result.length != 0) {
                    Collections.addAll(players, result);
                } else {
                    throw new PlayerNotFoundException();
                }
            } else {
                Player player = MinecraftServer.getInstance().getPlayers().getPlayer(arg);

                if (player != null) {
                    players.add(player);
                } else {
                    throw new PlayerNotFoundException();
                }
            }
        }

        if (players.isEmpty()) {
            throw new PlayerNotFoundException();
        }

        source.sendMessage(ChatMessageComponent.forTranslation("commands.spreadplayers.spreading." + (respectTeams ? "teams" : "players"), joinPlayerNames(players), x, z, minDist, maxDist));

        spreadPlayers(source, players, new Position(x, z), minDist, maxDist, players.get(0).level, respectTeams);
    }

    private void spreadPlayers(CommandSender source, List<LivingEntity> players, Position center, double spreadDist, double maxDistFromCenter, Level level, boolean respectTeams) {
        Random random = new Random();
        double minX = center.x - maxDistFromCenter;
        double minZ = center.z - maxDistFromCenter;
        double maxX = center.x + maxDistFromCenter;
        double maxZ = center.z + maxDistFromCenter;

        Position[] positions = createInitialPositions(random, respectTeams ? getNumberOfTeams(players) : players.size(), minX, minZ, maxX, maxZ);
        int iterations = spreadPositions(center, spreadDist, level, random, minX, minZ, maxX, maxZ, positions, respectTeams);
        double avgDistance = setPlayerPositions(players, level, positions, respectTeams);

        logAdminAction(source, "commands.spreadplayers.success." + (respectTeams ? "teams" : "players"), positions.length, center.x, center.z);
        if (positions.length > 1) source.sendMessage(ChatMessageComponent.forTranslation("commands.spreadplayers.info." + (respectTeams ? "teams" : "players"), String.format("%.2f", avgDistance),
                iterations));
    }

    private int getNumberOfTeams(List<LivingEntity> players) {
        Set<Team> teams = Sets.newHashSet();

        for (LivingEntity player : players) {
            if (player instanceof Player) {
                teams.add(((Player) player).getTeam());
            } else {
                teams.add(null);
            }
        }

        return teams.size();
    }

    private int spreadPositions(Position center, double spreadDist, Level level, Random random, double minX, double minZ, double maxX, double maxZ, Position[] positions, boolean respectTeams) {
        boolean hasCollisions = true;
        int iteration;
        double minDistance = Float.MAX_VALUE;

        for (iteration = 0; iteration < MAX_ITERATION_COUNT && hasCollisions; iteration++) {
            hasCollisions = false;
            minDistance = Float.MAX_VALUE;

            for (int i = 0; i < positions.length; i++) {
                Position position = positions[i];
                int neighbourCount = 0;
                Position averageNeighbourPos = new Position();

                for (int j = 0; j < positions.length; j++) {
                    if (i == j) continue;
                    Position neighbour = positions[j];

                    double dist = position.dist(neighbour);
                    minDistance = Math.min(dist, minDistance);
                    if (dist < spreadDist) {
                        neighbourCount++;
                        averageNeighbourPos.x += neighbour.x - position.x;
                        averageNeighbourPos.z += neighbour.z - position.z;
                    }
                }

                if (neighbourCount > 0) {
                    averageNeighbourPos.x /= neighbourCount;
                    averageNeighbourPos.z /= neighbourCount;
                    double length = averageNeighbourPos.getLength();

                    if (length > 0) {
                        averageNeighbourPos.normalize();

                        position.moveAway(averageNeighbourPos);
                    } else {
                        position.randomize(random, minX, minZ, maxX, maxZ);
                    }

                    hasCollisions = true;
                }

                if (position.clamp(minX, minZ, maxX, maxZ)) {
                    hasCollisions = true;
                }
            }

            if (!hasCollisions) {
                for (Position position : positions) {
                    if (!position.isSafe(level)) {
                        position.randomize(random, minX, minZ, maxX, maxZ);
                        hasCollisions = true;
                    }
                }
            }
        }

        if (iteration >= MAX_ITERATION_COUNT) {
            throw new CommandException("commands.spreadplayers.failure." + (respectTeams ? "teams" : "players"), positions.length, center.x, center.z, String.format("%.2f", minDistance));
        }

        return iteration;
    }

    private double setPlayerPositions(List<LivingEntity> players, Level level, Position[] positions, boolean respectTeams) {
        double avgDistance = 0;
        int positionIndex = 0;
        Map<Team, Position> teamPositions = Maps.newHashMap();

        for (int i = 0; i < players.size(); i++) {
            LivingEntity player = players.get(i);
            Position position;

            if (respectTeams) {
                Team team = player instanceof Player ? ((Player) player).getTeam() : null;

                if (!teamPositions.containsKey(team)) {
                    teamPositions.put(team, positions[positionIndex++]);
                }

                position = teamPositions.get(team);
            } else {
                position = positions[positionIndex++];
            }

            player.teleportTo(Mth.floor(position.x) + 0.5f, position.getSpawnY(level), Mth.floor(position.z) + 0.5);

            double closest = Double.MAX_VALUE;
            for (int j = 0; j < positions.length; j++) {
                if (position == positions[j]) continue;

                double dist = position.dist(positions[j]);
                closest = Math.min(dist, closest);
            }
            avgDistance += closest;
        }

        avgDistance /= players.size();
        return avgDistance;
    }

    private Position[] createInitialPositions(Random random, int count, double minX, double minZ, double maxX, double maxZ) {
        Position[] result = new Position[count];

        for (int i = 0; i < result.length; i++) {
            Position position = new Position();

            position.randomize(random, minX, minZ, maxX, maxZ);

            result[i] = position;
        }

        return result;
    }

    private static class Position {
        double x;
        double z;

        Position() {
        }

        Position(double x, double z) {
            this.x = x;
            this.z = z;
        }

        void set(double x, double z) {
            this.x = x;
            this.z = z;
        }

        double dist(Position target) {
            double dx = x - target.x;
            double dz = z - target.z;

            return Math.sqrt(dx * dx + dz * dz);
        }

        void normalize() {
            double dist = (double) getLength();
            x /= dist;
            z /= dist;
        }

        float getLength() {
            return Mth.sqrt(x * x + z * z);
        }

        public void moveAway(Position pos) {
            x -= pos.x;
            z -= pos.z;
        }

        public boolean clamp(double minX, double minZ, double maxX, double maxZ) {
            boolean changed = false;

            if (x < minX) {
                x = minX;
                changed = true;
            } else if (x > maxX) {
                x = maxX;
                changed = true;
            }

            if (z < minZ) {
                z = minZ;
                changed = true;
            } else if (z > maxZ) {
                z = maxZ;
                changed = true;
            }

            return changed;
        }

        public int getSpawnY(Level level) {
            int xt = Mth.floor(x);
            int zt = Mth.floor(z);

            for (int y = Level.maxBuildHeight; y > 0; y--) {
                int tile = level.getTile(xt, y, zt);

                if (tile != 0) {
                    return y + 1;
                }
            }

            return Level.maxBuildHeight + 1;
        }

        public boolean isSafe(Level level) {
            int xt = Mth.floor(x);
            int zt = Mth.floor(z);

            for (int y = Level.maxBuildHeight; y > 0; y--) {
                int tile = level.getTile(xt, y, zt);

                if (tile != 0) {
                    Material material = Tile.tiles[tile].material;

                    return !material.isLiquid() && material != Material.fire;
                }
            }

            return false;
        }

        public void randomize(Random random, double minX, double minZ, double maxX, double maxZ) {
            x = Mth.nextDouble(random, minX, maxX);
            z = Mth.nextDouble(random, minZ, maxZ);
        }
    }
}

*/