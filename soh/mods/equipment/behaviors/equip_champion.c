/**
 * equip_champion.c - Champion's Tunic (Extended Tunic Slot 3)
 *
 * Features:
 *  1. Flurry Rush: rising-edge PLAYER_STATE2_HOPPING + ENEMY/BOSS within
 *     range → world slows to 15%, Link gets iframes, up to 7-hit window.
 *  2. Bullet Time: use aimable item while airborne → world slows to 15%,
 *     Link floats, analog stick controls pitch+yaw (Zora boomerang style).
 *     Camera follows behind Link. Items fire in aimed direction.
 *
 * Slow-motion via gChampionSlowFactor (z_actor.c Actor_UpdatePos).
 * Screen tint via play->envCtx.fillScreen + screenFillColor[].
 * Champion_Cleanup() takes PlayState* so it can clear the tint on unequip.
 *
 * Included by ext_equip_behavior.c (unity build).
 */

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
#define CHAMPION_FLURRY_DURATION 120 // real frames the slow window lasts (~2s)
#define CHAMPION_FLURRY_HIT_MAX 7    // hits that end the window early
#define CHAMPION_SLOW_FACTOR 0.15f   // world speed multiplier during both modes
#define CHAMPION_ENEMY_RANGE 150.0f  // units to scan for enemies/bosses at dodge time
#define CHAMPION_SCREEN_FLASH 5      // initial bright-tint burst frames
#define CHAMPION_BULLET_FLOAT 1.15f  // velocity.y counterforce each frame (net fall ≈ -0.05/frame, near-suspension)
#define CHAMPION_AIM_SENSITIVITY 10  // stick-to-rotation scale (s8 stick * this = s16 delta/frame)
#define CHAMPION_YAW_LIMIT 0x5555    // ±120° yaw range from initial facing
#define CHAMPION_PITCH_LIMIT 0x2000  // ±45° pitch range
#define CHAMPION_TINT_ALPHA 30       // subtle blue tint (BOTW has no heavy overlay)

#ifndef BGCHECKFLAG_GROUND
#define BGCHECKFLAG_GROUND 0x0001
#endif

// Forward declarations (defined later in z_player.c unity build)
extern void Player_SetIntangibility(Player* player, s32 timer);

// ---------------------------------------------------------------------------
// State machine
// ---------------------------------------------------------------------------
typedef enum {
    CHAMPION_IDLE,
    CHAMPION_FLURRY_RUSH,
    CHAMPION_BULLET_TIME,
} ChampionState;

// ---------------------------------------------------------------------------
// Module-level statics
// ---------------------------------------------------------------------------
static ChampionState sChampionState = CHAMPION_IDLE;
static s16 sChampionTimer = 0;
static u8 sChampionHitCount = 0;
static u8 sPrevHopping = 0; // for rising-edge detection
static s16 sScreenFlashTimer = 0;
static s16 sLockedYaw = 0; // Link's yaw when Bullet Time starts
static s16 sAimYaw = 0;    // relative yaw offset from locked direction
static s16 sAimPitch = 0;  // relative pitch offset (up/down)

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/**
 * Returns 1 if Link is holding any first-person aimable item:
 * bow variants (0x08-0x0E), slingshot (0x0F), hookshot/longshot (0x10-0x11),
 * boomerang (0x14).
 */
static u8 Champion_IsAimableItem(Player* player) {
    PlayerItemAction ia = player->heldItemAction;
    if (ia >= PLAYER_IA_BOW && ia <= PLAYER_IA_LONGSHOT)
        return 1; // bow..sling..hookshot..longshot
    if (ia == PLAYER_IA_BOOMERANG)
        return 1;
    return 0;
}

/**
 * Returns 1 if any ENEMY or BOSS actor is within CHAMPION_ENEMY_RANGE in the
 * same room as the player. Only these categories trigger Flurry Rush — doors,
 * chests, NPCs and props are intentionally excluded.
 */
