#pragma once

// 4J Not converted
#if 0
class ScoreboardSaveData extends SavedData {
    public static final String FILE_ID = "scoreboard";

    private Scoreboard scoreboard;
    private CompoundTag delayLoad;

    public ScoreboardSaveData() {
        this(FILE_ID);
    }

    public ScoreboardSaveData(String id) {
        super(id);
    }

    public void setScoreboard(Scoreboard scoreboard) {
        this.scoreboard = scoreboard;

        if (delayLoad != null) {
            load(delayLoad);
        }
    }

    @Override
    public void load(CompoundTag tag) {
        if (scoreboard == null) {
            delayLoad = tag;
            return;
        }

        loadObjectives((ListTag<CompoundTag>) tag.getList("Objectives"));
        loadPlayerScores((ListTag<CompoundTag>) tag.getList("PlayerScores"));

        if (tag.contains("DisplaySlots")) {
            loadDisplaySlots(tag.getCompound("DisplaySlots"));
        }

        if (tag.contains("Teams")) {
            loadTeams((ListTag<CompoundTag>) tag.getList("Teams"));
        }
    }

    protected void loadTeams(ListTag<CompoundTag> list) {
        for (int i = 0; i < list.size(); i++) {
            CompoundTag tag = list.get(i);

            PlayerTeam team = scoreboard.addPlayerTeam(tag.getString("Name"));
            team.setDisplayName(tag.getString("DisplayName"));
            team.setPrefix(tag.getString("Prefix"));
            team.setSuffix(tag.getString("Suffix"));
            if (tag.contains("AllowFriendlyFire")) team.setAllowFriendlyFire(tag.getBoolean("AllowFriendlyFire"));
            if (tag.contains("SeeFriendlyInvisibles")) team.setSeeFriendlyInvisibles(tag.getBoolean("SeeFriendlyInvisibles"));

            loadTeamPlayers(team, (ListTag<StringTag>) tag.getList("Players"));
        }
    }

    protected void loadTeamPlayers(PlayerTeam team, ListTag<StringTag> list) {
        for (int i = 0; i < list.size(); i++) {
            scoreboard.addPlayerToTeam(list.get(i).data, team);
        }
    }

    protected void loadDisplaySlots(CompoundTag tag) {
        for (int i = 0; i < Scoreboard.DISPLAY_SLOTS; i++) {
            if (tag.contains("slot_" + i)) {
                String name = tag.getString("slot_" + i);
                Objective objective = scoreboard.getObjective(name);
                scoreboard.setDisplayObjective(i, objective);
            }
        }
    }

    protected void loadObjectives(ListTag<CompoundTag> list) {
        for (int i = 0; i < list.size(); i++) {
            CompoundTag tag = list.get(i);

            ObjectiveCriteria criteria = ObjectiveCriteria.CRITERIA_BY_NAME.get(tag.getString("CriteriaName"));
            Objective objective = scoreboard.addObjective(tag.getString("Name"), criteria);
            objective.setDisplayName(tag.getString("DisplayName"));
        }
    }

    protected void loadPlayerScores(ListTag<CompoundTag> list) {
        for (int i = 0; i < list.size(); i++) {
            CompoundTag tag = list.get(i);

            Objective objective = scoreboard.getObjective(tag.getString("Objective"));
            Score score = scoreboard.getPlayerScore(tag.getString("Name"), objective);
            score.setScore(tag.getInt("Score"));
        }
    }

    @Override
    public void save(CompoundTag tag) {
        if (scoreboard == null) {
            MinecraftServer.getInstance().getLogger().warning("Tried to save scoreboard without having a scoreboard...");
            return;
        }

        tag.put("Objectives", saveObjectives());
        tag.put("PlayerScores", savePlayerScores());
        tag.put("Teams", saveTeams());

        saveDisplaySlots(tag);
    }

    protected ListTag<CompoundTag> saveTeams() {
        ListTag<CompoundTag> list = new ListTag<CompoundTag>();
        Collection<PlayerTeam> teams = scoreboard.getPlayerTeams();

        for (PlayerTeam team : teams) {
            CompoundTag tag = new CompoundTag();

            tag.putString("Name", team.getName());
            tag.putString("DisplayName", team.getDisplayName());
            tag.putString("Prefix", team.getPrefix());
            tag.putString("Suffix", team.getSuffix());
            tag.putBoolean("AllowFriendlyFire", team.isAllowFriendlyFire());
            tag.putBoolean("SeeFriendlyInvisibles", team.canSeeFriendlyInvisibles());

            ListTag<StringTag> playerList = new ListTag<StringTag>();

            for (String player : team.getPlayers()) {
                playerList.add(new StringTag("", player));
            }

            tag.put("Players", playerList);

            list.add(tag);
        }

        return list;
    }

    protected void saveDisplaySlots(CompoundTag tag) {
        CompoundTag slots = new CompoundTag();
        boolean hasDisplaySlot = false;

        for (int i = 0; i < Scoreboard.DISPLAY_SLOTS; i++) {
            Objective objective = scoreboard.getDisplayObjective(i);

            if (objective != null) {
                slots.putString("slot_" + i, objective.getName());
                hasDisplaySlot = true;
            }
        }

        if (hasDisplaySlot) tag.putCompound("DisplaySlots", slots);
    }

    protected ListTag<CompoundTag> saveObjectives() {
        ListTag<CompoundTag> list = new ListTag<CompoundTag>();
        Collection<Objective> objectives = scoreboard.getObjectives();

        for (Objective objective : objectives) {
            CompoundTag tag = new CompoundTag();

            tag.putString("Name", objective.getName());
            tag.putString("CriteriaName", objective.getCriteria().getName());
            tag.putString("DisplayName", objective.getDisplayName());

            list.add(tag);
        }

        return list;
    }

    protected ListTag<CompoundTag> savePlayerScores() {
        ListTag<CompoundTag> list = new ListTag<CompoundTag>();
        Collection<Score> scores = scoreboard.getScores();

        for (Score score : scores) {
            CompoundTag tag = new CompoundTag();

            tag.putString("Name", score.getOwner());
            tag.putString("Objective", score.getObjective().getName());
            tag.putInt("Score", score.getScore());

            list.add(tag);
        }

        return list;
    }
};
#endif