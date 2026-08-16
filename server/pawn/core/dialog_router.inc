/**
 * Single OnDialogResponse, dispatching by id.
 *
 * Included last so every module's handler is already visible. Keeping dialog
 * ownership in one file is what makes the "Quay lai" button behave predictably:
 * each leaf knows which screen it came from.
 */

#if defined _gtasan_dialog_router_included
	#endinput
#endif
#define _gtasan_dialog_router_included

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
	switch (dialogid)
	{
		// --- accounts ------------------------------------------------------
		case DIALOG_REGISTER, DIALOG_REGISTER_RETRY:
		{
			if (!response)
			{
				Kick(playerid);
				return 1;
			}
			Account_Register(playerid, inputtext);
			return 1;
		}

		case DIALOG_LOGIN, DIALOG_LOGIN_RETRY:
		{
			if (!response)
			{
				Kick(playerid);
				return 1;
			}
			Account_Login(playerid, inputtext);
			return 1;
		}

		// --- panel root ----------------------------------------------------
		case DIALOG_MENU_ROOT:
		{
			if (!response)
			{
				return 1;
			}

			switch (listitem)
			{
				case MENUROOT_CHARACTER: Menu_ShowCharacter(playerid);
				case MENUROOT_JOB:       Menu_ShowJobRoot(playerid);
				case MENUROOT_BANK:      Menu_ShowBank(playerid);
				case MENUROOT_MAP:       Menu_ShowMap(playerid);
				case MENUROOT_SETTINGS:  Menu_ShowSettings(playerid);
				case MENUROOT_HELP:      Menu_ShowHelp(playerid);
			}
			return 1;
		}

		// Leaf screens whose left button is "Quay lai".
		case DIALOG_MENU_CHARACTER, DIALOG_HELP:
		{
			if (response)
			{
				Menu_ShowRoot(playerid);
			}
			return 1;
		}

		// --- jobs ----------------------------------------------------------
		case DIALOG_MENU_JOB:
		{
			if (!response)
			{
				Menu_ShowRoot(playerid);
				return 1;
			}

			// Two fixed actions, then one entry per category.
			switch (listitem)
			{
				case 0: Job_StartShift(playerid);
				case 1: Job_Leave(playerid, true);
				default: Menu_ShowJobList(playerid, listitem - 2);
			}
			return 1;
		}

		case DIALOG_JOB_LIST:
		{
			if (!response)
			{
				Menu_ShowJobRoot(playerid);
				return 1;
			}

			new jobid = Menu_JobFromListitem(playerid, listitem);
			if (jobid != JOB_NONE)
			{
				Job_Join(playerid, jobid);
			}
			return 1;
		}

		case DIALOG_JOB_QUIT:
		{
			if (response)
			{
				Job_Leave(playerid, true);
			}
			return 1;
		}

		// --- bank ----------------------------------------------------------
		case DIALOG_MENU_BANK:
		{
			if (!response)
			{
				Menu_ShowRoot(playerid);
				return 1;
			}

			if (!Bank_IsPlayerAtBank(playerid))
			{
				SendClientMessage(playerid, COLOR_ERROR, "Ban khong con o ngan hang.");
				return 1;
			}

			switch (listitem)
			{
				case 0:
					ShowPlayerDialog(playerid, DIALOG_BANK_DEPOSIT, DIALOG_STYLE_INPUT,
						"Gui tien", "Nhap so tien muon gui vao ngan hang:", "Gui", "Huy");
				case 1:
					ShowPlayerDialog(playerid, DIALOG_BANK_WITHDRAW, DIALOG_STYLE_INPUT,
						"Rut tien", "Nhap so tien muon rut:", "Rut", "Huy");
				case 2:
					SendClientMessage(playerid, COLOR_MONEY, "So du: %s | Tien mat: %s",
						Money(Player[playerid][pBank]), Money(Player[playerid][pCash]));
			}
			return 1;
		}

		case DIALOG_BANK_DEPOSIT, DIALOG_BANK_WITHDRAW:
		{
			if (!response)
			{
				Menu_ShowBank(playerid);
				return 1;
			}
			Bank_OnAmount(playerid, dialogid, inputtext);
			return 1;
		}

		// --- map -----------------------------------------------------------
		case DIALOG_MENU_MAP:
		{
			if (!response)
			{
				Menu_ShowRoot(playerid);
				return 1;
			}

			switch (listitem)
			{
				case 0:
				{
					if (Player[playerid][pJob] == JOB_NONE)
					{
						SendClientMessage(playerid, COLOR_GREY, "Ban chua co nghe.");
					}
					else
					{
						Job_MarkWorkpoint(playerid);
						SendClientMessage(playerid, COLOR_INFO, "Da danh dau diem lam viec.");
					}
				}
				case 1: Bank_MarkNearest(playerid);
				case 2:
				{
					SetPlayerMapIcon(playerid, 3, 1172.0, -1323.2, 15.4, 22, 0, MAPICON_GLOBAL);
					SendClientMessage(playerid, COLOR_INFO, "Da danh dau benh vien.");
				}
				case 3:
				{
					RemovePlayerMapIcon(playerid, 2);
					RemovePlayerMapIcon(playerid, 3);
					SendClientMessage(playerid, COLOR_GREY, "Da xoa danh dau.");
				}
			}
			return 1;
		}

		// --- settings ------------------------------------------------------
		case DIALOG_MENU_SETTINGS:
		{
			if (!response)
			{
				Menu_ShowRoot(playerid);
				return 1;
			}

			switch (listitem)
			{
				case 0:
				{
					SendClientMessage(playerid, COLOR_INFO, "%s v%s - %s",
						GM_NAME, GM_VERSION, DB_DriverName());
				}
				case 1: CallLocalFunction("cmd_players", "is", playerid, "");
				case 2:
				{
					SendClientMessage(playerid, COLOR_INFO,
						"Micro: bam nut TALK tren man hinh khi dang trong game.");
					SendClientMessage(playerid, COLOR_GREY,
						"Neu khong thay nut, bat VoiceChatEnable trong settings.ini cua client.");
				}
			}
			return 1;
		}
	}
	return 0;
}
