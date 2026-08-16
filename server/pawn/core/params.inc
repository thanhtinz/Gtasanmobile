/**
 * Command parameter parsing.
 *
 * Deliberately dependency-free. The usual choice here is the sscanf plugin, but
 * every plugin is one more binary a server owner has to match to their platform,
 * and this gamemode is meant to be easy to host. These helpers cover what it
 * actually needs.
 *
 *     new pos = 0, amount;
 *     if (!Param_Int(params, pos, amount)) return Usage(playerid, "/pay <id> <so tien>");
 */

#if defined _gtasan_params_included
	#endinput
#endif
#define _gtasan_params_included

/// True when nothing is left to read.
stock bool:Param_Empty(const params[], pos = 0)
{
	while (params[pos] == ' ')
	{
		++pos;
	}
	return params[pos] == EOS;
}

/// Reads the next whitespace-delimited token, advancing `pos`.
stock bool:Param_Next(const params[], &pos, dest[], size = sizeof (dest))
{
	dest[0] = EOS;

	while (params[pos] == ' ')
	{
		++pos;
	}

	if (params[pos] == EOS)
	{
		return false;
	}

	new written = 0;
	while (params[pos] != EOS && params[pos] != ' ')
	{
		if (written < size - 1)
		{
			dest[written++] = params[pos];
		}
		++pos;
	}

	dest[written] = EOS;
	return written > 0;
}

stock bool:Param_Int(const params[], &pos, &value)
{
	new token[24];
	if (!Param_Next(params, pos, token, sizeof token) || !IsNumericString(token))
	{
		return false;
	}
	value = strval(token);
	return true;
}

stock bool:Param_Float(const params[], &pos, &Float:value)
{
	new token[32];
	if (!Param_Next(params, pos, token, sizeof token))
	{
		return false;
	}

	// floatstr returns 0.0 for junk, so the token is validated by hand.
	new i = (token[0] == '-' || token[0] == '+') ? 1 : 0, digits = 0, dots = 0;
	if (token[i] == EOS)
	{
		return false;
	}
	for ( ; token[i] != EOS; ++i)
	{
		if (token[i] == '.')
		{
			if (++dots > 1)
			{
				return false;
			}
		}
		else if ('0' <= token[i] <= '9')
		{
			++digits;
		}
		else
		{
			return false;
		}
	}
	if (digits == 0)
	{
		return false;
	}

	value = floatstr(token);
	return true;
}

/// Everything from `pos` to the end, trimmed.
stock bool:Param_Rest(const params[], &pos, dest[], size = sizeof (dest))
{
	dest[0] = EOS;

	while (params[pos] == ' ')
	{
		++pos;
	}

	if (params[pos] == EOS)
	{
		return false;
	}

	new written = 0;
	while (params[pos] != EOS && written < size - 1)
	{
		dest[written++] = params[pos++];
	}
	dest[written] = EOS;

	while (written > 0 && dest[written - 1] == ' ')
	{
		dest[--written] = EOS;
	}
	return written > 0;
}

/**
 * Resolves a player reference: a numeric id, or a case-insensitive fragment of
 * a nickname. An ambiguous fragment resolves to nobody, which is the safe
 * outcome for commands that move money or hand out punishments.
 */
stock Param_Player(const params[], &pos)
{
	new token[MAX_PLAYER_NAME + 1];
	if (!Param_Next(params, pos, token, sizeof token))
	{
		return INVALID_PLAYER_ID;
	}

	if (IsNumericString(token))
	{
		new id = strval(token);
		return IsPlayerConnectedEx(id) ? id : INVALID_PLAYER_ID;
	}

	new found = INVALID_PLAYER_ID;
	foreach_player(i)
	{
		if (ContainsIgnoreCase(GetName(i), token))
		{
			if (found != INVALID_PLAYER_ID)
			{
				return INVALID_PLAYER_ID; // ambiguous
			}
			found = i;
		}
	}
	return found;
}

stock Usage(playerid, const text[])
{
	SendClientMessage(playerid, COLOR_GREY, "[Cach dung] %s", text);
	return 1;
}
