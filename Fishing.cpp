#include "../YO-Game-Mod-Q4/idlib/precompiled.h"
#pragma hdrstop

#include "../../../git/mod-newfiles/YO-Game-Mod-Q4/game/Game_local.h"
#include "Fishing.h"
#include "idlib/CmdArgs.h"
#include "idlib/Str.h"
#include <cstddef>
#include "../YO-Game-Mod-Q4/framework/CVarSystem.h"

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

static const char* helpTitle[HELP_PAGES] = {"Controls", "How to fish", "Fish and bait", "Gear and shop", "Travel and trophies" };

static const char* helpText[HELP_PAGES][10] = {
	{ "Fire (left mouse) .... cast / hook the fish / reel",
	  "F1 .... this help screen",
	  "F2 .... shop (buy bait and gear, sell fish)",
	  "F3 .... travel guide",
	  "F4 .... trophy room",
	  "Number keys .... choose an option inside the menu",
	  "Press the same F key again to close the menu",
	  "Console: 'fish status' prints all your stats", NULL },

	{ "1. Aim at the floor in front of you (it is your water).",
	  "    Press Fire to cast.",
	  "2. Wait. When you see BITE, press Fire immediately.",
	  "3. HOLD Fire to reel in. Release to let the line rest.",
	  "4. Watch TENSION. Above the limit the line snaps!",
	  "5. Fill the PROGRESS bar to land the fish.",
	  "6. Strong fish pull in surges. Reel during the calm.",
	  "7. Sell your catch in the shop (F2) for money.", NULL },

	{ "Worm: free and unlimited.  Cricket: faster bites.",
	  "Shrimp: faster bites and more rare fish.",
	  "Glow Lure: fastest bites, best for rare fish.",
	  "Every fish has a favourite bait (3x more likely).",
	  "10 fish in 3 locations:",
	  "Pond: Minnow, Perch, Bluegill",
	  "River: Trout, Catfish, Bass",
	  "Deep Lake: Pike, Salmon, Sturgeon, Golden Koi", NULL },

	{ "Four gear slots, each with levels 0 to 2:",
	  "Rod: weaker fish pull on you",
	  "Line: higher snap limit",
	  "Reel: faster reeling",
	  "Hook: faster bites and a longer hook window",
	  "Level 1 costs $150, level 2 costs $400.",
	  "Bigger fish sell for more money.", NULL },

	{ "Travel guide (F3): Pond free, River $200, Deep Lake $600.",
	  "Unlocking a location is permanent.",
	  "Traveling changes which fish you can catch.",
	  "Money, gear, bait and records are never lost.",
	  "Trophy room (F4): every fish, count, personal best,",
	  "and your biggest catch. Unknown fish show as ???.", NULL }
};

static idStr Bar(float frac, int n) {
	int k = idMath::ClampInt(0, n, (int)(frac * n));
	idStr s = "[";
	for (int i = 0; i < n; i++) {
		s += (i < k) ? "#" : "-";
	}
	s += "]";
	return s;
}

/* ===================== PERSISTENCE ===================== */

void yoFishing::Load() {
	if (loaded) return;
	loaded = true;
	memset(&sv, 0, sizeof(sv));
	sv.unlocked = 1;
	sv.bait[0] = 9999;

	idStr s = fish_save.GetString();
	if (s.Length() < 10) return;

	idLexer lex(s.c_str(), s.Length(), "fish", LEXFL_NOERRORS);
	sv.money = lex.ParseInt();  sv.pending = lex.ParseInt(); sv.heldCount = lex.ParseInt();
	sv.curBait = lex.ParseInt(); sv.loc = lex.ParseInt(); sv.unlocked = lex.ParseInt();
	sv.helpSeen = lex.ParseInt(); sv.bigIdx = lex.ParseInt(); sv.bigSize = lex.ParseInt();
	for (int i = 0; i < GEAR_SLOTS; i++)  sv.gear[i] = lex.ParseInt();
	for (int i = 0; i < BAIT_COUNT; i++)  sv.bait[i] = lex.ParseInt();
	for (int i = 0; i < FISH_COUNT; i++)  sv.caught[i] = lex.ParseInt();
	for (int i = 0; i < FISH_COUNT; i++)  sv.best[i] = lex.ParseInt();
	sv.bait[0] = 9999;
}

