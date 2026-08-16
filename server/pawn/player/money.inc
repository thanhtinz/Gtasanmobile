/**
 * Money.
 *
 * The client's money counter is a display only: the authoritative balance is
 * Player[][pCash] and the HUD is re-synced from it after every change. That
 * keeps a modified client from inventing cash, which matters more here than on
 * a PC-only server — injected mobile clients are common.
 */

#if defined _gtasan_money_included
	#endinput
#endif
#define _gtasan_money_included

stock Money_Get(playerid)
{
	return Player[playerid][pCash];
}

stock Money_GetBank(playerid)
{
	return Player[playerid][pBank];
}

/// Pushes the authoritative balance to the client HUD.
stock Money_Sync(playerid)
{
	if (!IsPlayerConnectedEx(playerid))
	{
		return 0;
	}
	ResetPlayerMoney(playerid);
	GivePlayerMoney(playerid, Player[playerid][pCash]);
	return 1;
}

stock Money_Set(playerid, amount, const reason[] = "set")
{
	if (amount < 0)
	{
		amount = 0;
	}
	else if (amount > GM_MAX_CASH)
	{
		amount = GM_MAX_CASH;
	}

	Player[playerid][pCash] = amount;
	Money_Sync(playerid);
	Money_Record(playerid, 0, reason);
	return 1;
}

/// Adds, or with a negative amount removes, cash. Clamped, never negative.
stock Money_Give(playerid, amount, const reason[] = "unknown")
{
	if (!Player_IsLoggedIn(playerid) || amount == 0)
	{
		return 0;
	}

	new balance = Player[playerid][pCash] + amount;
	if (balance < 0)
	{
		balance = 0;
	}
	else if (balance > GM_MAX_CASH)
	{
		balance = GM_MAX_CASH;
	}

	Player[playerid][pCash] = balance;
	Money_Sync(playerid);
	Money_Record(playerid, amount, reason);
	return 1;
}

/// Removes `amount` only if the player can afford it.
stock bool:Money_Take(playerid, amount, const reason[] = "unknown")
{
	if (amount <= 0 || Player[playerid][pCash] < amount)
	{
		return false;
	}
	Money_Give(playerid, -amount, reason);
	return true;
}

stock bool:Money_Has(playerid, amount)
{
	return Player[playerid][pCash] >= amount;
}

// ---------------------------------------------------------------------------
// Bank
// ---------------------------------------------------------------------------

stock bool:Bank_Deposit(playerid, amount)
{
	if (amount <= 0 || !Money_Has(playerid, amount))
	{
		return false;
	}

	Money_Give(playerid, -amount, "bank deposit");
	Player[playerid][pBank] += amount;
	return true;
}

stock bool:Bank_Withdraw(playerid, amount)
{
	if (amount <= 0 || Player[playerid][pBank] < amount)
	{
		return false;
	}

	Player[playerid][pBank] -= amount;
	Money_Give(playerid, amount, "bank withdraw");
	return true;
}

stock bool:Bank_Transfer(playerid, targetid, amount)
{
	if (amount <= 0 || playerid == targetid)
	{
		return false;
	}
	if (!Player_IsLoggedIn(targetid) || Player[playerid][pBank] < amount)
	{
		return false;
	}

	Player[playerid][pBank] -= amount;
	Player[targetid][pBank] += amount;

	Money_Record(playerid, -amount, "transfer out");
	Money_Record(targetid, amount, "transfer in");
	return true;
}

// ---------------------------------------------------------------------------
// Audit trail
// ---------------------------------------------------------------------------

/// Appends to money_log. Large movements also hit the economy log file so a
/// duplication bug can be traced without opening a database client.
stock Money_Record(playerid, amount, const reason[])
{
	if (Player[playerid][pAccountID] <= 0)
	{
		return 0;
	}

	new
		escaped[64],
		query[256];

	DB_Escape(reason, escaped, sizeof escaped);
	format(query, sizeof query,
		"INSERT INTO money_log (account_id, amount, balance, reason, stamp) "\
		"VALUES (%d, %d, %d, '%s', %d);",
		Player[playerid][pAccountID], amount,
		Player[playerid][pCash] + Player[playerid][pBank], escaped, gettime());
	DB_Query(query);

	if (amount >= 100000 || amount <= -100000)
	{
		new line[160];
		format(line, sizeof line, "%s (acc %d) %s%d - %s",
			GetName(playerid), Player[playerid][pAccountID],
			amount > 0 ? ("+") : (""), amount, reason);
		LogEconomy(line);
	}
	return 1;
}
