/**
 * PauseItemDescriptions.cpp - C-Up item descriptions in pause menu
 *
 * When the player presses C-Up while hovering over a custom item/equipment/mask
 * in the pause menu, a short utility-focused description textbox is displayed.
 */

#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/Enhancements/game-interactor/GameInteractor_Hooks.h"
#include "soh/Enhancements/custom-message/CustomMessageTypes.h"
#include "soh/Enhancements/custom-message/CustomMessageManager.h"
#include "soh/ShipInit.hpp"

extern "C" {
#include "z64.h"
#include "z64item.h"
#include "macros.h"
#include "variables.h"
#include "mods/extended_equipment.h"
#include "expansions/sw97/sw97_config.h"
}

// ---------------------------------------------------------------------------
// Description table: { itemId, textId, description }
// ---------------------------------------------------------------------------

struct LocalizedDesc {
    const char* english;
    const char* german;
    const char* french;
};

struct ItemDescEntry {
    u16 itemId;
    u16 textId;
    LocalizedDesc desc; // Bündelt jetzt alle 3 Sprachen
};

static const ItemDescEntry sCustomItemDescs[] = {
    { ITEM_ROCS_FEATHER_SKIJER, TEXT_DESC_ROCS_FEATHER, {
        /*english*/ "Jump in ground and small jump from water.",
        /*german*/ "Sprung vom Boden und kleiner Sprung&aus dem Wasser.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_ROCS_CAPE, TEXT_DESC_ROCS_CAPE, {
        /*english*/ "Jump from ground or water. Press again&in the air for a double jump.",
        /*german*/ "Sprung vom Boden oder Wasser.&Drücke erneut in der Luft&für einen Doppelsprung.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_DESIRE_SENSOR, TEXT_DESC_DESIRE_SENSOR, {
        /*english*/ "Sense major items in this area.&Costs 3 hearts. Randomizer only.",
        /*german*/ "Erkennt wichtige Items&in diesem Gebiet.&Kostet 3 Herzen.&Nur im Randomizer.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_HYLIAS_GRACE, TEXT_DESC_HYLIAS_GRACE, {
        /*english*/ "Fairy flight for 10s. Ignores walls.&A=up, B=down, L=sprint. 12 MP.",
        /*german*/ "Feenflug für 10 Sek.&Ignoriert Wände.&A=hoch, B=runter, L=Sprint.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_ZONAI_PERMAFROST, TEXT_DESC_ZONAI_PERMAFROST, {
        /*english*/ "Stop time for 10s. Enemies, NPCs&and bosses freeze. Costs 6 magic.",
        /*german*/ "Zeit für 10 Sek. anhalten.&Gegner, NPCs und Bosse werden&eingefroren.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_DEMISE_DESTRUCTION, TEXT_DESC_DEMISE_DESTRUCTION, {
        /*english*/ "Massive AoE explosion. Damages all&enemies in range. Ground only. 6 MP.",
        /*german*/ "Gewaltige Flächenexplosion.&Verletzt alle Gegner in Reichweite.&Nur am Boden.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_DEKU_LEAF, TEXT_DESC_DEKU_LEAF, {
        /*english*/ "Ground: blow wind gust. Air: hold&to glide. Drains magic while gliding.",
        /*german*/ "Am Boden: Windstoß. In der Luft:&Halten, um zu gleiten.&Verbraucht MP beim Gleiten.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_SWITCH_HOOK, TEXT_DESC_SWITCH_HOOK, {
        /*english*/ "Aim and fire to swap positions&with objects and enemies.",
        /*german*/ "Zielen und feuern, um die Position&mit Objekten oder Gegnern zu tauschen.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_MOGMA_MITTS, TEXT_DESC_MOGMA_MITTS, {
        /*english*/ "Toggle to climb any wall.&Drains magic over time.",
        /*german*/ "Ein-/Ausschalten, um jede Wand&zu erklimmen.&Verbraucht mit der Zeit MP.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_GUST_JAR, TEXT_DESC_GUST_JAR, {
        /*english*/ "Pull enemies toward you, then push&them away. Hold C for element select.",
        /*german*/ "Gegner heranziehen und dann&wegstoßen. C halten zur Elementwahl.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_BALL_AND_CHAIN, TEXT_DESC_BALL_AND_CHAIN, {
        /*english*/ "Heavy thrown weapon. Breaks ice walls&and heavy objects. Hold C to charge.&C-Up to aim.",
        /*german*/ "Schwere Wurfwaffe. Zerstört Eiswände&und schwere Objekte. C halten&zum Aufladen. C-Hoch zum Zielen.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_WHIP, TEXT_DESC_WHIP, {
        /*english*/ "Grapple from any bar surface. Swing&with joystick. Release for momentum&launch.",
        /*german*/ "An jeder Stange schwingen.&Mit dem Stick schwingen.&Loslassen für Schwung&und weiten Sprung.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_SPINNER, TEXT_DESC_SPINNER, {
        /*english*/ "Toggle to ride. A for homing dash&attack. Breaks rocks.",
        /*german*/ "Ein-/Ausschalten zum Fahren.&A für zielverfolgenden Dash-Angriff.&Zerstört Steine.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_CANE_OF_SOMARIA, TEXT_DESC_CANE_OF_SOMARIA, {
        /*english*/ "Create statues (max 3) that press&any switch. Hookable and throwable.",
        /*german*/ "Statuen erschaffen (max. 3),&die Schalter drücken.&Mit Greifhaken nutzbar und werfbar.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_DOMINION_ROD, TEXT_DESC_DOMINION_ROD, {
        /*english*/ "Fire orb to possess Beamos, Armos&or Anubis. Control them with analog+C.",
        /*german*/ "Kugel abfeuern, um Beamos, Armos oder&Anubis zu übernehmen.&Steuerung mit Stick + C.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_TIME_GATE, TEXT_DESC_TIME_GATE, {
        /*english*/ "Travel through time. Swap between&child and adult. Costs 48 magic.",
        /*german*/ "Reise durch die Zeit. Wechsel zwischen&Kind und Erwachsen.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_BOMB_ARROWS, TEXT_DESC_BOMB_ARROWS, {
        /*english*/ "Explosive arrows. Hold C to aim.&Consumes 1 arrow and 1 bomb per shot.",
        /*german*/ "Explosive Pfeile.&C halten zum Zielen.&Verbraucht 1 Pfeil und 1 Bombe&pro Schuss.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_ROD_FIRE, TEXT_DESC_FIRE_ROD, {
        /*english*/ "Slash=3 fireballs. Stab=long shot.&Jump=flamethrower. Spin=fire AoE.&C-Up to aim.",
        /*german*/ "Hieb=3 Feuerbälle. Stich=Fernschuss.&Sprung=Flammenwerfer.&Wirbel=Feuer-AoE.&C-Hoch zum Zielen.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_ROD_ICE, TEXT_DESC_ICE_ROD, {
        /*english*/ "Slash=3 iceballs. Stab=long shot.&Jump=ice wave. Spin=ice AoE.&C-Up to aim.",
        /*german*/ "Hieb=3 Eisbälle. Stich=Fernschuss.&Sprung=Eiswelle. Wirbel=Eis-AoE.&C-Hoch zum Zielen.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_ROD_LIGHT, TEXT_DESC_LIGHT_ROD, {
        /*english*/ "Slash=3 orbs. Stab=long shot.&Jump=beam. Spin=light AoE.&C-Up to aim.",
        /*german*/ "Hieb=3 Lichtkugeln. Stich=Fernschuss.&Sprung=Lichtstrahl. Wirbel=Licht-AoE.&C-Hoch zum Zielen.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_BEETLE, TEXT_DESC_BEETLE, {
        /*english*/ "Launch remote beetle. Steer with&joystick. B=boost. Grabs items and&hits enemies.",
        /*german*/ "Fernkäfer starten.&Mit dem Stick steuern.&B=Turbo. Sammelt Items&und trifft Gegner.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_SHOVEL, TEXT_DESC_SHOVEL, {
        /*english*/ "Dig to uncover grottos, Gold&Skulltulas and graveyard rewards.",
        /*german*/ "Graben, um Grotten, Goldene Skulltulas&und Friedhofsbelohnungen zu finden.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_MINISH_CAP, TEXT_DESC_MINISH_CAP, {
        /*english*/ "Fast travel to 10 pod soil spots.&Kill Gold Skulltulas to unlock them.",
        /*german*/ "Schnellreise zu 10 weichen Böden.&Goldene Skulltulas schalten sie frei.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_LANTERN, TEXT_DESC_LANTERN, {
        /*english*/ "Swing near fire to catch it. 4 types.&Blue=melts red ice. Green=HP regen.&Poe/Green=free Lens. Swing=fire dmg.",
        /*german*/ "An Feuer schwingen zum Einfangen.&4 Arten.&Blau=schmilzt Roteis. Grün=LP-Reg.&Poe/Grün=freie Linse. Schwung=Feuer.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_CHATEAU_ROMANI, TEXT_DESC_CHATEAU_ROMANI, {
        /*english*/ "Drink for infinite magic.&One-time consumable.",
        /*german*/ "Trinken für unendliche Magie.&Einmaliger Verbrauch.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_POKEBALL, TEXT_DESC_POKEBALL, {
        /*english*/ "Transform into Pikachu.&Press again to revert.",
        /*german*/ "Verwandle dich in Pikachu.&Erneut drücken zum Rückverwandeln.",
        /*french*/ TODO_TRANSLATE
} },
};