void yoFishing::Save() {
	idStr s;
	s += va("%d %d %d %d %d %d %d %d %d ", sv.money, sv.pending, sv.heldCount, sv.curBait,
		sv.loc, sv.unlocked, sv.helpSeen, sv.bigIdx, sv.bigSize);
	for (int i = 0; i < GEAR_SLOTS; i++)  s += va("%d ", sv.gear[i]);
	for (int i = 0; i < BAIT_COUNT; i++)  s += va("%d ", sv.bait[i]);
	for (int i = 0; i < FISH_COUNT; i++)  s += va("%d ", sv.caught[i]);
	for (int i = 0; i < FISH_COUNT; i++)  s += va("%d ", sv.best[i]);
	cvarSystem->SetCVarString("fish_save", s.c_str());
}

/* ===================== HELPERS ===================== */

void yoFishing::Cancel() {
	state = FS_IDLE; nextCastTime = 0; wasDown = false;
	dist = tension = surge = 0.0f; fishIdx = 0;
	biteTime = hookEnd = nextSurge = nextHud = 0;

	// UI: pointers are dropped on every level load and reloaded in LoadGuis()
	hudGui = NULL;
	menuGui = NULL;
	menuType = MENU_NONE;
	helpPage = 0;
	lastImpulseSeq = -1;
	lastTime = 0;
	holdUntil = 0;
	castTime = 0;
}

void yoFishing::Notice(idPlayer* p, const char* msg) {
	lastMsg = msg;
	msgUntil = gameLocal.time + 4000;
	gameLocal.Printf("[Fishing] %s\n", msg);

	if (p && p->hud) {
		p->hud->SetStateString("main_notice_text", msg);
		p->hud->HandleNamedEvent("main_notice");
	}
}

float yoFishing::GearBonus(int slot) {
	return gearBonus[idMath::ClampInt(0, 2, sv.gear[slot])];
}

// The FLOOR is the water: casting works when the view ray hits an upward-facing surface.
bool yoFishing::FindWater(idPlayer* p) {
	if (fish_anywhere.GetBool()) {
		return true;
	}

	const float maxCast = 600.0f;     // how far you can cast
	const float minCast = 48.0f;      // too close to your feet is not allowed

	idVec3 start = p->GetEyePosition();
	idVec3 end = start + p->viewAxis[0] * maxCast;

	trace_t tr;
	gameLocal.TracePoint(p, tr, start, end, MASK_SOLID, p);

	if (tr.fraction >= 1.0f)               return false;   // sky / nothing
	if (tr.fraction* maxCast < minCast)   return false;   // too close
	return tr.c.normal.z > 0.7f;                             // floors yes, walls/ceilings no
}

int yoFishing::PickFish() {
	float total = 0.0f, w[FISH_COUNT];
	for (int i = 0; i < FISH_COUNT; i++) {
		w[i] = 0.0f;
		if (fishTable[i].loc != sv.loc) continue;
		w[i] = (float)fishTable[i].weight;
		if (fishTable[i].favBait == sv.curBait) w[i] *= 3.0f;
		if (fishTable[i].weight <= 8) w[i] *= baitTable[sv.curBait].rareMult;
		total += w[i];
	}
	float r = gameLocal.random.RandomFloat() * total;
	for (int i = 0; i < FISH_COUNT; i++) {
		if (w[i] <= 0.0f) continue;
		if (r < w[i]) return i;
		r -= w[i];
	}
	return 0;
}

/* ===================== MINIGAME ===================== */

bool yoFishing::WeaponBusy() {
	return gameLocal.time < holdUntil;
}

bool yoFishing::TryCast(idPlayer* p) {
	Load();
	holdUntil = gameLocal.time + 150;               // every Fire press keeps the weapon in Fire for 150 ms

	if (menuType != MENU_NONE) return false;      // clicking inside a menu must not cast

	if (state == FS_WAIT) {
		if (gameLocal.time < castTime + 700) return false;   // still holding the button from the cast
		Lose(p, "You reeled in empty line.");
		return false;
	}
	if (state == FS_BITE) {                       // pressed fire in time: hook!
		state = FS_REEL;
		dist = 100.0f; tension = 20.0f; surge = 0.5f; nextSurge = 0;
		Notice(p, va("Hooked a %s! HOLD fire to reel, release to relax the line.", fishTable[fishIdx].name));
		return false;
	}
	if (state == FS_REEL) return false;
	if (gameLocal.time < nextCastTime) return false;

	if (!FindWater(p)) {
		Notice(p, "Aim at the floor in front of you to cast.");
		nextCastTime = gameLocal.time + 500;
		return false;
	}

	if (sv.bait[sv.curBait] <= 0) { sv.curBait = 0; Notice(p, "Out of bait, switching to Worm."); }
	if (sv.curBait != 0) sv.bait[sv.curBait]--;

	float delay = 3.0f + gameLocal.random.RandomFloat() * 5.0f;
	delay /= (baitTable[sv.curBait].biteMult * GearBonus(3));
	biteTime = gameLocal.time + SEC2MS(delay);
	state = FS_WAIT;
	castTime = gameLocal.time;
	Notice(p, va("Cast with %s. Waiting for a bite...", baitTable[sv.curBait].name));
	return true;                                    // weapon plays its fire animation
}

