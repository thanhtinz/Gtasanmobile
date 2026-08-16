/**
 * Per-player state.
 *
 * One array, one enum. Everything that must survive a disconnect is mirrored
 * into the accounts / characters tables by player/account.inc; anything that
 * only matters for the current session lives here alone.
 */

#if defined _gtasan_player_data_included
	#endinput
#endif
#define _gtasan_player_data_included

enum E_PLAYER
{
	// identity
	bool:pLoggedIn,
	pAccountID,
	pCharID,
	pAdmin,
	pVIP,
	bool:pBanned,

	// economy
	pCash,
	pBank,

	// progression
	pLevel,
	pExp,

	// appearance / position
	pSkin,
	Float:pHealth,
	Float:pArmour,
	Float:pPosX,
	Float:pPosY,
	Float:pPosZ,
	Float:pPosA,
	pInterior,
	pWorld,

	// work — per-job rank lives in the job_progress table, not here
	pJob,

	// crime and combat
	pWanted,
	pKills,
	pDeaths,
	pArrests,
	pJailTime,

	// social
	pCrew,
	pPhone,

	// session only
	pPlaytime,          // total seconds accumulated across sessions
	pSessionStart,
	pLoginAttempts,
	bool:pSpawned,
	pMenuPage
}

new Player[MAX_PLAYERS][E_PLAYER];

stock bool:Player_IsLoggedIn(playerid)
{
	if (!IsPlayerConnectedEx(playerid))
	{
		return false;
	}
	return Player[playerid][pLoggedIn];
}

stock bool:Player_IsAdmin(playerid, level = 1)
{
	return Player_IsLoggedIn(playerid) && Player[playerid][pAdmin] >= level;
}

/// Wipes a slot back to defaults. Called on connect and on disconnect.
stock Player_Reset(playerid)
{
	if (!(0 <= playerid < MAX_PLAYERS))
	{
		return 0;
	}

	// Clearing the whole row is cheaper and safer than listing every field;
	// only the non-zero defaults are restored afterwards.
	new empty[E_PLAYER];
	Player[playerid] = empty;

	Player[playerid][pAccountID] = -1;
	Player[playerid][pCharID]    = -1;
	Player[playerid][pJob]       = -1;
	Player[playerid][pCrew]      = -1;
	Player[playerid][pLevel]     = 1;
	Player[playerid][pSkin]      = 26;
	Player[playerid][pHealth]    = 100.0;

	Cmd_ResetPlayer(playerid);
	return 1;
}

/// Seconds spent in game, including the current session.
stock Player_Playtime(playerid)
{
	new total = Player[playerid][pPlaytime];
	if (Player[playerid][pSessionStart] > 0)
	{
		total += gettime() - Player[playerid][pSessionStart];
	}
	return total;
}

/// Copies live client state into the struct so a save writes current values.
stock Player_CaptureState(playerid)
{
	if (!Player_IsLoggedIn(playerid) || !Player[playerid][pSpawned])
	{
		return 0;
	}

	new Float:x, Float:y, Float:z, Float:angle;
	GetPlayerPos(playerid, x, y, z);
	GetPlayerFacingAngle(playerid, angle);

	Player[playerid][pPosX]     = x;
	Player[playerid][pPosY]     = y;
	Player[playerid][pPosZ]     = z;
	Player[playerid][pPosA]     = angle;
	Player[playerid][pInterior] = GetPlayerInterior(playerid);
	Player[playerid][pWorld]    = GetPlayerVirtualWorld(playerid);

	new Float:health, Float:armour;
	GetPlayerHealth(playerid, health);
	GetPlayerArmour(playerid, armour);
	Player[playerid][pHealth] = health;
	Player[playerid][pArmour] = armour;

	Player[playerid][pSkin] = GetPlayerSkin(playerid);
	return 1;
}