static u8 Champion_EnemyNearby(Player* player, PlayState* play) {
    static const s32 sCats[] = { ACTORCAT_ENEMY, ACTORCAT_BOSS };
    s32 i;
    for (i = 0; i < 2; i++) {
        Actor* a = play->actorCtx.actorLists[sCats[i]].head;
        while (a != NULL) {
            if (a->room == player->actor.room) {
                if (Math_Vec3f_DistXYZ(&a->world.pos, &player->actor.world.pos) <= CHAMPION_ENEMY_RANGE) {
                    return 1;
                }
            }
            a = a->next;
        }
    }
    return 0;
}

/**
 * Set or clear the screen tint.
 */
static void Champion_SetScreenTint(PlayState* play, u8 golden, u8 alpha) {
    if (alpha == 0) {
        play->envCtx.fillScreen = false;
        play->envCtx.screenFillColor[3] = 0;
        return;
    }
    play->envCtx.fillScreen = true;
    if (golden) {
        play->envCtx.screenFillColor[0] = 220;
        play->envCtx.screenFillColor[1] = 180;
        play->envCtx.screenFillColor[2] = 40;
    } else {
        play->envCtx.screenFillColor[0] = 6;
        play->envCtx.screenFillColor[1] = 24;
        play->envCtx.screenFillColor[2] = 66;
    }
    play->envCtx.screenFillColor[3] = alpha;
}

// ---------------------------------------------------------------------------
// State transitions
// ---------------------------------------------------------------------------