void yoFishing::CheckLevelChange() {
	if (gameLocal.time < lastTime) {
		Cancel();
	}
	lastTime = gameLocal.time;
}

void yoFishing::Think(idPlayer* p) {
	CheckLevelChange();
	Load();
	UpdateUI(p);

	bool down = (p->usercmd.buttons & BUTTON_ATTACK) != 0;
	float dt = MS2SEC(gameLocal.msec);

	if (state == FS_WAIT && gameLocal.time >= biteTime) {
		fishIdx = PickFish();
		state = FS_BITE;
		hookEnd = gameLocal.time + SEC2MS(1.5f * GearBonus(3));
		Notice(p, "A BITE! Press fire NOW!");
	}
	else if (state == FS_BITE && gameLocal.time > hookEnd) {
		Lose(p, "Too slow, the fish got away.");
	}
	else if (state == FS_REEL) {
		const fishInfo_t& f = fishTable[fishIdx];

		if (gameLocal.time >= nextSurge) {                 // fish changes how hard it pulls
			surge = 0.3f + gameLocal.random.RandomFloat() * 1.2f;
			nextSurge = gameLocal.time + 700 + gameLocal.random.RandomInt(1000);
		}
		float pull = f.strength * 4.0f * surge / GearBonus(0);   // Rod
		float reelSpeed = 12.0f * GearBonus(2);                       // Reel
		float limit = 100.0f * GearBonus(1);                      // Line

		if (down) {
			tension += (pull * 0.6f + 8.0f) * dt;
			dist -= (reelSpeed - pull * 0.15f) * dt;
		}
		else {
			tension -= 25.0f * dt;
			dist += pull * 0.3f * dt;
		}
		tension = idMath::ClampFloat(0.0f, 200.0f, tension);

		if (tension > limit) { Lose(p, "SNAP! The line broke."); }
		else if (dist >= 100.0f) { Lose(p, "The fish escaped."); }
		else if (dist <= 0.0f) { Catch(p); }
	}
	wasDown = down;
}

void yoFishing::Lose(idPlayer* p, const char* why) {
	state = FS_IDLE;
	nextCastTime = gameLocal.time + 1000;
	Notice(p, why);
}

void yoFishing::Catch(idPlayer* p) {
	const fishInfo_t& f = fishTable[fishIdx];
	float r = gameLocal.random.RandomFloat();
	int size = f.minG + (int)((f.maxG - f.minG) * r * r);   // squared: small fish are common

	int price = f.value + (f.value * size) / f.maxG;
	sv.pending += price; sv.heldCount++;
	sv.caught[fishIdx]++;
	if (size > sv.best[fishIdx]) sv.best[fishIdx] = size;
	bool record = size > sv.bigSize;
	if (record) { sv.bigSize = size; sv.bigIdx = fishIdx; }
	Save();

	state = FS_IDLE;
	nextCastTime = gameLocal.time + 1000;
	Notice(p, va("Caught a %s! %.2f kg%s", f.name, size / 1000.0f, record ? "  NEW BIGGEST!" : ""));
}

/* ===================== GUI PLUMBING ===================== */

void yoFishing::LoadGuis() {
	if (hudGui && menuGui) {
		return;
	}
	hudGui = uiManager->FindGui("guis/fish_hud.gui", true, false, true);
	menuGui = uiManager->FindGui("guis/fish_menu.gui", true, false, true);
	if (hudGui) { hudGui->Activate(true, gameLocal.time); }
	if (menuGui) { menuGui->Activate(true, gameLocal.time); }
}

