/**
 * Registration, login and persistence.
 *
 * Passwords are stored as SHA-256 over a per-account random salt — never in
 * plaintext, and never with a shared salt, so one leaked row does not help
 * against any other.
 *
 * open.mp deprecates SHA256_PassHash and points at bcrypt, and it is right to:
 * SHA-256 is fast, which is exactly the wrong property for a password hash, and
 * a leaked table is crackable on a GPU. bcrypt is not in the standard library
 * though — it is a separate plugin binary the server owner would have to match
 * to their platform, and keeping this gamemode plugin-free is what makes it
 * deployable anywhere. If you expect a player base worth attacking, add the
 * bcrypt plugin and swap the two SHA256_PassHash calls below; nothing else in
 * the gamemode touches the hash. See docs/SERVER.md.
 */

#if defined _gtasan_account_included
	#endinput
#endif
#define _gtasan_account_included

// Login-flow scratch values, kept out of the main player enum because nothing
// else has any business reading them.
static
	g_Salt[MAX_PLAYERS][GM_SALT_LEN],
	g_Hash[MAX_PLAYERS][GM_HASH_LEN];

forward OnAccountChecked(playerid, extra);
forward OnAccountCreated(playerid, extra);
forward OnCharacterLoaded(playerid, extra);

stock Account_OnConnect(playerid)
{
	new
		name[MAX_PLAYER_NAME + 1],
		escaped[MAX_PLAYER_NAME * 2 + 1],
		query[256];

	GetPlayerName(playerid, name, sizeof name);
	DB_Escape(name, escaped, sizeof escaped);

	format(query, sizeof query,
		"SELECT id, password, salt, admin, vip, banned, playtime "\
		"FROM accounts WHERE name = '%s' LIMIT 1;", escaped);

	DB_QueryCB(query, "OnAccountChecked", playerid);
	return 1;
}

public OnAccountChecked(playerid, extra)
{
	#pragma unused extra

	if (!IsPlayerConnectedEx(playerid))
	{
		return 0;
	}

	if (!DB_Next())
	{
		Account_ShowRegister(playerid, false);
		return 1;
	}

	Player[playerid][pAccountID] = DB_Int("id");
	Player[playerid][pAdmin]     = DB_Int("admin");
	Player[playerid][pVIP]       = DB_Int("vip");
	Player[playerid][pBanned]    = DB_Bool("banned");
	Player[playerid][pPlaytime]  = DB_Int("playtime");

	DB_Str("password", g_Hash[playerid], GM_HASH_LEN);
	DB_Str("salt", g_Salt[playerid], GM_SALT_LEN);

	if (Player[playerid][pBanned])
	{
		SendClientMessage(playerid, COLOR_ERROR, "Tai khoan cua ban da bi khoa.");
		Kick(playerid);
		return 1;
	}

	Account_ShowLogin(playerid, false);
	return 1;
}

stock Account_ShowRegister(playerid, bool:retry)
{
	new body[512];
	format(body, sizeof body,
		"%sChao mung den voi %s!\n\n"\
		"Ten %s{FFFFFF} chua duoc dang ky.\n"\
		"Nhap mat khau de tao tai khoan moi (%d-%d ky tu).\n\n%s",
		EMBED_WHITE, GM_NAME, GetName(playerid),
		GM_MIN_PASSWORD_LEN, GM_MAX_PASSWORD_LEN,
		retry ? (EMBED_ERROR "Mat khau khong hop le, thu lai.") : (""));

	ShowPlayerDialog(playerid, retry ? DIALOG_REGISTER_RETRY : DIALOG_REGISTER,
		DIALOG_STYLE_PASSWORD, "Dang ky", body, "Dang ky", "Thoat");
	return 1;
}

