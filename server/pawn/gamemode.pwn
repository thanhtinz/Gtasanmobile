/**
 * GTA:SA Mobile — open.mp gamemode
 *
 * A GTA V / GTA Online flavoured freeroam-RP mode. It runs on open.mp, which
 * serves the 0.3.7 protocol, so the PC SA-MP client and the Android SA-MP
 * client both connect to the same server and play together.
 *
 * Build with tools/server-build.sh; see docs/SERVER.md.
 *
 * Include order matters: Pawn resolves constants and variables top-down (only
 * functions may be referenced ahead of their definition), so the core comes
 * first, then data, then the systems that operate on it, and the dialog router
 * last because it has to see every module.
 */

#include <open.mp>

// --- core -------------------------------------------------------------------
#include "core/config.inc"
#include "core/colors.inc"
#include "core/util.inc"
#include "core/params.inc"
#include "core/log.inc"
#include "core/cmd.inc"
#include "core/dialogs.inc"
#include "core/db.inc"
#include "core/schema.inc"

// --- player state -----------------------------------------------------------
#include "player/data.inc"
#include "player/money.inc"
#include "player/level.inc"

// --- world ------------------------------------------------------------------
#include "world/zones.inc"
#include "world/spawn.inc"

// --- systems ----------------------------------------------------------------
#include "jobs/jobs.inc"
#include "world/crime.inc"
#include "player/bank.inc"
#include "player/account.inc"
#include "player/events.inc"

// --- interface --------------------------------------------------------------
#include "core/menu.inc"
#include "core/commands.inc"
#include "core/dialog_router.inc"

// ---------------------------------------------------------------------------
// Entry points
// ---------------------------------------------------------------------------

main()
{
	// open.mp drives gamemodes through OnGameModeInit; main() stays empty.
}

public OnGameModeInit()
{
	SetGameModeText(GM_MODE_TEXT);
	ShowPlayerMarkers(PLAYER_MARKERS_MODE_GLOBAL);
	ShowNameTags(true);
	SetNameTagDrawDistance(40.0);
	EnableStuntBonusForAll(false);
	DisableInteriorEnterExits();
	SetWeather(GM_DEFAULT_WEATHER);
	SetWorldTime(12);
	UsePlayerPedAnims();

	Log("--------------------------------------------------");
	Log("%s v%s starting", GM_NAME, GM_VERSION);

	if (!DB_Init())
	{
		Log("FATAL: database initialisation failed");
		SendRconCommand("exit");
		return 0;
	}
	Log("database driver: %s", DB_DriverName());

	Schema_Create();
	Zones_Init();
	Spawn_Init();
	Player_StartTimers();

	Log("%d jobs loaded", Job_Count());
	Log("gamemode ready");
	Log("--------------------------------------------------");
	return 1;
}

public OnGameModeExit()
{
	foreach_player(i)
	{
		if (Player_IsLoggedIn(i))
		{
			Account_Save(i);
		}
	}

	DB_Exit();
	return 1;
}

public OnPlayerConnect(playerid)
{
	Player_Reset(playerid);
	Job_ResetPlayer(playerid);

	SendClientMessage(playerid, COLOR_INFO, "Chao mung den voi %s.", GM_NAME);
	Account_OnConnect(playerid);
	return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
	#pragma unused reason

	if (Player_IsLoggedIn(playerid))
	{
		Job_EndShift(playerid, false);
		Account_Save(playerid);
	}

	Player_Reset(playerid);
	Job_ResetPlayer(playerid);
	return 1;
}

public OnPlayerRequestClass(playerid, classid)
{
	#pragma unused classid

	// Skin selection is skipped entirely — players arrive through the account
	// flow, so the camera just parks on the city until they log in.
	Spawn_ShowLoginCamera(playerid);
	return 1;
}

public OnPlayerRequestSpawn(playerid)
{
	if (!Player_IsLoggedIn(playerid))
	{
		SendClientMessage(playerid, COLOR_ERROR, "Ban phai dang nhap truoc.");
		return 0;
	}
	return 1;
}

public OnPlayerSpawn(playerid)
{
	if (!Player_IsLoggedIn(playerid))
	{
		Spawn_ShowLoginCamera(playerid);
		return 0;
	}

	Spawn_Player(playerid);
	return 1;
}

public OnPlayerDeath(playerid, killerid, WEAPON:reason)
{
	Player_OnDeath(playerid, killerid, reason);
	return 1;
}

public OnPlayerEnterCheckpoint(playerid)
{
	Job_OnCheckpoint(playerid);
	return 1;
}

public OnPlayerText(playerid, text[])
{
	#pragma unused text

	if (!Player_IsLoggedIn(playerid))
	{
		SendClientMessage(playerid, COLOR_ERROR, "Ban phai dang nhap truoc khi chat.");
		return 0;
	}
	return 1;
}
