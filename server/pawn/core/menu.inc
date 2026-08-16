/**
 * The player panel.
 *
 * Everything the gamemode can do is reachable from /menu, which the mobile
 * client opens with an on-screen button so nobody has to type a slash command
 * on a phone keyboard. The chat commands still exist and still work — the menu
 * calls the same functions rather than duplicating their logic.
 *
 * Dialogs are the right surface for this: the Android client renders them as
 * native views with a real scrolling list, so the touch experience is better
 * than anything drawn into the game overlay, and changing the menu never
 * requires shipping a new APK.
 */

#if defined _gtasan_menu_included
	#endinput
#endif
#define _gtasan_menu_included

// Root entries, in display order. Kept as an enum so the listitem indices in
// the router cannot drift out of sync with the text below.
enum
{
	MENUROOT_CHARACTER,
	MENUROOT_JOB,
	MENUROOT_BANK,
	MENUROOT_MAP,
	MENUROOT_SETTINGS,
	MENUROOT_HELP
}

stock Menu_ShowRoot(playerid)
{
	if (!Player_IsLoggedIn(playerid))
	{
		return 0;
	}

	new
		body[640],
		jobName[26],
		shift[64];

	Job_Name(Player[playerid][pJob], jobName, sizeof jobName);
	Job_ShiftInfo(playerid, shift, sizeof shift);

	format(body, sizeof body,
		"%sNhan vat\t%sLevel %d\n"\
		"%sNghe nghiep\t%s%s\n"\
		"%sNgan hang\t%s%s\n"\
		"%sBan do\t%s%s\n"\
		"%sCai dat\t%s-\n"\
		"%sHuong dan\t%s-",
		EMBED_WHITE, EMBED_LEVEL, Player[playerid][pLevel],
		EMBED_WHITE, EMBED_JOB, jobName,
		EMBED_WHITE, EMBED_MONEY, Money(Player[playerid][pBank]),
		EMBED_WHITE, EMBED_GREY, shift,
		EMBED_WHITE, EMBED_GREY,
		EMBED_WHITE, EMBED_GREY);

	ShowPlayerDialog(playerid, DIALOG_MENU_ROOT, DIALOG_STYLE_TABLIST,
		GM_NAME, body, "Chon", "Dong");
	return 1;
}

// ---------------------------------------------------------------------------
// Character
// ---------------------------------------------------------------------------

stock Menu_ShowCharacter(playerid)
{
	new
		body[768],
		jobName[26],
		playtime[32],
		zone[40];

	Job_Name(Player[playerid][pJob], jobName, sizeof jobName);
	FormatDuration(playtime, sizeof playtime, Player_Playtime(playerid));
	Zone_OfPlayer(playerid, zone, sizeof zone);

	format(body, sizeof body,
		"%sTen\t%s%s\n"\
		"%sLevel\t%s%d  (%d/%d RP)\n"\
		"%sTien mat\t%s%s\n"\
		"%sNgan hang\t%s%s\n"\
		"%sNghe\t%s%s (bac %d)\n"\
		"%sHa guc / Chet\t%s%d / %d\n"\
		"%sBat giu\t%s%d\n"\
		"%sTruy na\t%s%d sao\n"\
		"%sDien thoai\t%s%d\n"\
		"%sVi tri\t%s%s\n"\
		"%sThoi gian choi\t%s%s",
		EMBED_GREY, EMBED_WHITE, GetName(playerid),
		EMBED_GREY, EMBED_LEVEL, Player[playerid][pLevel], Player[playerid][pExp],
			Level_Required(Player[playerid][pLevel]),
		EMBED_GREY, EMBED_MONEY, Money(Player[playerid][pCash]),
		EMBED_GREY, EMBED_MONEY, Money(Player[playerid][pBank]),
		EMBED_GREY, EMBED_JOB, jobName, Job_Rank(playerid, Player[playerid][pJob]),
		EMBED_GREY, EMBED_WHITE, Player[playerid][pKills], Player[playerid][pDeaths],
		EMBED_GREY, EMBED_WHITE, Player[playerid][pArrests],
		EMBED_GREY, EMBED_WHITE, Player[playerid][pWanted],
		EMBED_GREY, EMBED_WHITE, Player[playerid][pPhone],
		EMBED_GREY, EMBED_WHITE, zone,
		EMBED_GREY, EMBED_WHITE, playtime);

	ShowPlayerDialog(playerid, DIALOG_MENU_CHARACTER, DIALOG_STYLE_TABLIST,
		"Nhan vat", body, "Quay lai", "Dong");
	return 1;
}

// ---------------------------------------------------------------------------
// Jobs
// ---------------------------------------------------------------------------

stock Menu_ShowJobRoot(playerid)
{
	new
		body[512],
		jobName[26],
		row[64];

	Job_Name(Player[playerid][pJob], jobName, sizeof jobName);

	format(body, sizeof body, "%sVao ca lam\n%sNghi viec\n", EMBED_WHITE, EMBED_WHITE);
	for (new category = 0; category < JOBCAT_COUNT; ++category)
	{
		new name[24];
		Job_CategoryName(category, name, sizeof name);
		format(row, sizeof row, "%s%s\n", EMBED_JOB, name);
		strcat(body, row);
	}

	format(row, sizeof row, "%sNghe hien tai: %s", EMBED_GREY, jobName);
	strcat(body, row);

	ShowPlayerDialog(playerid, DIALOG_MENU_JOB, DIALOG_STYLE_LIST,
		"Nghe nghiep", body, "Chon", "Quay lai");
	return 1;
}