stock Account_ShowLogin(playerid, bool:retry)
{
	new body[512], tail[48];

	if (retry)
	{
		format(tail, sizeof tail, EMBED_ERROR "Sai mat khau. Con %d lan thu.",
			GM_LOGIN_ATTEMPTS - Player[playerid][pLoginAttempts]);
	}
	else
	{
		tail[0] = EOS;
	}

	format(body, sizeof body,
		"%sChao mung tro lai, %s%s{FFFFFF}!\n\n"\
		"Nhap mat khau de dang nhap.\n\n%s",
		EMBED_WHITE, EMBED_INFO, GetName(playerid), tail);

	ShowPlayerDialog(playerid, retry ? DIALOG_LOGIN_RETRY : DIALOG_LOGIN,
		DIALOG_STYLE_PASSWORD, "Dang nhap", body, "Dang nhap", "Thoat");
	return 1;
}

static Account_MakeSalt(playerid)
{
	static const charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	new size = sizeof (charset) - 1;

	for (new i = 0; i < GM_SALT_LEN - 1; ++i)
	{
		g_Salt[playerid][i] = charset[random(size)];
	}
	g_Salt[playerid][GM_SALT_LEN - 1] = EOS;
	return 1;
}

stock Account_Register(playerid, const password[])
{
	new length = strlen(password);
	if (length < GM_MIN_PASSWORD_LEN || length > GM_MAX_PASSWORD_LEN)
	{
		Account_ShowRegister(playerid, true);
		return 0;
	}

	Account_MakeSalt(playerid);
	SHA256_PassHash(password, g_Salt[playerid], g_Hash[playerid], GM_HASH_LEN);

	new
		name[MAX_PLAYER_NAME + 1],
		escapedName[MAX_PLAYER_NAME * 2 + 1],
		ip[16],
		escapedIp[40],
		query[512];

	GetPlayerName(playerid, name, sizeof name);
	GetPlayerIp(playerid, ip, sizeof ip);
	DB_Escape(name, escapedName, sizeof escapedName);
	DB_Escape(ip, escapedIp, sizeof escapedIp);

	format(query, sizeof query,
		"INSERT INTO accounts (name, password, salt, ip, registered, last_login) "\
		"VALUES ('%s', '%s', '%s', '%s', %d, %d);",
		escapedName, g_Hash[playerid], g_Salt[playerid], escapedIp,
		gettime(), gettime());

	DB_QueryCB(query, "OnAccountCreated", playerid);
	return 1;
}

public OnAccountCreated(playerid, extra)
{
	#pragma unused extra

	if (!IsPlayerConnectedEx(playerid))
	{
		return 0;
	}

	Player[playerid][pAccountID] = DB_InsertId();
	if (Player[playerid][pAccountID] <= 0)
	{
		SendClientMessage(playerid, COLOR_ERROR, "Loi tao tai khoan, thu ket noi lai.");
		Kick(playerid);
		return 0;
	}

	new query[512];
	format(query, sizeof query,
		"INSERT INTO characters (account_id, cash, bank, level, skin, pos_x, pos_y, pos_z, pos_a) "\
		"VALUES (%d, %d, %d, 1, 26, %f, %f, %f, %f);",
		Player[playerid][pAccountID], GM_START_CASH, GM_START_BANK,
		SPAWN_DEFAULT_X, SPAWN_DEFAULT_Y, SPAWN_DEFAULT_Z, SPAWN_DEFAULT_A);
	DB_Query(query);

	SendClientMessage(playerid, COLOR_SUCCESS, "Dang ky thanh cong! Chao mung den voi %s.", GM_NAME);
	SendClientMessage(playerid, COLOR_INFO, "Ban nhan %s tien mat va %s trong ngan hang.",
		Money(GM_START_CASH), Money(GM_START_BANK));

	Account_LoadCharacter(playerid);
	return 1;
}

