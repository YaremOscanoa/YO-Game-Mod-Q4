#include "idlib/CmdArgs.h"
#include <cstddef>
#include "idlib/Str.h"
#ifndef __FISHING_H__
#define __FISHING_H__

#define FISH_COUNT 10
#define BAIT_COUNT 4
#define GEAR_SLOTS 4
#define LOC_COUNT 3
#define HELP_PAGES 5
#define MENU_LINES 16

class idPlayer;
class idUserInterface;

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
	const char* name;
	int price;			// cost to buy
	float biteMult;		// multiplier for bite chance
	float rareMult;		// multiplier for hook window
} baitInfo_t;

typedef struct {
	int money, pending, heldCount;
	int curBait, loc, unlocked, helpSeen;
	int bigIdx, bigSize;
	int gear[GEAR_SLOTS];
	int bait[BAIT_COUNT];
	int caught[FISH_COUNT];
	int best[FISH_COUNT];
} fishSave_t;

class yoFishing {
public:
	enum { FS_IDLE, FS_WAIT, FS_BITE, FS_REEL };
	enum { MENU_NONE, MENU_HELP, MENU_SHOP, MENU_TRAVEL, MENU_TROPHY };

	yoFishing() { hudGui = NULL; menuGui = NULL; loaded = false; msgUntil = 0; Cancel(); }

	void Cancel();						// reset minigame state
	bool TryCast(idPlayer* p);			// called by the weapon's Fire state
	bool WeaponBusy();					// returns true if the weapon is busy with fishing
	void Think(idPlayer* p);			// called by the weapon every frame
	void Draw(idPlayer* p);				// called by the weapon every frame
	void Command(idPlayer* p, const idCmdArgs& args);
	bool MenuOpen() const { return menuType != MENU_NONE; }

private:
	void Load();
	void Save();
	void Notice(idPlayer* p, const char* msg);
	bool FindWater(idPlayer* p);
	int PickFish();
	float GearBonus(int slot);
	void Catch(idPlayer* p);
	void Lose(idPlayer* p, const char* why);

	//UI
	void CheckLevelChange();
	void LoadGuis();
	void SetMenuText(const char* key, const char* text);
	void UpdateUI(idPlayer* p);
	void OpenMenu(int type);

	void RefreshMenu();
	void Pick(idPlayer* p, int n);
	void Put(int line, const char* text);
	void BuildHelp();
	void BuildShop();
	void BuildTravel();
	void BuildTrophy();

	void BuyBait(idPlayer* p, int n);
	void BuyGear(idPlayer* p, int n);
	void SellAll(idPlayer* p);
	void CycleBait(idPlayer* p);
	void Travel(idPlayer* p, int n);
	void Status();

	int holdUntil, castTime;
	int state, nextCastTime, biteTime, hookEnd, nextSurge, nextHud;
	int fishIdx;
	bool wasDown, loaded;
	float dist, tension, surge;
	fishSave_t sv;

	idUserInterface* hudGui, * menuGui;
	int menuType, helpPage, lastImpulseSeq, msgUntil, lastTime;
	idStr lastMsg;
};

extern yoFishing fishing;
void Cmd_Fish_f(const idCmdArgs& args);

#endif