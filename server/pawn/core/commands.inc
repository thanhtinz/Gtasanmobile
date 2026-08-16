/**
 * Chat commands.
 *
 * These stay alongside the panel rather than being replaced by it: typing is
 * still faster on a PC client, and every command here is the same function the
 * menu calls.
 */

#if defined _gtasan_commands_included
	#endinput
#endif
#define _gtasan_commands_included

#define REQUIRE_LOGIN(%0) \
	if (!Player_IsLoggedIn(%0)) return 1

CMD:menu(playerid, params[])
{
	#pragma unused params
	REQUIRE_LOGIN(playerid);
	Menu_ShowRoot(playerid);
	return 1;
}
/// `/m` — the same panel, short enough to type on a phone keyboard.
CMD:m(playerid, params[])
{
	return cmd_menu(playerid, params);
}

CMD:help(playerid, params[])
{
	#pragma unused params
	REQUIRE_LOGIN(playerid);
	Menu_ShowHelp(playerid);
	return 1;
}

CMD:stats(playerid, params[])
{
	#pragma unused params
	REQUIRE_LOGIN(playerid);
	Menu_ShowCharacter(playerid);
	return 1;
}

CMD:jobs(playerid, params[])
{
	#pragma unused params
	REQUIRE_LOGIN(playerid);
	Menu_ShowJobRoot(playerid);
	return 1;
}

CMD:work(playerid, params[])
{
	#pragma unused params
	REQUIRE_LOGIN(playerid);
	Job_StartShift(playerid);
	return 1;
}

CMD:stopwork(playerid, params[])
{
	#pragma unused params
	REQUIRE_LOGIN(playerid);

	if (!Job_IsWorking(playerid))
	{
		SendClientMessage(playerid, COLOR_GREY, "Ban khong o trong ca lam nao.");
		return 1;
	}

	Job_EndShift(playerid, true);
	return 1;
}

CMD:quitjob(playerid, params[])
{
	#pragma unused params
	REQUIRE_LOGIN(playerid);

	if (Player[playerid][pJob] == JOB_NONE)
	{
		SendClientMessage(playerid, COLOR_GREY, "Ban dang khong lam nghe nao.");
		return 1;
	}

	ShowPlayerDialog(playerid, DIALOG_JOB_QUIT, DIALOG_STYLE_MSGBOX,
		"Nghi viec",
		"Ban co chac muon nghi nghe hien tai?\nBac nghe van duoc giu lai cho lan sau.",
		"Nghi viec", "Huy");
	return 1;
}

CMD:bank(playerid, params[])
{
	#pragma unused params
	REQUIRE_LOGIN(playerid);

	if (!Bank_IsPlayerAtBank(playerid))
	{
		SendClientMessage(playerid, COLOR_ERROR, "Ban phai dung tai mot chi nhanh ngan hang.");
		return 1;
	}

	Menu_ShowBank(playerid);
	return 1;
}

CMD:balance(playerid, params[])
{
	#pragma unused params
	REQUIRE_LOGIN(playerid);

	SendClientMessage(playerid, COLOR_MONEY, "Tien mat: %s | Ngan hang: %s | Tong: %s",
		Money(Player[playerid][pCash]), Money(Player[playerid][pBank]),
		Money(Player[playerid][pCash] + Player[playerid][pBank]));
	return 1;
}

/// Hand cash to a nearby player.
CMD:pay(playerid, params[])
{
	REQUIRE_LOGIN(playerid);

	new pos = 0;
	new targetid = Param_Player(params, pos);
	new amount;

	if (targetid == INVALID_PLAYER_ID || !Param_Int(params, pos, amount))
	{
		return Usage(playerid, "/pay <id/ten> <so tien>");
	}

	if (targetid == playerid)
	{
		SendClientMessage(playerid, COLOR_ERROR, "Ban khong the tra tien cho chinh minh.");
		return 1;
	}

	if (amount <= 0)
	{
		SendClientMessage(playerid, COLOR_ERROR, "So tien phai lon hon 0.");
		return 1;
	}

	new Float:tx, Float:ty, Float:tz;
	GetPlayerPos(targetid, tx, ty, tz);
	if (!IsPlayerInRangeOfPoint(playerid, 6.0, tx, ty, tz))
	{
		SendClientMessage(playerid, COLOR_ERROR, "Ban phai dung gan nguoi do.");
		return 1;
	}

	if (!Money_Take(playerid, amount, "pay out"))
	{
		SendClientMessage(playerid, COLOR_ERROR, "Ban khong du tien mat.");
		return 1;
	}

	Money_Give(targetid, amount, "pay in");

	SendClientMessage(playerid, COLOR_MONEY, "Ban da tra %s cho %s.", Money(amount), GetName(targetid));
	SendClientMessage(targetid, COLOR_MONEY, "%s da tra ban %s.", GetName(playerid), Money(amount));
	return 1;
}

