/**
 * Command dispatcher.
 *
 * The well-known ZCMD idiom — every command is a public named `cmd_<name>` and
 * dispatch is one funcidx lookup — reimplemented here so the gamemode carries
 * no include-pack dependency.
 *
 *     CMD:heal(playerid, params[])
 *     {
 *         SetPlayerHealth(playerid, 100.0);
 *         return 1;
 *     }
 *
 * Returning 0 reports "unknown command" to the player, matching the SA-MP
 * convention; returning 1 means handled.
 */

#if defined _gtasan_cmd_included
	#endinput
#endif
#define _gtasan_cmd_included

#define CMD:%0(%1) \
	forward cmd_%0(%1); \
	public cmd_%0(%1)

static g_LastCommandTick[MAX_PLAYERS];

public OnPlayerCommandText(playerid, cmdtext[])
{
	if (cmdtext[1] == EOS)
	{
		return 1;
	}

	new tick = GetTickCount();
	if (tick - g_LastCommandTick[playerid] < GM_ANTISPAM_MS / 6)
	{
		SendClientMessage(playerid, COLOR_GREY, "Ban dang go lenh qua nhanh.");
		return 1;
	}
	g_LastCommandTick[playerid] = tick;

	new
		function[40] = "cmd_",
		pos = 1,
		written = 4;

	while (cmdtext[pos] != EOS && cmdtext[pos] != ' ')
	{
		if (written < sizeof (function) - 1)
		{
			new c = cmdtext[pos];
			function[written++] = ('A' <= c <= 'Z') ? (c + 32) : c;
		}
		++pos;
	}
	function[written] = EOS;

	// Skip the single separating space so `params` starts at the first argument.
	if (cmdtext[pos] == ' ')
	{
		++pos;
	}

	if (funcidx(function) == -1)
	{
		SendClientMessage(playerid, COLOR_ERROR, "Lenh khong ton tai. Go /menu de mo bang dieu khien.");
		return 1;
	}

	if (!CallLocalFunction(function, "is", playerid, cmdtext[pos]))
	{
		SendClientMessage(playerid, COLOR_ERROR, "Lenh khong ton tai. Go /menu de mo bang dieu khien.");
	}
	return 1;
}

/// Called from Player_Reset so a reconnecting slot starts with a clean cooldown.
stock Cmd_ResetPlayer(playerid)
{
	g_LastCommandTick[playerid] = 0;
	return 1;
}
