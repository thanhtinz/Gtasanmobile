/**
 * Levelling — the GTA Online "RP" progression.
 *
 * Experience is awarded for essentially every meaningful action, and converts
 * into levels on a curve that stretches as the player climbs, so early levels
 * arrive fast and later ones stay meaningful.
 */

#if defined _gtasan_level_included
	#endinput
#endif
#define _gtasan_level_included

#define LEVEL_MAX               (300)
#define LEVEL_BASE_EXP          (400)
#define LEVEL_CASH_REWARD       (2500)

/// Experience needed to advance *from* `level` to the next one.
stock Level_Required(level)
{
	if (level < 1)
	{
		level = 1;
	}
	// Mildly superlinear: level 1 costs 400, level 10 about 2.2k, level 50 21k.
	return LEVEL_BASE_EXP + (level * level * 8) + (level * 60);
}

/**
 * Awards RP. Returns the number of levels gained — usually 0 or 1, but a heist
 * payout landing on a low-level character can be worth more.
 */
stock Level_GiveExp(playerid, amount, const reason[] = "")
{
	if (!Player_IsLoggedIn(playerid) || amount <= 0)
	{
		return 0;
	}

	if (Player[playerid][pLevel] >= LEVEL_MAX)
	{
		return 0;
	}

	Player[playerid][pExp] += amount;

	if (reason[0] != EOS)
	{
		SendClientMessage(playerid, COLOR_LEVEL, "+%d RP (%s)", amount, reason);
	}

	new gained = 0;
	while (Player[playerid][pLevel] < LEVEL_MAX)
	{
		new required = Level_Required(Player[playerid][pLevel]);
		if (Player[playerid][pExp] < required)
		{
			break;
		}

		Player[playerid][pExp] -= required;
		Player[playerid][pLevel]++;
		++gained;
	}

	if (gained > 0)
	{
		Level_OnUp(playerid, gained);
	}
	return gained;
}

static Level_OnUp(playerid, gained)
{
	new reward = LEVEL_CASH_REWARD * Player[playerid][pLevel] / 2;
	Money_Give(playerid, reward, "level up");

	GameTextForPlayer(playerid, "~y~LEVEL ~w~%d", 4000, 1, Player[playerid][pLevel]);
	SendClientMessage(playerid, COLOR_LEVEL,
		"Chuc mung! Ban len level %d (+%d) va nhan %s.",
		Player[playerid][pLevel], gained, Money(reward));

	// Unlocks are announced rather than silently granted, so the number means
	// something concrete to the player.
	switch (Player[playerid][pLevel])
	{
		case 5:  SendClientMessage(playerid, COLOR_INFO, "Mo khoa: nhieu nghe moi trong /menu.");
		case 10: SendClientMessage(playerid, COLOR_INFO, "Mo khoa: nhom nghe khai thac.");
		case 15: SendClientMessage(playerid, COLOR_INFO, "Mo khoa: lap bang hoi.");
		case 20: SendClientMessage(playerid, COLOR_INFO, "Mo khoa: nhom nghe phe phai.");
		case 25: SendClientMessage(playerid, COLOR_INFO, "Mo khoa: nhom nghe phi phap.");
	}

	SendClientMessageToAll(COLOR_GREY, "%s vua dat level %d.",
		GetName(playerid), Player[playerid][pLevel]);
	return 1;
}
