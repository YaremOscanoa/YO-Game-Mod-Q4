#ifndef __FISHING_H__
#define __FISHING_H__

#define FISH_COUNT 10
#define BAIT_COUNT 4
#define GEAR_SLOTS 4
#define LOC_COUNT 3

class idPlayer;

typedef struct {
	const char* name;	
	int minG, maxG;		// size range in grams
	float strength;		// 1..10
	int value;			// base sell value
	int weight;			// spawn weight (low = rare)
	int favBait;		// bait index that attracts it
	int loc;			// location index
} fishInfo_t;

typedef struct {
	int money, pending, heldCount;
	int curBait, loc, unlocked, helpSeen;
	int bigIdx, bigSize;
	int gear[GEAR_SLOTS];
	int bait[BAIT_COUNT];
	int caught[FISH_COUNT];
	int best[FISH_COUNT];
} fishSave_t;

class rvFishing {
public:
	enum { FS_IDLE, FS_WAIT, FS_BITE, FS_REEL };

	rvFishing() { Cancel(); loaded = false; }

	void Cancel();
	bool TryCast(idPlayer* p);
	void Think(idPlayer* p);
	void Command(idPlayer* p, const idCmdArgs &args);
private:
	void Load();
	void Save();
	void Notice(idPlayer* p, const char* msg);
	bool FindWater(idPlayer* p);
	int PickFish();
	float GearBonus(int slot);
	void Catch(idPlayer* p);
	void Lose(idPlayer* p, const char *why);

	int state, nextCastTime, biteTime, hookEnd, nextSurge, nextHud;
	int fishIdx;
	bool wasDown, loaded;
	float dist, tension, surge;
	fishSave_t sv;
};

extern rvFishing fishing;
void Cmd_Fish_f(const idCmdArgs& args);

#endif