void yoFishing::SetMenuText(const char* key, const char* text) {
	if (menuGui) {
		menuGui->SetStateString(key, text);
	}
}

void yoFishing::Put(int line, const char* text) {
	SetMenuText(va("l%d", line), text);
}

void yoFishing::OpenMenu(int type) {
	LoadGuis();
	if (menuType == type) {          // same key again closes it
		menuType = MENU_NONE;
		return;
	}
	menuType = type;
	helpPage = 0;
	RefreshMenu();
}

void yoFishing::RefreshMenu() {
	if (!menuGui) {
		return;
	}
	for (int i = 0; i < MENU_LINES; i++) {
		Put(i, "");
	}
	switch (menuType) {
	case MENU_HELP:   BuildHelp();   break;
	case MENU_SHOP:   BuildShop();   break;
	case MENU_TRAVEL: BuildTravel(); break;
	case MENU_TROPHY: BuildTrophy(); break;
	}
}

void yoFishing::UpdateUI(idPlayer* p) {
	if (!sv.helpSeen) {
		sv.helpSeen = 1;
		Save();
		OpenMenu(MENU_HELP);
	}

	int seq = p->usercmd.impulseSequence;
	if (lastImpulseSeq == -1) {
		lastImpulseSeq = seq;
		return;
	}
	if (seq != lastImpulseSeq) {
		lastImpulseSeq = seq;
		int imp = p->usercmd.impulse;
		if (menuType != MENU_NONE && imp >= 1 && imp <= 9) {
			Pick(p, imp);
		}
	}
}

void yoFishing::Pick(idPlayer* p, int n) {
	switch (menuType) {
	case MENU_SHOP:
		if (n >= 1 && n <= 3) { BuyBait(p, n); }
		else if (n >= 4 && n <= 7) { BuyGear(p, n - 4); }
		else if (n == 8) { SellAll(p); }
		else if (n == 9) { CycleBait(p); }
		break;
	case MENU_TRAVEL:
		if (n >= 1 && n <= LOC_COUNT) { Travel(p, n - 1); }
		break;
	case MENU_HELP:
		if (n >= 1 && n <= HELP_PAGES) { helpPage = n - 1; }
		break;
	}
	RefreshMenu();
}

/* ===================== MENU CONTENT ===================== */

void yoFishing::BuildHelp() {
	SetMenuText("title", va("HELP - %s  (page %d of %d)", helpTitle[helpPage], helpPage + 1, HELP_PAGES));
	for (int i = 0; helpText[helpPage][i]; i++) {
		Put(i, helpText[helpPage][i]);
	}
	Put(11, "Press a number key to change page:");
	Put(12, "1 Controls   2 How to fish   3 Fish and bait");
	Put(13, "4 Gear and shop   5 Travel and trophies");
	SetMenuText("foot", "Press F1 again to close");
}

void yoFishing::BuildShop() {
	SetMenuText("title", "FISHING SHOP");
	Put(0, va("Money: $%d	Unsold fish: %d (worth $%d)", sv.money, sv.heldCount, sv.pending));

	for (int i = 1; i < BAIT_COUNT; i++) {
		Put(1 + i, va("%d   Buy 5 %s for $%d	(you have %d)", i, baitTable[i].name, baitTable[i].price * 5, sv.bait[i]));
	}
	for (int g = 0; g < GEAR_SLOTS; g++) {
		idStr cost;
		if (sv.gear[g] >= 2) {
			cost = "MAX LEVEL";
		}
		else {
			cost = va("L%d to L%d for $%d", sv.gear[g], sv.gear[g] + 1, gearPrice[sv.gear[g] + 1]);
		}
		Put(6 + g, va("%d   %s: %s     %s", 4 + g, gearSlotName[g], gearSlotHelp[g], cost.c_str()));
	}
	Put(11, "8   Sell all fish");
	Put(12, va("9   Change bait (now using %s)", baitTable[sv.curBait].name));
	SetMenuText("foot", "Press a number to choose.  F2 closes the shop.");
}