/// Job list for one category. The category is carried in pMenuPage so the
/// router can map a listitem back to a real job id.
stock Menu_ShowJobList(playerid, category)
{
	if (!(0 <= category < JOBCAT_COUNT))
	{
		return 0;
	}

	Player[playerid][pMenuPage] = category;

	new
		body[2048],
		row[160],
		title[40];

	format(body, sizeof body, "Nghe\tLevel\tBac\tLuong/chang\n");

	for (new jobid = 0; jobid < Job_Count(); ++jobid)
	{
		if (Job_Category(jobid) != category)
		{
			continue;
		}
		Job_ListRow(playerid, jobid, row, sizeof row);
		strcat(body, row);
		strcat(body, "\n");
	}

	Job_CategoryName(category, title, sizeof title);
	ShowPlayerDialog(playerid, DIALOG_JOB_LIST, DIALOG_STYLE_TABLIST_HEADERS,
		title, body, "Nhan viec", "Quay lai");
	return 1;
}

/// Maps a listitem inside a category page back to a job id.
stock Menu_JobFromListitem(playerid, listitem)
{
	new
		category = Player[playerid][pMenuPage],
		seen = 0;

	for (new jobid = 0; jobid < Job_Count(); ++jobid)
	{
		if (Job_Category(jobid) != category)
		{
			continue;
		}
		if (seen == listitem)
		{
			return jobid;
		}
		++seen;
	}
	return JOB_NONE;
}

// ---------------------------------------------------------------------------
// Bank
// ---------------------------------------------------------------------------

stock Menu_ShowBank(playerid)
{
	new body[384];
	format(body, sizeof body,
		"%sGui tien\n"\
		"%sRut tien\n"\
		"%sXem so du\n"\
		"%sTien mat %s | Ngan hang %s",
		EMBED_WHITE, EMBED_WHITE, EMBED_WHITE,
		EMBED_GREY, Money(Player[playerid][pCash]), Money(Player[playerid][pBank]));

	ShowPlayerDialog(playerid, DIALOG_MENU_BANK, DIALOG_STYLE_LIST,
		"Ngan hang", body, "Chon", "Quay lai");
	return 1;
}

// ---------------------------------------------------------------------------
// Map
// ---------------------------------------------------------------------------

stock Menu_ShowMap(playerid)
{
	new body[384];
	format(body, sizeof body,
		"%sDiem lam viec cua nghe hien tai\n"\
		"%sNgan hang gan nhat\n"\
		"%sBenh vien gan nhat\n"\
		"%sXoa danh dau",
		EMBED_WHITE, EMBED_WHITE, EMBED_WHITE, EMBED_GREY);

	ShowPlayerDialog(playerid, DIALOG_MENU_MAP, DIALOG_STYLE_LIST,
		"Ban do", body, "Chon", "Quay lai");
	return 1;
}

// ---------------------------------------------------------------------------
// Help
// ---------------------------------------------------------------------------

stock Menu_ShowHelp(playerid)
{
	new body[1024];
	format(body, sizeof body,
		"%s%s v%s\n\n"\
		"%sBam nut MENU tren man hinh de mo bang nay bat cu luc nao.\n\n"\
		"%sCach kiem tien:\n"\
		"%s- Mo MENU, chon Nghe nghiep, chon mot nhom nghe roi nhan viec.\n"\
		"%s- Di den diem lam viec da danh dau tren ban do.\n"\
		"%s- Chon Vao ca lam, roi lai xe toi tung diem duoc danh dau.\n"\
		"%s- Moi chang tra tien theo quang duong. Lam nhieu thi len bac, luong tang.\n\n"\
		"%sLenh chat van dung duoc: /menu /stats /work /jobs /bank /pay /pm /me\n\n"\
		"%sTien trong nguoi mat mot phan khi chet, tien trong ngan hang thi khong.",
		EMBED_INFO, GM_NAME, GM_VERSION,
		EMBED_WHITE,
		EMBED_LEVEL, EMBED_WHITE, EMBED_WHITE, EMBED_WHITE, EMBED_WHITE,
		EMBED_GREY, EMBED_MONEY);

	ShowPlayerDialog(playerid, DIALOG_HELP, DIALOG_STYLE_MSGBOX,
		"Huong dan", body, "Quay lai", "Dong");
	return 1;
}

// ---------------------------------------------------------------------------
// Settings
// ---------------------------------------------------------------------------

stock Menu_ShowSettings(playerid)
{
	new body[384];
	format(body, sizeof body,
		"%sXem thong tin server\n"\
		"%sDanh sach nguoi choi\n"\
		"%sHuong dan bat micro",
		EMBED_WHITE, EMBED_WHITE, EMBED_WHITE);

	ShowPlayerDialog(playerid, DIALOG_MENU_SETTINGS, DIALOG_STYLE_LIST,
		"Cai dat", body, "Chon", "Quay lai");
	return 1;
}
