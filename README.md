# Quake 4 Fishing Mod

**Author:** Yarem M Oscanoa Ballardo    **Course:** IT 266 001  **Date:** October 28th, 2026

In this mod the **grenade launcher is a fishing rod** and **the floor is the water**. Cast, hook the fish, win a tension-based reeling minigame, sell your catch, buy bait and gear upgrades, switch between three fishing zones with different fish, and fill your trophy room. Everything is built with Quake 4's own code, GUIs and engine features. No custom models, sounds or textures are used.

---

## 1. Requirements
- Quake 4 for PC, Windows 10/11

## 2. Controls
| Key | Action |
|---|---|
| **Left mouse (Fire)** | Cast / set the hook when a fish bites / reel in |
| **F1** | Help screen (also opens by itself the first time you play) |
| **F2** | Shop |
| **F3** | Travel guide |
| **F4** | Trophy room |
| **1 to 9** | Choose an option in the open menu |
| **Same F key again** | Close the menu |
| Console: `fish status` | Print your money, bait, gear and location |

The console opens with **Ctrl+Alt+~** in retail Quake 4.

## 3. How to play
1. **Aim at the floor** in front of you (the floor counts as water, walls and sky do not) and press **Fire** to cast.
2. Wait for **"BITE! PRESS FIRE NOW!"** and press **Fire** straight away. You have about 1.5 seconds. Miss it and the fish gets away.
3. **Hold Fire to reel in, release to let the line rest.**
   - The **TENSION** bar rises while you reel. If it passes your line's limit, **the line snaps**.
   - The **PROGRESS** bar fills as the fish comes closer. Fill it to catch the fish.
   - If you release for too long the fish swims away and escapes.
   - Strong fish pull in surges. Reel during the calm moments and rest during surges.
4. Press **F2** and sell your fish for money.
5. Spend money on better bait and gear, or unlock new zones in the travel guide (**F3**).

You start with $0, the free **Worm** bait and level-0 gear in the **Pond** zone. Catch a few small fish first and sell them.

## 4. Game data

### Fish (10 species, 3 zones)
| Fish | Zone | Rarity | Strength | Weight range | Favourite bait |
|---|---|---|---|---|---|
| Minnow | Pond | Common | 1.0 | 0.05 - 0.2 kg | Worm |
| Perch | Pond | Common | 2.0 | 0.2 - 0.9 kg | Worm |
| Bluegill | Pond | Common | 2.0 | 0.15 - 0.7 kg | Cricket |
| Trout | River | Uncommon | 3.0 | 0.5 - 3 kg | Cricket |
| Catfish | River | Uncommon | 4.0 | 1 - 9 kg | Shrimp |
| Bass | River | Uncommon | 5.0 | 0.8 - 6 kg | Shrimp |
| Pike | Deep Lake | Rare | 6.0 | 2 - 12 kg | Glow Lure |
| Salmon | Deep Lake | Rare | 7.0 | 3 - 15 kg | Glow Lure |
| Sturgeon | Deep Lake | Epic | 8.5 | 10 - 60 kg | Glow Lure |
| Golden Koi | Deep Lake | Legendary | 10.0 | 5 - 30 kg | Shrimp |

A fish's favourite bait makes it 3 times more likely to bite. Bigger fish sell for more money, and big fish are rare (sizes are weighted towards the small end).

### Bait
| Bait | Price | Effect |
|---|---|---|
| Worm | Free, unlimited | Normal bite speed |
| Cricket | $5 each (sold in 5s) | Bites 1.3x faster |
| Shrimp | $15 each (sold in 5s) | Bites 1.5x faster, 1.5x chance of rare fish |
| Glow Lure | $40 each (sold in 5s) | Bites 2x faster, 2.5x chance of rare fish |

### Gear (4 modifications, 3 levels each)
| Slot | Effect | Level 1 | Level 2 |
|---|---|---|---|
| **Rod** | Weaker pull from fish | $150 | $400 |
| **Line** | Higher snap limit | $150 | $400 |
| **Reel** | Faster reeling | $150 | $400 |
| **Hook** | Faster bites and a longer hook window | $150 | $400 |

Each level multiplies the stat: level 0 = x1.0, level 1 = x1.25, level 2 = x1.5.

### Zones (travel guide)
| Zone | Cost | Fish |
|---|---|---|
| Pond | Free | Minnow, Perch, Bluegill |
| River | $200 | Trout, Catfish, Bass |
| Deep Lake | $600 | Pike, Salmon, Sturgeon, Golden Koi |

Unlocking is permanent. This version uses one map, so travelling changes the fish you can catch instead of loading a new level.

## 5. Saving
Your money, bait, gear, unlocked zones, catches and records are saved in an archived console variable (`fish_save`) every time something changes. They are kept between maps and between play sessions. To start over, open the console and type `fish reset`.

## 6. Console commands (for testing)
| Command | Does |
|---|---|
| `fish status` | Print stats |
| `fish help` / `shop` / `travel` / `trophy` | Opens a screen |
| `fish pick N` | Choose option N in the open menu |
| `fish buy bait N` / `fish buy gear N` | Buy bait (N = 1-3) / upgrade gear (N = 0-3) |
| `fish sell` | Sell all fish |
| `fish bait N` | Select bait N (0-3) |
| `fish money` / `fish money N` | 	Demo helper: add $100,000 (or $N) |
| `fish reset` | Erase all fishing data |
| `seta fish_anywhere 1` | Debug: cast at any surface (set to 0 for normal play) |

## 7. Design notes
- **Rod = grenade launcher.** The launcher's fire state casts, hooks and reels instead of shooting. Ammo is not used.
- **One system drives everything.** Fish strength, bait and gear all feed into the same formulas, so each upgrade has a clear effect on the minigame.
- **Tension-meter minigame.** Chosen because it ties fish strength, line and reel stats together naturally and is easy to read on a text HUD.
- **Floor as water.** The cast uses a trace from the player's eyes and accepts upward-facing surfaces (floors), so the single map works without water geometry.
- **Data tables are in code** (`Fishing.cpp`), which makes balancing quick.
- **Text-based UI.** The HUD and menus are Quake 4 GUI files filled with live text from the game code, with the tension and progress bars drawn using characters.

## 8. Known issues
- Number keys in menus need the weapon-switch block in `Player.cpp`, allowing the user to use the number in the menus. Text spacing depends on the GUI font chosen, so some of the texts are wrongly or not spaced at all.
- Loading a Quake 4 saved game while a menu is open may leave the menu open (`fish close` fixes it).
- Single player only. Multiplayer is not supported.

## 9. Credits
- Quake 4 and its SDK by id Software and Raven Software. Built on the Quake 4 game SDK.
- My friends who help me to come up with some ideas.