static const ItemDescEntry sMaskDescs[] = {
    { ITEM_MM_MASK_ALL_NIGHT, TEXT_DESC_MASK_ALL_NIGHT, {
        /*english*/ "Spawns night-only Gold Skulltulas&during daytime.",
        /*german*/ "Erzeugt nachts erscheinende Goldene&Skulltulas auch am Tag.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_MM_MASK_BLAST, TEXT_DESC_MASK_BLAST, {
        /*english*/ "Press B for instant explosion at&your feet. Has cooldown.",
        /*german*/ "B drücken für sofortige Explosion&zu deinen Füßen. Hat Abklingzeit.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_MM_MASK_STONE, TEXT_DESC_MASK_STONE, {
        /*english*/ "Enemies ignore you completely.",
        /*german*/ "Gegner ignorieren dich vollständig.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_MM_MASK_GREAT_FAIRY, TEXT_DESC_MASK_GREAT_FAIRY, {
        /*english*/ "In fountain: A=claim reward.&B=teleport menu between fountains.",
        /*german*/ "Am Feenbrunnen: A=Belohnung.&B=Teleportmenü zwischen Brunnen.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_MM_MASK_DEKU, TEXT_DESC_MASK_DEKU, {
        /*english*/ "Transform into Deku form.&Full moveset from Majora's Mask.",
        /*german*/ "Verwandle dich in Deku-Link.&Kompletter Moveset aus Majora's Mask.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_MM_MASK_BUNNY, TEXT_DESC_MASK_BUNNY, {
        /*english*/ "Run 1.5x faster.",
        /*german*/ "1,5-fache höhere Laufgeschwindigkeit.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_MM_MASK_DON_GERO, TEXT_DESC_MASK_DON_GERO, {
        /*english*/ "At Zora's River frog log: A=collect&all frog rewards at once.",
        /*german*/ "Am Froschstamm im Zora-Fluss:&A=sammelt alle&Froschbelohnungen auf einmal.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_MM_MASK_GORON, TEXT_DESC_MASK_GORON, {
        /*english*/ "Transform into Goron form.&Full moveset from Majora's Mask.",
        /*german*/ "Verwandle dich in Goron-Link.&Kompletter Moveset aus Majora's Mask.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_MM_MASK_ROMANI, TEXT_DESC_MASK_ROMANI, {
        /*english*/ "Get milk from cows without&Epona's Song.",
        /*german*/ "Milch von Kühen ohne&Eponas Lied erhalten.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_MM_MASK_COUPLE, TEXT_DESC_MASK_COUPLE, {
        /*english*/ "Passive regen. Day=HP recovery.&Night=MP recovery.",
        /*german*/ "Passive Regeneration.&Tag=LP. Nacht=MP.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_MM_MASK_ZORA, TEXT_DESC_MASK_ZORA, {
        /*english*/ "Transform into Zora form.&Full moveset from Majora's Mask.",
        /*german*/ "Verwandle dich in Zora-Link.&Kompletter Moveset aus Majora's Mask.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_MM_MASK_KAMARO, TEXT_DESC_MASK_KAMARO, {
        /*english*/ "Hold A to dance. Dance near&Darunia for reward.",
        /*german*/ "A halten zum Tanzen.&Tanze neben Darunia für eine Belohnung.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_MM_MASK_CAPTAIN, TEXT_DESC_MASK_CAPTAIN, {
        /*english*/ "Spawns Stalchildren (child) or Stalfos&(adult) at night in Hyrule Field.",
        /*german*/ "Erzeugt nachts Stal-Kinder (Kind)&oder Stalfos (Erwachsen) in der Ebene.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_MM_MASK_FIERCE_DEITY, TEXT_DESC_MASK_FIERCE_DEITY, {
        /*english*/ "Transform into Fierce Deity form.&Full moveset from Majora's Mask.",
        /*german*/ "Verwandle dich in den Grimmigen Gott.&Kompletter Moveset aus Majora's Mask.",
        /*french*/ TODO_TRANSLATE
} },
};

