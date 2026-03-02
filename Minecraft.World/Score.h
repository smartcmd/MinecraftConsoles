#pragma once

class Scoreboard;
class Objective;

class Score
{
public:
	// 4J Not converted
#if 0
	static final Comparator<Score> SCORE_COMPARATOR = new Comparator<Score>() {
		@Override
			public int compare(Score o1, Score o2) {
				if (o1.getScore() > o2.getScore()) {
					return 1;
				} else if (o1.getScore() < o2.getScore()) {
					return -1;
				} else {
					return 0;
				}
		}
	};
#endif

private:
	Scoreboard *scoreboard;
	Objective *objective;
	wstring owner;
	int count;

public:
	Score(Scoreboard *scoreboard, Objective *objective, const wstring &owner);

	void add(int count);
	void remove(int count);
	void increment();
	void decrement();
	int getScore();
	void setScore(int score);
	Objective *getObjective();
	wstring getOwner();
	Scoreboard *getScoreboard();
	void updateFor(vector<shared_ptr<Player> > *players);
};