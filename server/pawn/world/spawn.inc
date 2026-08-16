/**
 * Spawning and the login camera.
 */

#if defined _gtasan_spawn_included
	#endinput
#endif
#define _gtasan_spawn_included

// Hospitals players respawn at after dying; the nearest one wins.
static const Float:g_Hospitals[][3] =
{
	{1172.0, -1323.2,  15.4},   // Los Santos county general
	{2034.0, -1402.7,  17.2},   // Jefferson
	{-2655.6,  635.2,  14.4},   // San Fierro
	{1607.0,  1815.7,  10.8}    // Las Venturas
};

stock Spawn_Init()
{
	// The class is a formality: OnPlayerRequestClass parks the camera instead
	// of showing the usual skin selection.
	AddPlayerClass(26, SPAWN_DEFAULT_X, SPAWN_DEFAULT_Y, SPAWN_DEFAULT_Z,
		SPAWN_DEFAULT_A, WEAPON:0, 0, WEAPON:0, 0, WEAPON:0, 0);
	return 1;
}

stock Spawn_ShowLoginCamera(playerid)
{
	TogglePlayerSpectating(playerid, true);
	InterpolateCameraPos(playerid,
		LOGIN_CAM_X, LOGIN_CAM_Y, LOGIN_CAM_Z,
		LOGIN_CAM_X - 90.0, LOGIN_CAM_Y - 40.0, LOGIN_CAM_Z - 10.0, 30000);
	InterpolateCameraLookAt(playerid,
		LOGIN_LOOK_X, LOGIN_LOOK_Y, LOGIN_LOOK_Z,
		LOGIN_LOOK_X, LOGIN_LOOK_Y, LOGIN_LOOK_Z, 30000);
	return 1;
}

/// Restores everything the client forgets across a spawn.
stock Spawn_Player(playerid)
{
	SetPlayerSkin(playerid, Player[playerid][pSkin]);
	SetPlayerPos(playerid, Player[playerid][pPosX], Player[playerid][pPosY], Player[playerid][pPosZ]);
	SetPlayerFacingAngle(playerid, Player[playerid][pPosA]);
	SetPlayerInterior(playerid, Player[playerid][pInterior]);
	SetPlayerVirtualWorld(playerid, Player[playerid][pWorld]);
	SetPlayerHealth(playerid, Player[playerid][pHealth]);
	SetPlayerArmour(playerid, Player[playerid][pArmour]);
	SetPlayerWantedLevel(playerid, Player[playerid][pWanted]);
	SetCameraBehindPlayer(playerid);

	Money_Sync(playerid);
	Player[playerid][pSpawned] = true;
	return 1;
}

/// Sends a dead player to the closest hospital.
stock Spawn_AtHospital(playerid)
{
	new
		Float:x, Float:y, Float:z,
		best = 0,
		Float:bestDistance = 999999.0;

	GetPlayerPos(playerid, x, y, z);

	for (new i = 0; i < sizeof (g_Hospitals); ++i)
	{
		new Float:distance = VectorSize(
			g_Hospitals[i][0] - x, g_Hospitals[i][1] - y, g_Hospitals[i][2] - z);
		if (distance < bestDistance)
		{
			bestDistance = distance;
			best = i;
		}
	}

	Player[playerid][pPosX]     = g_Hospitals[best][0];
	Player[playerid][pPosY]     = g_Hospitals[best][1];
	Player[playerid][pPosZ]     = g_Hospitals[best][2];
	Player[playerid][pPosA]     = 0.0;
	Player[playerid][pInterior] = 0;
	Player[playerid][pWorld]    = 0;
	Player[playerid][pHealth]   = 100.0;
	Player[playerid][pArmour]   = 0.0;
	return 1;
}
