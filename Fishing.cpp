#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "../Game_local.h"
#include "Fishing.h"

yoFishing fishing;

idCVar fish_save("fish_save", "", CVAR_GAME | CVAR_ARCHIVE, "fishing mod save data");
idCVar fish_anywhere("fish_anywhere", "0", CVAR_GAME | CVAR_BOOL, "debug: cast without aiming at the floor");

static const fishInfo_t fishTable[FISH_COUNT] = {
	//NAME			MIN		MAX		STR		VALUE	WEIGHT	FAVBAIT	LOC
	{ "Minnow",       50,   200,	1.0f,	  5,	  40,	   0,	 0 },
	{ "Perch",       200,   900,	2.0f,	 12,	  30,	   0,	 0 },
	{ "Bluegill",    150,   700,	2.0f,	 10,	  30,	   1,	 0 },
	{ "Trout",       500,  3000,	3.0f,	 25,	  20,	   1,	 1 },
	{ "Catfish",    1000,  9000,	4.0f,	 40,	  15,	   2,	 1 },
	{ "Bass",        800,  6000,	5.0f,	 55,	  12,	   2,	 1 },
	{ "Pike",       2000, 12000,	6.0f,	 90,	   8,	   3,	 2 },
	{ "Salmon",     3000, 15000,	7.0f,	120,	   6,	   3,	 2 },
	{ "Sturgeon",  10000, 60000,	8.5f,	300,	   3,	   3,	 2 },
	{ "Golden Koi", 5000, 30000,   10.0f,	800,	   1,	   2,	 2 }
};

static const baitInfo_t baitTable[BAIT_COUNT] = {
	//NAME			ATTRACTS
	{ "Worm",      0, 1.0f, 1.0f },   // free, unlimited
	{ "Cricket",   5, 1.3f, 1.0f },
	{ "Shrimp",   15, 1.5f, 1.5f },
	{ "Glow Lure",40, 2.0f, 2.5f }
};

static const char *gearSlotName[GEAR_SLOTS] = { "Rod", "Line", "Reel", "Hook" };
static const char* gearSlotHelp[GEAR_SLOTS] = {"weaker fish pull", "higher snap limit", "faster reeling", "faster bites, longer hook window" };
static const int gearPrice[3] = { 0, 150, 40 };
static const float gearBonus[3] = { 1.0f, 1.25f, 1.5f };

static const char *locName[LOC_COUNT] = { "Pond", "River", "DeepLake" };
static const int locCost[LOC_COUNT] = { 0, 200, 600 };

static const char* helpTitle[HELP_PAGES] = {"Controls", "How to fish", "Gear and shop", "Travel and trophies" };