CMD:where(playerid, params[])
{
	#pragma unused params
	REQUIRE_LOGIN(playerid);

	new zone[40];
	Zone_OfPlayer(playerid, zone, sizeof zone);
	SendClientMessage(playerid, COLOR_INFO, "Ban dang o: %s", zone);
	return 1;
}

CMD:players(playerid, params[])
{
	#pragma unused params
	REQUIRE_LOGIN(playerid);

	new count = 0;
	SendClientMessage(playerid, COLOR_INFO, "--- Nguoi choi dang online ---");

	foreach_player(i)
	{
		if (!Player_IsLoggedIn(i))
		{
			continue;
		}

		new jobName[26];
		Job_Name(Player[i][pJob], jobName, sizeof jobName);
		SendClientMessage(playerid, COLOR_GREY, "[%d] %s - level %d, %s",
			i, GetName(i), Player[i][pLevel], jobName);
		++count;
	}

	SendClientMessage(playerid, COLOR_INFO, "Tong: %d nguoi.", count);
	return 1;
}

CMD:pm(playerid, params[])
{
	REQUIRE_LOGIN(playerid);

	new pos = 0;
	new targetid = Param_Player(params, pos);
	new message[144];

	if (targetid == INVALID_PLAYER_ID || !Param_Rest(params, pos, message, sizeof message))
	{
		return Usage(playerid, "/pm <id/ten> <noi dung>");
	}

	if (targetid == playerid)
	{
		SendClientMessage(playerid, COLOR_ERROR, "Ban khong the nhan tin cho chinh minh.");
		return 1;
	}

	SendClientMessage(targetid, COLOR_INFO, "PM tu %s (%d): %s", GetName(playerid), playerid, message);
	SendClientMessage(playerid, COLOR_GREY, "PM den %s: %s", GetName(targetid), message);
	return 1;
}

CMD:me(playerid, params[])
{
	REQUIRE_LOGIN(playerid);

	new action[128], pos = 0;
	if (!Param_Rest(params, pos, action, sizeof action))
	{
		return Usage(playerid, "/me <hanh dong>");
	}

	new Float:x, Float:y, Float:z;
	GetPlayerPos(playerid, x, y, z);

	foreach_player(i)
	{
		if (IsPlayerInRangeOfPoint(i, 25.0, x, y, z))
		{
			SendClientMessage(i, COLOR_CREW, "* %s %s", GetName(playerid), action);
		}
	}
	return 1;
}

/// Police only: arrest a nearby wanted player.
CMD:arrest(playerid, params[])
{
	REQUIRE_LOGIN(playerid);

	if (!Crime_IsPolice(playerid))
	{
		SendClientMessage(playerid, COLOR_ERROR, "Chi canh sat moi dung duoc lenh nay.");
		return 1;
	}

	new pos = 0;
	new targetid = Param_Player(params, pos);

	if (targetid == INVALID_PLAYER_ID)
	{
		return Usage(playerid, "/arrest <id/ten>");
	}

	new Float:tx, Float:ty, Float:tz;
	GetPlayerPos(targetid, tx, ty, tz);
	if (!IsPlayerInRangeOfPoint(playerid, 5.0, tx, ty, tz))
	{
		SendClientMessage(playerid, COLOR_ERROR, "Ban phai dung sat nguoi do.");
		return 1;
	}

	Crime_Arrest(playerid, targetid);
	return 1;
}

CMD:wanted(playerid, params[])
{
	#pragma unused params
	REQUIRE_LOGIN(playerid);

	new count = 0;
	SendClientMessage(playerid, COLOR_POLICE, "--- Danh sach truy na ---");

	foreach_player(i)
	{
		if (!Player_IsLoggedIn(i) || Player[i][pWanted] <= 0)
		{
			continue;
		}

		new zone[40];
		Zone_OfPlayer(i, zone, sizeof zone);
		SendClientMessage(playerid, COLOR_GREY, "%s - %d sao - %s",
			GetName(i), Player[i][pWanted], zone);
		++count;
	}

	if (count == 0)
	{
		SendClientMessage(playerid, COLOR_GREY, "Hien khong co ai bi truy na.");
	}
	return 1;
}