stock Account_Login(playerid, const password[])
{
	new attempt[GM_HASH_LEN];
	SHA256_PassHash(password, g_Salt[playerid], attempt, sizeof attempt);

	if (strcmp(attempt, g_Hash[playerid], false) != 0)
	{
		Player[playerid][pLoginAttempts]++;

		if (Player[playerid][pLoginAttempts] >= GM_LOGIN_ATTEMPTS)
		{
			SendClientMessage(playerid, COLOR_ERROR, "Sai mat khau qua nhieu lan.");
			Kick(playerid);
			return 0;
		}

		Account_ShowLogin(playerid, true);
		return 0;
	}

	new ip[16], escapedIp[40], query[256];
	GetPlayerIp(playerid, ip, sizeof ip);
	DB_Escape(ip, escapedIp, sizeof escapedIp);

	format(query, sizeof query,
		"UPDATE accounts SET last_login = %d, ip = '%s' WHERE id = %d;",
		gettime(), escapedIp, Player[playerid][pAccountID]);
	DB_Query(query);

	Account_LoadCharacter(playerid);
	return 1;
}

stock Account_LoadCharacter(playerid)
{
	new query[256];
	format(query, sizeof query,
		"SELECT * FROM characters WHERE account_id = %d LIMIT 1;",
		Player[playerid][pAccountID]);
	DB_QueryCB(query, "OnCharacterLoaded", playerid);
	return 1;
}

public OnCharacterLoaded(playerid, extra)
{
	#pragma unused extra

	if (!IsPlayerConnectedEx(playerid))
	{
		return 0;
	}

	if (!DB_Next())
	{
		// An account with no character row can only happen if a save was
		// interrupted mid-registration; rebuilding it beats kicking.
		new query[256];
		format(query, sizeof query,
			"INSERT INTO characters (account_id, cash, bank, level, skin) VALUES (%d, %d, %d, 1, 26);",
			Player[playerid][pAccountID], GM_START_CASH, GM_START_BANK);
		DB_Query(query);

		Player[playerid][pCash] = GM_START_CASH;
		Player[playerid][pBank] = GM_START_BANK;
		Player[playerid][pPosX] = SPAWN_DEFAULT_X;
		Player[playerid][pPosY] = SPAWN_DEFAULT_Y;
		Player[playerid][pPosZ] = SPAWN_DEFAULT_Z;
		Player[playerid][pPosA] = SPAWN_DEFAULT_A;
	}
	else
	{
		Player[playerid][pCharID]   = DB_Int("id");
		Player[playerid][pCash]     = DB_Int("cash");
		Player[playerid][pBank]     = DB_Int("bank");
		Player[playerid][pLevel]    = DB_Int("level");
		Player[playerid][pExp]      = DB_Int("exp");
		Player[playerid][pSkin]     = DB_Int("skin");
		Player[playerid][pHealth]   = DB_Float("health");
		Player[playerid][pArmour]   = DB_Float("armour");
		Player[playerid][pPosX]     = DB_Float("pos_x");
		Player[playerid][pPosY]     = DB_Float("pos_y");
		Player[playerid][pPosZ]     = DB_Float("pos_z");
		Player[playerid][pPosA]     = DB_Float("pos_a");
		Player[playerid][pInterior] = DB_Int("interior");
		Player[playerid][pWorld]    = DB_Int("world");
		Player[playerid][pJob]      = DB_Int("job");
		Player[playerid][pWanted]   = DB_Int("wanted");
		Player[playerid][pKills]    = DB_Int("kills");
		Player[playerid][pDeaths]   = DB_Int("deaths");
		Player[playerid][pArrests]  = DB_Int("arrests");
		Player[playerid][pCrew]     = DB_Int("crew_id");
		Player[playerid][pPhone]    = DB_Int("phone");

		if (Player[playerid][pLevel] < 1)
		{
			Player[playerid][pLevel] = 1;
		}
		if (Player[playerid][pHealth] <= 0.0)
		{
			Player[playerid][pHealth] = 100.0;
		}
		if (Player[playerid][pPosX] == 0.0 && Player[playerid][pPosY] == 0.0)
		{
			Player[playerid][pPosX] = SPAWN_DEFAULT_X;
			Player[playerid][pPosY] = SPAWN_DEFAULT_Y;
			Player[playerid][pPosZ] = SPAWN_DEFAULT_Z;
			Player[playerid][pPosA] = SPAWN_DEFAULT_A;
		}
	}

	// A phone number is handed out once, derived from the account id so it
	// stays the same across sessions.
	if (Player[playerid][pPhone] <= 0)
	{
		Player[playerid][pPhone] = 100000 + (Player[playerid][pAccountID] % 900000);
	}

	Player[playerid][pLoggedIn]     = true;
	Player[playerid][pSessionStart] = gettime();

	SetSpawnInfo(playerid, NO_TEAM, Player[playerid][pSkin],
		Player[playerid][pPosX], Player[playerid][pPosY], Player[playerid][pPosZ],
		Player[playerid][pPosA], WEAPON:0, 0, WEAPON:0, 0, WEAPON:0, 0);
	TogglePlayerSpectating(playerid, false);
	SpawnPlayer(playerid);

	Job_LoadProgress(playerid);
	Account_Greet(playerid);
	return 1;
}

