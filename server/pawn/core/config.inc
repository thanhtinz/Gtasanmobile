/**
 * Compile-time configuration.
 *
 * A note on text: the SA-MP client renders through its own bitmap charset,
 * which has no Vietnamese diacritics — on PC and on the Android client alike.
 * Every in-game string in this gamemode is therefore written unaccented so it
 * stays readable. Comments and documentation use normal text.
 */

#if defined _gtasan_config_included
	#endinput
#endif
#define _gtasan_config_included

#define GM_NAME                 "GTA:SA Mobile Online"
#define GM_MODE_TEXT            "Freeroam RP v0.1"
#define GM_VERSION              "0.1.0"

#define GM_DEFAULT_WEATHER      (2)

// --- economy ----------------------------------------------------------------
#define GM_START_CASH           (5000)
#define GM_START_BANK           (25000)
#define GM_MAX_CASH             (999999999)

// Cash carried on the body is partly lost on death; banked money never is.
// That difference is the entire reason the bank is worth walking to.
#define GM_DEATH_CASH_LOSS      (0.10)

// --- persistence ------------------------------------------------------------
#define GM_SAVE_INTERVAL_MS     (120000)
#define GM_PAYDAY_INTERVAL_MS   (600000)

// --- accounts ---------------------------------------------------------------
#define GM_MIN_PASSWORD_LEN     (4)
#define GM_MAX_PASSWORD_LEN     (64)
#define GM_LOGIN_ATTEMPTS       (3)

// SHA256_PassHash emits 64 hex characters.
#define GM_HASH_LEN             (65)
#define GM_SALT_LEN             (17)

// --- world ------------------------------------------------------------------
// Grove Street. New characters start here, and it is the fallback whenever a
// stored position looks corrupt.
#define SPAWN_DEFAULT_X         (2495.0)
#define SPAWN_DEFAULT_Y         (-1687.0)
#define SPAWN_DEFAULT_Z         (13.5)
#define SPAWN_DEFAULT_A         (0.0)

// Camera parked over Los Santos while the login dialog is up.
#define LOGIN_CAM_X             (1543.0)
#define LOGIN_CAM_Y             (-1676.0)
#define LOGIN_CAM_Z             (60.0)
#define LOGIN_LOOK_X            (1481.0)
#define LOGIN_LOOK_Y            (-1740.0)
#define LOGIN_LOOK_Z            (25.0)

// --- limits -----------------------------------------------------------------
#define MAX_PLAYER_VEHICLES     (5)
#define MAX_CREW_MEMBERS        (30)

// --- misc -------------------------------------------------------------------
#define GM_MAP_ICON_JOB         (56)
#define GM_ANTISPAM_MS          (1200)