void yoFishing::BuildTravel() {
	SetMenuText("title", "TRAVEL GUIDE");
	Put(0, va("Money: $%d      You are at: %s", sv.money, locName[sv.loc]));
	for (int i = 0; i < LOC_COUNT; i++) {
		idStr fish;
		for (int f = 0; f < FISH_COUNT; f++) {
			if (fishTable[f].loc == i) {
				if (fish.Length()) { fish += ", "; }
				fish += fishTable[f].name;
			}
		}
		idStr st;
		if (sv.unlocked & (1 << i)) {
			st = "UNLOCKED";
		}
		else {
			st = va("costs $%d", locCost[i]);
		}
		Put(2 + i * 2, va("%d   %s  (%s)", i + 1, locName[i], st.c_str()));
		Put(3 + i * 2, va("      Fish: %s", fish.c_str()));
	}
	SetMenuText("foot", "Press 1-3 to travel.  F3 closes the guide.");
}

void yoFishing::BuildTrophy() {
	SetMenuText("title", "TROPHY ROOM");
	int found = 0, total = 0;
	for (int i = 0; i < FISH_COUNT; i++) {
		if (sv.caught[i]) {
			found++;
			total += sv.caught[i];
			Put(i, va("%s   caught %d   best %.2f kg", fishTable[i].name, sv.caught[i], sv.best[i] / 1000.0f));
		}
		else {
			Put(i, "???   (not caught yet)");
		}
	}
	Put(11, va("Species found: %d of %d      Total fish: %d", found, FISH_COUNT, total));
	if (sv.bigSize) {
		Put(12, va("BIGGEST CATCH: %s, %.2f kg", fishTable[sv.bigIdx].name, sv.bigSize / 1000.0f));
	}
	else {
		Put(12, "BIGGEST CATCH: none yet");
	}
	SetMenuText("foot", "F4 closes the trophy room.");
}

/* ===================== HUD + MENU DRAWING ===================== */

void yoFishing::Draw(idPlayer* p) {
	CheckLevelChange();
	Load();
	LoadGuis();

	if (hudGui) {
		idStr baitStr = sv.curBait ? va("%d", sv.bait[sv.curBait]) : "inf";
		hudGui->SetStateString("money", va(" Money: $%d		Unsold fish: %d ($%d) ", sv.money, sv.heldCount, sv.pending) );
		hudGui->SetStateString("bait", va(" Bait: %s (%s)		Location: %s ", baitTable[sv.curBait].name, baitStr.c_str(), locName[sv.loc]) );
		hudGui->SetStateString("gear", va(" Rod L%d		Line L%d		Reel L%d		Hook L%d ", sv.gear[0], sv.gear[1], sv.gear[2], sv.gear[3]) );

		const char* st = " Aim at the floor and press Fire to cast ";
		switch (state) {
		case FS_WAIT: st = " Waiting for a bite... ";                      break;
		case FS_BITE: st = " BITE!	PRESS FIRE NOW! ";                    break;
		case FS_REEL: st = va(" Hooked: %s", fishTable[fishIdx].name );  break;
		}
		hudGui->SetStateString("status", st);

		idStr t, d;
		if (state == FS_REEL) {
			float limit = 100.0f * GearBonus(1);
			t = "Tension   " + Bar(tension / limit, 20);
			d = "Progress  " + Bar((100.0f - dist) / 100.0f, 20);
		}
		hudGui->SetStateString("bar1", t.c_str());
		hudGui->SetStateString("bar2", d.c_str());
		hudGui->SetStateString("msg", gameLocal.time < msgUntil ? lastMsg.c_str() : "");
		hudGui->Redraw(gameLocal.time);
	}

	if (menuType != MENU_NONE && menuGui) {
		menuGui->Redraw(gameLocal.time);
	}
}

/* ===================== ACTIONS (shared by menus and console) ===================== */

void yoFishing::BuyBait(idPlayer* p, int n) {
	int cost = baitTable[n].price * 5;
	if (sv.money < cost) { Notice(p, "Not enough money."); return; }
	sv.money -= cost;
	sv.bait[n] += 5;
	Save();
	Notice(p, va("Bought 5 %s.", baitTable[n].name));
}

void yoFishing::BuyGear(idPlayer* p, int n) {
	if (sv.gear[n] >= 2) { Notice(p, "Already max level."); return; }
	int cost = gearPrice[sv.gear[n] + 1];
	if (sv.money < cost) { Notice(p, "Not enough money."); return; }
	sv.money -= cost;
	sv.gear[n]++;
	Save();
	Notice(p, va("%s upgraded to level %d.", gearSlotName[n], sv.gear[n]));
}

