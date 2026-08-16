/**
 * Wanted level and jail.
 *
 * The client-side wanted stars are only a display; the server keeps the real
 * number so it survives a reconnect, and so a modified client cannot clear it.
 */

#if defined _gtasan_crime_included
	#endinput
#endif
#define _gtasan_crime_included

#define WANTED_MAX              (6)
#define JAIL_SECONDS_PER_STAR   (30)

// Where an arrested player is held.
#define JAIL_X                  (264.0)
#define JAIL_Y                  (77.0)
#define JAIL_Z                  (1001.0)
#define JAIL_INTERIOR           (6)

stock Wanted_Get(playerid)
{
	return Player[playerid][pWanted];
}

stock Wanted_Set(playerid, stars)
{
	if (stars < 0)
	{
		stars = 0;
	}
	else if (stars > WANTED_MAX)
	{
		stars = WANTED_MAX;
	}

	Player[playerid][pWanted] = stars;
	SetPlayerWantedLevel(playerid, stars);
	return 1;
}

stock Wanted_Add(playerid, stars)
{
	if (stars <= 0)
	{
		return 0;
	}

	new before = Player[playerid][pWanted];
	Wanted_Set(playerid, before + stars);

	if (Player[playerid][pWanted] > before)
	{
		SendClientMessage(playerid, COLOR_CRIME, "Do truy na cua ban tang len %d sao.",
			Player[playerid][pWanted]);
		Crime_AlertPolice(playerid);
	}
	return 1;
}

stock Wanted_Clear(playerid)
{
	Wanted_Set(playerid, 0);
	return 1;
}

/// Job name as a value, so callers can compare without their own buffer.
stock Job_NameOf(jobid)
{
	new name[26];
	Job_Name(jobid, name, sizeof name);
	return name;
}

stock bool:Crime_IsPolice(playerid)
{
	new jobid = Player[playerid][pJob];
	return Job_IsValid(jobid) && Job_Category(jobid) == JOBCAT_FACTION
		&& strcmp(Job_NameOf(jobid), "Canh sat", true) == 0;
}

/// Tells every on-duty officer where a wanted player was last seen.
stock Crime_AlertPolice(playerid)
{
	new zone[40];
	Zone_OfPlayer(playerid, zone, sizeof zone);

	foreach_player(i)
	{
		if (!Player_IsLoggedIn(i) || !Crime_IsPolice(i))
		{
			continue;
		}
		SendClientMessage(i, COLOR_POLICE, "[BO DAM] %s - %d sao - khu vuc %s.",
			GetName(playerid), Player[playerid][pWanted], zone);
	}
	return 1;
}

/**
 * Arrest. Pays the arresting officer, jails the suspect for a time based on how
 * wanted they were, and takes a cut of the cash they were carrying.
 */
stock Crime_Arrest(officerid, suspectid)
{
	new stars = Player[suspectid][pWanted];
	if (stars <= 0)
	{
		SendClientMessage(officerid, COLOR_ERROR, "Nguoi nay khong bi truy na.");
		return 0;
	}

	new
		reward = 1500 * stars,
		fine = Player[suspectid][pCash] / 4,
		seconds = stars * JAIL_SECONDS_PER_STAR;

	Money_Give(officerid, reward, "arrest reward");
	Level_GiveExp(officerid, 20 * stars, "bat giu toi pham");
	Player[officerid][pArrests]++;

	if (fine > 0)
	{
		Money_Give(suspectid, -fine, "arrest fine");
	}

	Wanted_Clear(suspectid);
	Crime_Jail(suspectid, seconds);

	SendClientMessage(officerid, COLOR_POLICE, "Ban da bat giu %s (+%s).",
		GetName(suspectid), Money(reward));
	SendClientMessage(suspectid, COLOR_POLICE,
		"Ban bi %s bat giu. Ngoi tu %d giay, phat %s.",
		GetName(officerid), seconds, Money(fine));

	new line[160];
	format(line, sizeof line, "%s arrested %s (%d stars, fine %d)",
		GetName(officerid), GetName(suspectid), stars, fine);
	LogAdmin(line);
	return 1;
}

stock Crime_Jail(playerid, seconds)
{
	Player[playerid][pJailTime] = seconds;

	SetPlayerInterior(playerid, JAIL_INTERIOR);
	SetPlayerPos(playerid, JAIL_X, JAIL_Y, JAIL_Z);
	ResetPlayerWeapons(playerid);

	Player[playerid][pInterior] = JAIL_INTERIOR;
	Player[playerid][pPosX] = JAIL_X;
	Player[playerid][pPosY] = JAIL_Y;
	Player[playerid][pPosZ] = JAIL_Z;
	return 1;
}

stock bool:Crime_IsJailed(playerid)
{
	return Player[playerid][pJailTime] > 0;
}

/// Ticked once a second; releases the player when their time runs out.
stock Crime_Tick(playerid)
{
	if (Player[playerid][pJailTime] <= 0)
	{
		return 0;
	}

	Player[playerid][pJailTime]--;

	if (Player[playerid][pJailTime] <= 0)
	{
		SendClientMessage(playerid, COLOR_SUCCESS, "Ban da man han. Duoc tha ra.");
		Spawn_AtHospital(playerid);
		SetPlayerInterior(playerid, 0);
		SetPlayerPos(playerid, Player[playerid][pPosX], Player[playerid][pPosY], Player[playerid][pPosZ]);
	}
	else if (Player[playerid][pJailTime] % 15 == 0)
	{
		SendClientMessage(playerid, COLOR_GREY, "Con %d giay trong tu.", Player[playerid][pJailTime]);
	}
	return 1;
}