static Account_Greet(playerid)
{
	new playtime[32];
	FormatDuration(playtime, sizeof playtime, Player[playerid][pPlaytime]);

	SendClientMessage(playerid, COLOR_INFO, "Dang nhap thanh cong.");
	SendClientMessage(playerid, COLOR_GREY,
		"Level %d | Tien mat %s | Ngan hang %s | Da choi %s",
		Player[playerid][pLevel], Money(Player[playerid][pCash]),
		Money(Player[playerid][pBank]), playtime);
	SendClientMessage(playerid, COLOR_INFO, "Bam nut MENU tren man hinh, hoac go /menu.");

	SendClientMessageToAll(COLOR_GREY, "* %s (id %d) vua vao server.", GetName(playerid), playerid);
	return 1;
}

stock Account_Save(playerid)
{
	if (!Player_IsLoggedIn(playerid) || Player[playerid][pAccountID] <= 0)
	{
		return 0;
	}

	Player_CaptureState(playerid);

	new query[DB_QUERY_LEN];

	format(query, sizeof query,
		"UPDATE accounts SET playtime = %d, admin = %d, vip = %d, banned = %d WHERE id = %d;",
		Player_Playtime(playerid), Player[playerid][pAdmin], Player[playerid][pVIP],
		Player[playerid][pBanned] ? 1 : 0, Player[playerid][pAccountID]);
	DB_Query(query);

	format(query, sizeof query,
		"UPDATE characters SET "\
		"cash = %d, bank = %d, level = %d, exp = %d, skin = %d, "\
		"health = %f, armour = %f, pos_x = %f, pos_y = %f, pos_z = %f, pos_a = %f, "\
		"interior = %d, world = %d, job = %d, "\
		"wanted = %d, kills = %d, deaths = %d, arrests = %d, crew_id = %d, phone = %d "\
		"WHERE account_id = %d;",
		Player[playerid][pCash], Player[playerid][pBank], Player[playerid][pLevel],
		Player[playerid][pExp], Player[playerid][pSkin],
		Player[playerid][pHealth], Player[playerid][pArmour],
		Player[playerid][pPosX], Player[playerid][pPosY], Player[playerid][pPosZ],
		Player[playerid][pPosA], Player[playerid][pInterior], Player[playerid][pWorld],
		Player[playerid][pJob],
		Player[playerid][pWanted], Player[playerid][pKills], Player[playerid][pDeaths],
		Player[playerid][pArrests], Player[playerid][pCrew], Player[playerid][pPhone],
		Player[playerid][pAccountID]);
	DB_Query(query);

	Job_SaveProgress(playerid);
	return 1;
}