static const ItemDescEntry sSw97ArrowDescs[] = {
    { ITEM_SW97_ARROW_FIRE, TEXT_DESC_SW97_ARROW_FIRE, {
        /*english*/ "Fire elemental arrow. 4 MP per shot.",
        /*german*/ "Feuer Elementarpfeil. 4 MP pro Schuss.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_SW97_ARROW_ICE, TEXT_DESC_SW97_ARROW_ICE, {
        /*english*/ "Ice elemental arrow. 4 MP per shot.",
        /*german*/ "Eis Elementarpfeil. 4 MP pro Schuss.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_SW97_ARROW_LIGHT, TEXT_DESC_SW97_ARROW_LIGHT, {
        /*english*/ "Light elemental arrow. 8 MP per shot.",
        /*german*/ "Licht Elementarpfeil. 8 MP pro Schuss.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_SW97_ARROW_DARK, TEXT_DESC_SW97_ARROW_DARK, {
        /*english*/ "Dark elemental arrow. 4 MP per shot.",
        /*german*/ "Dunkel Elementarpfeil. 4 MP pro Schuss.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_SW97_ARROW_SOUL, TEXT_DESC_SW97_ARROW_SOUL, {
        /*english*/ "Soul elemental arrow. 4 MP per shot.",
        /*german*/ "Seelen Elementarpfeil. 4 MP pro Schuss.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_SW97_ARROW_WIND, TEXT_DESC_SW97_ARROW_WIND, {
        /*english*/ "Wind elemental arrow. 4 MP per shot.",
        /*german*/ "Wind Elementarpfeil. 4 MP pro Schuss.",
        /*french*/ TODO_TRANSLATE
} },
};

