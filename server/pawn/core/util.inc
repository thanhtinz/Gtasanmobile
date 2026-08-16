/**
 * Small helpers shared by every module.
 */

#if defined _gtasan_util_included
	#endinput
#endif
#define _gtasan_util_included

#define IsPlayerConnectedEx(%0) (0 <= (%0) < MAX_PLAYERS && IsPlayerConnected(%0))

// GetPlayerPoolSize is deprecated in open.mp and documented as broken, so this
// walks the full slot range instead. Cheap enough at MAX_PLAYERS scale.
#define foreach_player(%1) \
	for (new %1 = 0; %1 < MAX_PLAYERS; ++%1) \
		if (IsPlayerConnected(%1))

stock GetName(playerid)
{
	new name[MAX_PLAYER_NAME + 1];
	if (!GetPlayerName(playerid, name, sizeof name))
	{
		name = "Unknown";
	}
	return name;
}

/// Formats an amount as `$1,234,567`, handling negatives.
stock FormatMoney(dest[], size, amount)
{
	new
		buffer[16],
		out[24],
		bool:negative = (amount < 0),
		pos = 0;

	// cellmin has no positive counterpart, so it is clamped rather than negated.
	if (amount == cellmin)
	{
		amount = cellmax;
	}
	else if (negative)
	{
		amount = -amount;
	}

	format(buffer, sizeof buffer, "%d", amount);

	new len = strlen(buffer);
	for (new i = 0; i < len; ++i)
	{
		if (i != 0 && (len - i) % 3 == 0)
		{
			out[pos++] = ',';
		}
		out[pos++] = buffer[i];
	}
	out[pos] = EOS;

	format(dest, size, "%s$%s", negative ? ("-") : (""), out);
	return 1;
}

/// Same thing, returned directly so it can be dropped into a format() call.
stock Money(amount)
{
	new out[24];
	FormatMoney(out, sizeof out, amount);
	return out;
}

stock Float:GetPlayerSpeed(playerid)
{
	new Float:x, Float:y, Float:z;
	if (GetPlayerVehicleID(playerid) != INVALID_VEHICLE_ID)
	{
		GetVehicleVelocity(GetPlayerVehicleID(playerid), x, y, z);
	}
	else
	{
		GetPlayerVelocity(playerid, x, y, z);
	}
	// 180 converts SA units-per-tick into something close to km/h.
	return floatsqroot(x * x + y * y + z * z) * 180.0;
}

stock ToLowerString(string[])
{
	for (new i = 0, len = strlen(string); i < len; ++i)
	{
		if ('A' <= string[i] <= 'Z')
		{
			string[i] += 32;
		}
	}
	return 1;
}

stock bool:IsNumericString(const string[])
{
	if (string[0] == EOS)
	{
		return false;
	}

	new i = (string[0] == '-' || string[0] == '+') ? 1 : 0;
	if (string[i] == EOS)
	{
		return false;
	}

	for ( ; string[i] != EOS; ++i)
	{
		if (!('0' <= string[i] <= '9'))
		{
			return false;
		}
	}
	return true;
}

stock bool:ContainsIgnoreCase(const haystack[], const needle[])
{
	return strfind(haystack, needle, true) != -1;
}

/// Seconds rendered as `1h 05m 12s`.
stock FormatDuration(dest[], size, seconds)
{
	if (seconds < 0)
	{
		seconds = 0;
	}

	new
		h = seconds / 3600,
		m = (seconds % 3600) / 60,
		s = seconds % 60;

	if (h > 0)
	{
		format(dest, size, "%dh %02dm %02ds", h, m, s);
	}
	else if (m > 0)
	{
		format(dest, size, "%dm %02ds", m, s);
	}
	else
	{
		format(dest, size, "%ds", s);
	}
	return 1;
}

stock RandomEx(min, max)
{
	if (max <= min)
	{
		return min;
	}
	return min + random(max - min);
}

stock AnnounceToAll(const message[])
{
	SendClientMessageToAll(COLOR_ANNOUNCE, "[SERVER] %s", message);
	return 1;
}

/// Distance from a player to a point, or -1.0 when they are not connected.
stock Float:DistanceToPoint(playerid, Float:x, Float:y, Float:z)
{
	if (!IsPlayerConnected(playerid))
	{
		return -1.0;
	}

	new Float:px, Float:py, Float:pz;
	GetPlayerPos(playerid, px, py, pz);
	return VectorSize(x - px, y - py, z - pz);
}