void yoFishing::SellAll(idPlayer* p) {
	if (sv.heldCount == 0) { Notice(p, "You have no fish to sell."); return; }
	sv.money += sv.pending;
	Notice(p, va("Sold %d fish for $%d.", sv.heldCount, sv.pending));
	sv.pending = 0;
	sv.heldCount = 0;
	Save();
}

void yoFishing::CycleBait(idPlayer* p) {
	for (int i = 1; i <= BAIT_COUNT; i++) {
		int b = (sv.curBait + i) % BAIT_COUNT;
		if (sv.bait[b] > 0) {
			sv.curBait = b;
			break;
		}
	}
	Save();
	Notice(p, va("Using %s.", baitTable[sv.curBait].name));
}

void yoFishing::Travel(idPlayer* p, int n) {
	if (n < 0 || n >= LOC_COUNT) { return; }
	if (!(sv.unlocked & (1 << n))) {
		if (sv.money < locCost[n]) { Notice(p, "You can't afford that location."); return; }
		sv.money -= locCost[n];
		sv.unlocked |= (1 << n);
	}
	sv.loc = n;
	Save();
	Notice(p, va("You arrive at the %s. Check F3 to see which fish live here.", locName[n]));
}

void yoFishing::Status() {
	idStr b;
	for (int i = 0; i < BAIT_COUNT; i++) {
		b += va("%s:%s  ", baitTable[i].name, i ? va("%d", sv.bait[i]) : "inf");
	}
	gameLocal.Printf("Money $%d | Unsold fish %d ($%d) | Location %s | Bait %s\n  %s\n  Rod L%d Line L%d Reel L%d Hook L%d\n",
		sv.money, sv.heldCount, sv.pending, locName[sv.loc], baitTable[sv.curBait].name, b.c_str(),
		sv.gear[0], sv.gear[1], sv.gear[2], sv.gear[3]);
}

/* ===================== CONSOLE / KEY-BIND COMMANDS ===================== */

void yoFishing::Command(idPlayer* p, const idCmdArgs& args) {
	Load();
	idStr cmd = args.Argv(1);
	idStr a2 = args.Argv(2);
	int   n = atoi(args.Argv(2));

	if (!cmd.Icmp("help")) { OpenMenu(MENU_HELP); }
	else if (!cmd.Icmp("shop")) { OpenMenu(MENU_SHOP); }
	else if (!cmd.Icmp("trophy")) { OpenMenu(MENU_TROPHY); }
	else if (!cmd.Icmp("travel")) {
		if (args.Argc() > 2) { Travel(p, n); }
		else { OpenMenu(MENU_TRAVEL); }
	}
	else if (!cmd.Icmp("pick")) { if (menuType != MENU_NONE) { Pick(p, n); } }
	else if (!cmd.Icmp("close")) { menuType = MENU_NONE; }
	else if (!cmd.Icmp("status")) { Status(); }
	else if (!cmd.Icmp("buy")) {
		int k = atoi(args.Argv(3));
		if (!a2.Icmp("bait") && k >= 1 && k < BAIT_COUNT) { BuyBait(p, k); }
		else if (!a2.Icmp("gear") && k >= 0 && k < GEAR_SLOTS) { BuyGear(p, k); }
		RefreshMenu();
	}
	else if (!cmd.Icmp("sell")) { SellAll(p); RefreshMenu(); }
	else if (!cmd.Icmp("bait") && n >= 0 && n < BAIT_COUNT && sv.bait[n] > 0) {
		sv.curBait = n;
		Save();
		Notice(p, va("Using %s.", baitTable[n].name));
	}
	else if (!cmd.Icmp("money")) {                  // demo helper: fish money [amount]
		int amt = (args.Argc() > 2) ? n : 100000;
		sv.money += amt;
		Save();
		Notice(p, va("Added $%d. You now have $%d.", amt, sv.money));
		RefreshMenu();
	}
	else if (!cmd.Icmp("reset")) {
		loaded = true;
		memset(&sv, 0, sizeof(sv));
		sv.unlocked = 1;
		sv.bait[0] = 9999;
		Save();
		Notice(p, "Fishing data reset.");
	}
	else {
		gameLocal.Printf("fish help | shop | travel [N] | trophy | status | buy bait N | buy gear N | sell | bait N | money [N] | reset\n");
	}
}

void Cmd_Fish_f(const idCmdArgs& args) {
	idPlayer* p = gameLocal.GetLocalPlayer();
	if (!p) return;
	fishing.Command(p, args);
}