static const ItemDescEntry sExtEquipDescs[] = {
    { ITEM_EXT_SWORD_1, TEXT_DESC_EXT_BYRNA, {
        /*english*/ "BGS reach. Recover HP+MP on&melee hit.",
        /*german*/ "Reichweite des Biggoron-Schwerts.&Stellt LP+MP bei Nahkampftreffern&wieder her.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_EXT_SWORD_2, TEXT_DESC_EXT_FOUR_SWORD, {
        /*english*/ "R+B to charge. Spawns 3 clones&(18 MP). Clones mirror your attacks.",
        /*german*/ "R+B zum Aufladen. Erzeugt 3 Klone&(18 MP). Klone spiegeln deine Angriffe.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_EXT_SWORD_3, TEXT_DESC_EXT_IK_AXE, {
        /*english*/ "Hammer attacks. 2x damage, 2x reach.&Slower walk. Hold B to throw.",
        /*german*/ "Hammerangriffe.&Doppelter Schaden, doppelte Reichweite.&Langsameres Gehen.&B halten zum Werfen.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_EXT_SHIELD_1, TEXT_DESC_EXT_DIVINE_SHIELD, {
        /*english*/ "Fire immune. Block within 10 frames&to stun all nearby enemies.",
        /*german*/ "Feuerimmun. Blocke innerhalb von&10 Frames,&um alle nahen Gegner zu betäuben.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_EXT_SHIELD_2, TEXT_DESC_EXT_GERUDO_SCIMITAR, {
        /*english*/ "Surfing shield. (coming soon)",
        /*german*/ "Schild-Surfen. (kommt bald)",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_EXT_SHIELD_3, TEXT_DESC_EXT_SHIELD_IKANA, {
        /*english*/ "Perfect guard drains enemy HP.&Death save: revive once with 3 hearts.",
        /*german*/ "Perfektes Blocken entzieht Gegnern LP.&Todesrettung:&Einmal mit 3 Herzen wiederbeleben.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_EXT_TUNIC_1, TEXT_DESC_EXT_MAGIC_CAPE, {
        /*english*/ "Ganondorf's cape. Reduces magic&cost by half.",
        /*german*/ "Halbiert den Magieverbrauch.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_EXT_TUNIC_2, TEXT_DESC_EXT_BREASTPLATE, {
        /*english*/ "Damage immunity. Costs rupees per&hit. No rupees = slow movement.",
        /*german*/ "Schadensimmunität.&Kostet Rubine pro Treffer.&Ohne Rubine = langsame Bewegung.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_EXT_TUNIC_3, TEXT_DESC_EXT_CHAMPION_TUNIC, {
        /*english*/ "Flurry Rush on dodge. Bullet Time&when aiming in air. 15% world speed.",
        /*german*/ "Flurry Rush nach Ausweichen. Zeitlupe&beim Zielen in der Luft. 15 % Weltzeit.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_EXT_BOOTS_1, TEXT_DESC_EXT_PEGASUS_ANKLET, {
        /*english*/ "Hold B to dash with sword. Wind&barrier drains 1 MP/15 frames.",
        /*german*/ "B halten für Schwert-Dash.&Windbarriere verbraucht 1 MP/15 Frames.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_EXT_BOOTS_2, TEXT_DESC_EXT_PENDANT_MEMORIES, {
        /*english*/ "Mortal Draw near enemies. Ground&Pound in air. Parry Leap after 3&side hops.",
        /*german*/ "Todeshieb nahe Gegnern.&Bodenstampfer in der Luft.&Parier-Sprung nach 3 Seitwärtssprüngen.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_EXT_BOOTS_3, TEXT_DESC_EXT_WATER_DRAGON_SCALE, {
        /*english*/ "Zora swim. Barrel roll, dolphin jump.&Adult only.",
        /*german*/ "Zora-Schwimmen.&Fassrolle, Delfinsprung.&Nur Erwachsener.",
        /*french*/ TODO_TRANSLATE
} },
};

static const ItemDescEntry sMedallionDescs[] = {
    { ITEM_MEDALLION_FOREST, TEXT_DESC_MEDALLION_FOREST, {
        /*english*/ "Wind spell. 6 MP.&L to switch to arrow mode.",
        /*german*/ "Windzauber. 6 MP.&L zum Wechsel in Pfeilmodus.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_MEDALLION_FIRE, TEXT_DESC_MEDALLION_FIRE, {
        /*english*/ "Fire spell. 6 MP.&L to switch to arrow mode.",
        /*german*/ "Feuerzauber. 6 MP.&L zum Wechsel in Pfeilmodus.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_MEDALLION_WATER, TEXT_DESC_MEDALLION_WATER, {
        /*english*/ "Ice spell. 6 MP.&L to switch to arrow mode.",
        /*german*/ "Eiszauber. 6 MP.&L zum Wechsel in Pfeilmodus.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_MEDALLION_SPIRIT, TEXT_DESC_MEDALLION_SPIRIT, {
        /*english*/ "Soul spell. 12 MP.&L to switch to arrow mode.",
        /*german*/ "Seelenzauber. 12 MP.&L zum Wechsel in Pfeilmodus.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_MEDALLION_SHADOW, TEXT_DESC_MEDALLION_SHADOW, {
        /*english*/ "Dark spell. 12 MP.&L to switch to arrow mode.",
        /*german*/ "Dunkelzauber. 12 MP.&L zum Wechsel in Pfeilmodus.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_MEDALLION_LIGHT, TEXT_DESC_MEDALLION_LIGHT, {
        /*english*/ "Light spell. 12 MP.&L to switch to arrow mode.",
        /*german*/ "Lichtzauber. 12 MP.&L zum Wechsel in Pfeilmodus.",
        /*french*/ TODO_TRANSLATE
} },
};

static const ItemDescEntry sVanillaItemDescs[] = {
    { ITEM_STICK, TEXT_DESC_V_STICK, {
        /*english*/ "Deku Stick. Melee weapon that&lights from fire. Burns up fast.",
        /*german*/ "Nahkampfwaffe, die sich an Feuer&entzündet. Brennt schnell ab.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_NUT, TEXT_DESC_V_NUT, {
        /*english*/ "Deku Nut. Throw to stun enemies&and flash-blind nearby foes.",
        /*german*/ "Werfen, um Gegner zu betäuben&und nahe Feinde zu blenden.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_BOMB, TEXT_DESC_V_BOMB, {
        /*english*/ "Throw to blow up walls, enemies&and obstacles. Short fuse.",
        /*german*/ "Werfen, um Wände, Gegner und&Hindernisse zu sprengen.&Kurze Zündschnur.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_BOW, TEXT_DESC_V_BOW, {
        /*english*/ "Fire arrows. Hold C to aim.&Buy more arrows in shops.",
        /*german*/ "Pfeile schießen. C halten zum Zielen.&Weitere Pfeile in Läden kaufen.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_ARROW_FIRE, TEXT_DESC_V_ARROW_FIRE, {
        /*english*/ "Fire Arrow. Burns enemies and&lights torches. Costs magic.",
        /*german*/ "Verbrennt Geg. und entzündet Fackeln.&Kostet Magie.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_DINS_FIRE, TEXT_DESC_V_DINS_FIRE, {
        /*english*/ "Ring of flame around you. Burns&foes and lights torches. 6 MP.",
        /*german*/ "Feuerring um dich. Verbrennt&Gegner und entzündet Fackeln.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_SLINGSHOT, TEXT_DESC_V_SLINGSHOT, {
        /*english*/ "Child ranged weapon. Fires Deku&Seeds. Hold C to aim.",
        /*german*/ "Fernwaffe für Kinder.&Schießt Deku-Samen.&C halten zum Zielen.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_OCARINA_FAIRY, TEXT_DESC_V_OCARINA_FAIRY, {
        /*english*/ "Play songs to trigger magic.&Saria's Fairy Ocarina.",
        /*german*/ "Spiele Lieder, um Magie auszulösen.&Sarias Feen-Okarina.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_OCARINA_TIME, TEXT_DESC_V_OCARINA_TIME, {
        /*english*/ "Play songs to trigger magic.&The royal Ocarina of Time.",
        /*german*/ "Spiele Lieder, um Magie auszulösen.&Die königliche Okarina der Zeit.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_BOMBCHU, TEXT_DESC_V_BOMBCHU, {
        /*english*/ "Wind-up bomb that crawls along&floors and walls, then explodes.",
        /*german*/ "Aufziehbombe, die über Böden&und Wände kriecht und dann explodiert.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_HOOKSHOT, TEXT_DESC_V_HOOKSHOT, {
        /*english*/ "Fire to grab targets and pull&yourself in, or items to you.",
        /*german*/ "Abfeuern, um Ziele zu greifen&und dich oder Items heranzuziehen.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_LONGSHOT, TEXT_DESC_V_LONGSHOT, {
        /*english*/ "Like the Hookshot but with&twice the reach.",
        /*german*/ "Wie der Fanghaken, aber&mit doppelter Reichweite.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_ARROW_ICE, TEXT_DESC_V_ARROW_ICE, {
        /*english*/ "Ice Arrow. Freezes enemies&solid. Costs magic per shot.",
        /*german*/ "Friert Gegner komplett ein.&Kostet Magie.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_FARORES_WIND, TEXT_DESC_V_FARORES_WIND, {
        /*english*/ "Set a warp point, then teleport&back to it later. 6 MP.",
        /*german*/ "Setze einen Teleportpunkt und&kehre später dorthin zurück.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_BOOMERANG, TEXT_DESC_V_BOOMERANG, {
        /*english*/ "Throw to stun foes and grab&distant items. Returns to you.",
        /*german*/ "Werfen, um Gegner zu betäuben&und entfernte Items zu holen.&Kehrt zurück.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_LENS, TEXT_DESC_V_LENS, {
        /*english*/ "Lens of Truth. Reveals hidden&things and invisible foes. Drains MP.",
        /*german*/ "Enthüllt verborgene Dinge&und unsichtbare Gegner.&Verbraucht MP.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_BEAN, TEXT_DESC_V_BEAN, {
        /*english*/ "Magic Bean. Plant in soft soil&to grow a ride. 10 total.",
        /*german*/ "In weichen Boden pflanzen,&um eine Ranke wachsen zu lassen.&Maximal 10.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_HAMMER, TEXT_DESC_V_HAMMER, {
        /*english*/ "Megaton Hammer. Smash rusty&switches, posts and armor.",
        /*german*/ "Zerschmettert rostige Schalter,&Pfähle und Rüstungen.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_ARROW_LIGHT, TEXT_DESC_V_ARROW_LIGHT, {
        /*english*/ "Light Arrow. Devastating holy&damage. High magic cost.",
        /*german*/ "Verheerender heiliger Schaden.&Hohe Magiekosten.",
        /*french*/ TODO_TRANSLATE
    } },
    { ITEM_NAYRUS_LOVE, TEXT_DESC_V_NAYRUS_LOVE, {
        /*english*/ "Protective barrier that blocks&all damage for a time. 12 MP.",
        /*german*/ "Schutzbarriere, die für kurze Zeit&allen Schaden blockiert.",
        /*french*/ TODO_TRANSLATE
} },
};

// ---------------------------------------------------------------------------
// Lookup: item ID + page -> text ID (or 0)
// ---------------------------------------------------------------------------

extern "C" u16 PauseItemDesc_GetTextId(u16 cursorItem, s32 pageIndex) {
    // Custom items + masks + SW97 arrows on ITEM pages
    if (pageIndex == PAUSE_ITEM) {
        for (size_t i = 0; i < ARRAY_COUNT(sCustomItemDescs); i++) {
            if (sCustomItemDescs[i].itemId == cursorItem)
                return sCustomItemDescs[i].textId;
        }
        for (size_t i = 0; i < ARRAY_COUNT(sMaskDescs); i++) {
            if (sMaskDescs[i].itemId == cursorItem)
                return sMaskDescs[i].textId;
        }
        for (size_t i = 0; i < ARRAY_COUNT(sSw97ArrowDescs); i++) {
            if (sSw97ArrowDescs[i].itemId == cursorItem)
                return sSw97ArrowDescs[i].textId;
        }
        for (size_t i = 0; i < ARRAY_COUNT(sVanillaItemDescs); i++) {
            if (sVanillaItemDescs[i].itemId == cursorItem)
                return sVanillaItemDescs[i].textId;
        }
    }

    // Extended equipment on EQUIP page
    if (pageIndex == PAUSE_EQUIP) {
        for (size_t i = 0; i < ARRAY_COUNT(sExtEquipDescs); i++) {
            if (sExtEquipDescs[i].itemId == cursorItem)
                return sExtEquipDescs[i].textId;
        }
    }

    // SW97 Medallions on QUEST page (only when SW97 enabled)
    if (pageIndex == PAUSE_QUEST && SW97_MEDALLIONS_ENABLED()) {
        for (size_t i = 0; i < ARRAY_COUNT(sMedallionDescs); i++) {
            if (sMedallionDescs[i].itemId == cursorItem)
                return sMedallionDescs[i].textId;
        }
    }

    return 0;
}

// ---------------------------------------------------------------------------
// Message hook: build and load description into font
// ---------------------------------------------------------------------------

static void BuildDescMessage(const LocalizedDesc& desc, uint16_t* textId, bool* loadFromMessageTable) {
    // Hier werden die drei Sprachen separat an Ship of Harkinian übergeben:
    CustomMessage msg = CustomMessage(desc.english, desc.german, desc.french);
    msg.Format();
    msg.LoadIntoFont();
    *loadFromMessageTable = false;
}

// All description tables for single-hook lookup
static const ItemDescEntry* sAllDescs[] = {
    sCustomItemDescs, sMaskDescs, sSw97ArrowDescs, sExtEquipDescs, sMedallionDescs, sVanillaItemDescs,
};
static const size_t sAllDescCounts[] = {
    ARRAY_COUNT(sCustomItemDescs), ARRAY_COUNT(sMaskDescs),      ARRAY_COUNT(sSw97ArrowDescs),
    ARRAY_COUNT(sExtEquipDescs),   ARRAY_COUNT(sMedallionDescs), ARRAY_COUNT(sVanillaItemDescs),
};

// Single hook for all descriptions: fires on ANY OnOpenText, checks if textId matches
static void OnOpenTextDescHook(uint16_t* textId, bool* loadFromMessageTable) {
    for (size_t t = 0; t < ARRAY_COUNT(sAllDescs); t++) {
        for (size_t i = 0; i < sAllDescCounts[t]; i++) {
            if (sAllDescs[t][i].textId == *textId) {
                BuildDescMessage(sAllDescs[t][i].desc, textId, loadFromMessageTable);
                return;
            }
        }
    }
}

// Register all description hooks
static void RegisterPauseItemDescriptions() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnOpenText>(OnOpenTextDescHook);
}

static RegisterShipInitFunc initPauseDescs(RegisterPauseItemDescriptions);
