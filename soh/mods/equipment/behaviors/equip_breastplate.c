/**
 * equip_breastplate.c - Spirit Breastplate (Extended Tunic Slot 2)
 *
 * Behavior: Magic Armor (TP-style) — rupee-cost damage immunity.
 * - Makes Link immune to all damage while wearing (rupees > 0)
 * - Each HP of damage received costs 1 rupee
 * - No rupees = slow movement (cursed weight)
 * - With rupees: golden Iron Knuckle tint (Nabooru variant)
 * - Without rupees: dark Iron Knuckle tint
 * - Passive rupee drain: 1 rupee per 30 frames
 *
 * Damage immunity is implemented via a direct C call from Health_ChangeBy
 * in z_parameter.c to Breastplate_OnHealthChangeBefore (defined below).
 * Setting `player->invincibilityTimer = -1` from this behavior runs too late
 * in Player_Update (UpdateCommon ticks the timer back to 0 before the damage
 * handler runs), so we intercept at the Health_ChangeBy level instead.
 *
 * Included by ext_equip_behavior.c (unity build).
 */

// No extra includes — unity-built from ext_equip_behavior.c
extern void Rupees_ChangeBy(s16 rupeeChange);
u8 Breastplate_IsActive(void);

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
#define BREASTPLATE_RUPEE_INTERVAL 30 // Passive drain: 1 rupee every N frames
#define BREASTPLATE_SLOW_MULT 0.5f    // Speed multiplier when broke

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------
static s16 sBreastplateRupeeTick = 0;

// ---------------------------------------------------------------------------
// Main Behavior — runs every frame from ExtEquip_UpdateBehavior
// Handles passive rupee drain and the broke-mode movement penalty.
// Damage interception is handled separately by Breastplate_OnHealthChangeBefore.
// ---------------------------------------------------------------------------
static void Breastplate_Behavior(Player* player, PlayState* play) {
    // Skip during cutscenes, dying, etc.
    if (player->stateFlags1 & (PLAYER_STATE1_DEAD | PLAYER_STATE1_IN_CUTSCENE | PLAYER_STATE1_LOADING |
                               PLAYER_STATE1_IN_ITEM_CS | PLAYER_STATE1_GETTING_ITEM)) {
        return;
    }

    if (gSaveContext.rupees > 0) {
        sBreastplateRupeeTick++;
        if (sBreastplateRupeeTick >= BREASTPLATE_RUPEE_INTERVAL) {
            sBreastplateRupeeTick = 0;
            Rupees_ChangeBy(-1);
        }
    } else {
        // No rupees: heavy and slow
        sBreastplateRupeeTick = 0;
        player->linearVelocity *= BREASTPLATE_SLOW_MULT;
        player->actor.speedXZ *= BREASTPLATE_SLOW_MULT;
    }
}

// ---------------------------------------------------------------------------
// Pre-damage hook: convert incoming damage to rupee cost while breastplate
// is active and Link has rupees.
//
// Called directly from Health_ChangeBy in z_parameter.c BEFORE health is
// mutated. Setting *amount = 0 makes Health_ChangeBy return early without
// touching gSaveContext.health.
// ---------------------------------------------------------------------------
void Breastplate_OnHealthChangeBefore(int16_t* amount) {
    if (!Breastplate_IsActive()) {
        return;
    }
    if (*amount >= 0) {
        return; // healing — pass through
    }
    if (gSaveContext.rupees <= 0) {
        return; // broke — take damage normally
    }

    s16 damageHP = -*amount;
    s16 rupeeCost = damageHP;
    if (rupeeCost > gSaveContext.rupees) {
        rupeeCost = (s16)gSaveContext.rupees;
    }
    Rupees_ChangeBy(-rupeeCost);
    Sfx_PlaySfxCentered(NA_SE_IT_SHIELD_BOUND);

    *amount = 0; // block the damage
}

// ---------------------------------------------------------------------------
// Tunic color tint: gold (has rupees) or dark (broke)
// Called from the tunic color system to override Link's tunic color
// ---------------------------------------------------------------------------
u8 Breastplate_IsActive(void) {
    return ExtEquip_IsEnabled() && gExtEquipState.currentExtTunic == 2;
}

// Returns 1 if player has rupees (gold mode), 0 if broke (dark mode)
u8 Breastplate_HasPower(void) {
    return gSaveContext.rupees > 0;
}
