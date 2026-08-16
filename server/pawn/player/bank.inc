/**
 * Bank branches.
 *
 * Banking only works while standing at a branch, so the bank is a place you go
 * rather than a menu you carry — that is what makes carrying cash a risk worth
 * managing.
 */

#if defined _gtasan_bank_included
	#endinput
#endif
#define _gtasan_bank_included

static const Float:g_Banks[][3] =
{
	{1462.0, -1012.0, 26.8},   // Los Santos, Commerce
	{2131.0,  1400.0, 10.8},   // Las Venturas
	{-1980.0,  300.0, 35.4},   // San Fierro
	{1481.0, -1755.0, 13.5}    // Downtown LS kiosk
};

stock bool:Bank_IsPlayerAtBank(playerid)
{
	for (new i = 0; i < sizeof (g_Banks); ++i)
	{
		if (IsPlayerInRangeOfPoint(playerid, 5.0, g_Banks[i][0], g_Banks[i][1], g_Banks[i][2]))
		{
			return true;
		}
	}
	return false;
}

/// Marks the nearest branch on the player's map.
stock Bank_MarkNearest(playerid)
{
	new
		best = 0,
		Float:bestDistance = 999999.0;

	for (new i = 0; i < sizeof (g_Banks); ++i)
	{
		new Float:distance = DistanceToPoint(playerid, g_Banks[i][0], g_Banks[i][1], g_Banks[i][2]);
		if (distance >= 0.0 && distance < bestDistance)
		{
			bestDistance = distance;
			best = i;
		}
	}

	SetPlayerMapIcon(playerid, 2, g_Banks[best][0], g_Banks[best][1], g_Banks[best][2],
		52, 0, MAPICON_GLOBAL);
	SendClientMessage(playerid, COLOR_INFO, "Da danh dau ngan hang gan nhat (%d m).",
		floatround(bestDistance));
	return 1;
}

/// Handles the amount typed into the deposit / withdraw input dialog.
stock Bank_OnAmount(playerid, dialogid, const inputtext[])
{
	if (!IsNumericString(inputtext))
	{
		SendClientMessage(playerid, COLOR_ERROR, "So tien khong hop le.");
		Menu_ShowBank(playerid);
		return 0;
	}

	new amount = strval(inputtext);
	if (amount <= 0)
	{
		SendClientMessage(playerid, COLOR_ERROR, "So tien phai lon hon 0.");
		Menu_ShowBank(playerid);
		return 0;
	}

	if (dialogid == DIALOG_BANK_DEPOSIT)
	{
		if (!Bank_Deposit(playerid, amount))
		{
			SendClientMessage(playerid, COLOR_ERROR, "Ban khong du tien mat.");
			return 0;
		}
		SendClientMessage(playerid, COLOR_SUCCESS, "Da gui %s. So du: %s.",
			Money(amount), Money(Player[playerid][pBank]));
	}
	else
	{
		if (!Bank_Withdraw(playerid, amount))
		{
			SendClientMessage(playerid, COLOR_ERROR, "So du khong du.");
			return 0;
		}
		SendClientMessage(playerid, COLOR_SUCCESS, "Da rut %s. So du: %s.",
			Money(amount), Money(Player[playerid][pBank]));
	}

	Menu_ShowBank(playerid);
	return 1;
}