static void Champion_EnterFlurry(Player* player, PlayState* play) {
    sChampionState = CHAMPION_FLURRY_RUSH;
    sChampionTimer = CHAMPION_FLURRY_DURATION;
    sChampionHitCount = 0;

    gChampionSlowFactor = CHAMPION_SLOW_FACTOR;
    Player_SetIntangibility(player, CHAMPION_FLURRY_DURATION);

    sScreenFlashTimer = CHAMPION_SCREEN_FLASH;
    Champion_SetScreenTint(play, 1, 200);

    Audio_PlaySoundGeneral(NA_SE_SY_ATTENTION_ON, &player->actor.world.pos, 4, &gSfxDefaultFreqAndVolScale,
                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
}

static void Champion_ExitFlurry(PlayState* play) {
    sChampionState = CHAMPION_IDLE;
    sChampionTimer = 0;
    sChampionHitCount = 0;
    gChampionSlowFactor = 1.0f;
    Champion_SetScreenTint(play, 1, 0);
}

static void Champion_EnterBulletTime(Player* player, PlayState* play) {
    sChampionState = CHAMPION_BULLET_TIME;
    gChampionSlowFactor = CHAMPION_SLOW_FACTOR;
    player->actor.speedXZ = 0.0f;
    player->linearVelocity = 0.0f;
    sLockedYaw = player->actor.shape.rot.y;
    sAimYaw = 0;
    sAimPitch = 0;
    Champion_SetScreenTint(play, 0, CHAMPION_TINT_ALPHA);
}

static void Champion_ExitBulletTime(Player* player, PlayState* play) {
    (void)player;
    sChampionState = CHAMPION_IDLE;
    gChampionSlowFactor = 1.0f;
    Champion_SetScreenTint(play, 0, 0);
}

static void Champion_OnMeleeHit(Player* player, PlayState* play) {
    (void)player;
    if (sChampionState != CHAMPION_FLURRY_RUSH) {
        return;
    }
    sChampionHitCount++;
    if (sChampionHitCount >= CHAMPION_FLURRY_HIT_MAX) {
        Champion_ExitFlurry(play);
    }
}

// ---------------------------------------------------------------------------
// Per-frame behavior
// ---------------------------------------------------------------------------
static void Champion_Behavior(Player* player, PlayState* play) {
    // ---- Guard: clean exit during cutscenes / death / loading --------------
    u32 blockedFlags = PLAYER_STATE1_DEAD | PLAYER_STATE1_IN_CUTSCENE | PLAYER_STATE1_LOADING |
                       PLAYER_STATE1_IN_ITEM_CS | PLAYER_STATE1_GETTING_ITEM;
    if (player->stateFlags1 & blockedFlags) {
        if (sChampionState == CHAMPION_FLURRY_RUSH) {
            Champion_ExitFlurry(play);
        } else if (sChampionState == CHAMPION_BULLET_TIME) {
            Champion_ExitBulletTime(player, play);
        }
        sPrevHopping = 0;
        return;
    }

    // ---- Screen flash fade -------------------------------------------------
    if (sScreenFlashTimer > 0) {
        sScreenFlashTimer--;
        if (sScreenFlashTimer == 0 && sChampionState == CHAMPION_FLURRY_RUSH) {
            Champion_SetScreenTint(play, 1, 50);
        }
    }

    // ---- Per-frame reads ---------------------------------------------------
    u8 curHopping = (player->stateFlags2 & PLAYER_STATE2_HOPPING) != 0;
    u8 onGround = (player->actor.bgCheckFlags & BGCHECKFLAG_GROUND) != 0;

    // ---- State machine -----------------------------------------------------
    switch (sChampionState) {

        case CHAMPION_IDLE: {
            if (!onGround && CHECK_BTN_ALL(play->state.input[0].cur.button, BTN_Z) &&
                (player->unk_6AD == 2 || Champion_IsAimableItem(player))) {
                Champion_EnterBulletTime(player, play);
                break;
            }
            u8 risingEdge = curHopping && !sPrevHopping;
            if (risingEdge && Champion_EnemyNearby(player, play)) {
                Champion_EnterFlurry(player, play);
            }
            break;
        }

        case CHAMPION_FLURRY_RUSH: {
            if (sChampionTimer > 0) {
                sChampionTimer--;
                Player_SetIntangibility(player, sChampionTimer);
            }
            if (sChampionTimer <= 0) {
                Champion_ExitFlurry(play);
            }
            break;
        }

        case CHAMPION_BULLET_TIME: {
            u8 zHeld = CHECK_BTN_ALL(play->state.input[0].cur.button, BTN_Z);
            u8 cancel = onGround || !zHeld;
            if (cancel) {
                Champion_ExitBulletTime(player, play);
                break;
            }

            player->actor.velocity.y = CHAMPION_BULLET_FLOAT;
            player->unk_6AD = 2;

            s8 stickX = play->state.input[0].cur.stick_x;
            s8 stickY = play->state.input[0].cur.stick_y;

            if (ABS(stickX) > 10)
                sAimYaw -= stickX * CHAMPION_AIM_SENSITIVITY;
            if (ABS(stickY) > 10)
                sAimPitch += stickY * CHAMPION_AIM_SENSITIVITY;

            if (sAimYaw > (s16)CHAMPION_YAW_LIMIT)
                sAimYaw = (s16)CHAMPION_YAW_LIMIT;
            if (sAimYaw < -(s16)CHAMPION_YAW_LIMIT)
                sAimYaw = -(s16)CHAMPION_YAW_LIMIT;
            if (sAimPitch > (s16)CHAMPION_PITCH_LIMIT)
                sAimPitch = (s16)CHAMPION_PITCH_LIMIT;
            if (sAimPitch < -(s16)CHAMPION_PITCH_LIMIT)
                sAimPitch = -(s16)CHAMPION_PITCH_LIMIT;

            player->actor.shape.rot.y = sLockedYaw + sAimYaw;
            player->yaw = player->actor.shape.rot.y;
            player->upperLimbRot.x = sAimPitch;
            player->actor.focus.rot.y = player->actor.shape.rot.y;
            player->actor.focus.rot.x = sAimPitch;

            Champion_SetScreenTint(play, 0, CHAMPION_TINT_ALPHA);
            break;
        }
    }

    sPrevHopping = curHopping;
}

// ---------------------------------------------------------------------------
// Cleanup
// ---------------------------------------------------------------------------
static void Champion_Cleanup(PlayState* play) {
    gChampionSlowFactor = 1.0f;

    if (play != NULL) {
        Champion_SetScreenTint(play, 0, 0);
    }

    sChampionState = CHAMPION_IDLE;
    sChampionTimer = 0;
    sChampionHitCount = 0;
    sPrevHopping = 0;
    sScreenFlashTimer = 0;
    sLockedYaw = 0;
    sAimYaw = 0;
    sAimPitch = 0;
}