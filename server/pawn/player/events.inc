/**
 * Player lifecycle events and the periodic timers.
 */

#if defined _gtasan_player_events_included
	#endinput
#endif
#define _gtasan_player_events_included

forward Timer_Save();
forward Timer_Payday();
forward Timer_Second();

stock Player_OnDeath(playerid, killerid, WEAPON:reason)
{
	#pragma unused reason

	Player[playerid][pSpawned] = false;
	Player[playerid][pDeaths]++;

	Job_EndShift(playerid, false);

	// Dying costs a slice of what is in your pockets. Banked money is safe,
	// which is the whole reason the bank is worth walking to.
	new carried = Player[playerid][pCash];
	if (carried > 0)
	{
		new lost = floatround(float(carried) * GM_DEATH_CASH_LOSS);
		if (lost > 0)
		{
			Money_Give(playerid, -lost, "death");
			SendClientMessage(playerid, COLOR_ERROR, "Ban mat %s tien mat khi chet.", Money(lost));
		}
	}

	// Dying does not wipe a wanted level, it only halves it — otherwise
	// suicide would be the cheapest way to shake the police.
	if (Player[playerid][pWanted] > 0)
	{
		Wanted_Set(playerid, Player[playerid][pWanted] / 2);
	}

	if (IsPlayerConnectedEx(killerid) && Player_IsLoggedIn(killerid) && killerid != playerid)
	{
		Player[killerid][pKills]++;
		Level_GiveExp(killerid, 25, "ha guc doi thu");
		Money_Give(killerid, 500, "kill reward");
		SendClientMessage(killerid, COLOR_SUCCESS, "Ban da ha guc %s.", GetName(playerid));

		// Killing someone who was not wanted makes *you* wanted.
		if (Player[playerid][pWanted] == 0 && !Crime_IsPolice(killerid))
		{
			Wanted_Add(killerid, 2);
		}
	}

	Spawn_AtHospital(playerid);
	return 1;
}

stock Player_StartTimers()
{
	SetTimer("Timer_Save", GM_SAVE_INTERVAL_MS, true);
	SetTimer("Timer_Payday", GM_PAYDAY_INTERVAL_MS, true);
	SetTimer("Timer_Second", 1000, true);
	return 1;
}

public Timer_Save()
{
	new saved = 0;
	foreach_player(i)
	{
		if (Player_IsLoggedIn(i))
		{
			Account_Save(i);
			++saved;
		}
	}

	if (saved > 0)
	{
		Log("autosave: %d player(s)", saved);
	}
	return 1;
}

public Timer_Second()
{
	foreach_player(i)
	{
		if (Player_IsLoggedIn(i))
		{
			Crime_Tick(i);
		}
	}
	return 1;
}

/**
 * Payday: interest on savings plus a flat wage. This is the GTA Online drip
 * that keeps a player who logs in without doing anything from being stuck at
 * zero, without making doing nothing competitive with working.
 */
public Timer_Payday()
{
	foreach_player(i)
	{
		if (!Player_IsLoggedIn(i))
		{
			continue;
		}

		new
			interest = Player[i][pBank] / 200,          // 0.5% per payday
			wage = 500 + (Player[i][pLevel] * 25);

		if (interest > 25000)
		{
			interest = 25000;                          // caps runaway compounding
		}

		Player[i][pBank] += interest;
		Money_Give(i, wage, "payday");
		Level_GiveExp(i, 10);

		SendClientMessage(i, COLOR_MONEY, "--- PAYDAY ---");
		SendClientMessage(i, COLOR_MONEY, "Luong: %s | Lai ngan hang: %s",
			Money(wage), Money(interest));
	}
	return 1;
}
