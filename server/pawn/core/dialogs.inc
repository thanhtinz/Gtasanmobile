/**
 * Dialog identifiers.
 *
 * Response handling lives in core/dialog_router.inc, which is included last so
 * it can see every module. Keeping the ids in one table makes collisions
 * impossible to introduce by accident.
 */

#if defined _gtasan_dialogs_included
	#endinput
#endif
#define _gtasan_dialogs_included

enum
{
	DIALOG_NONE = 0,

	// accounts
	DIALOG_REGISTER = 1000,
	DIALOG_LOGIN,
	DIALOG_REGISTER_RETRY,
	DIALOG_LOGIN_RETRY,

	// the player panel
	DIALOG_MENU_ROOT = 1100,
	DIALOG_MENU_CHARACTER,
	DIALOG_MENU_JOB,
	DIALOG_MENU_BANK,
	DIALOG_MENU_CREW,
	DIALOG_MENU_MAP,
	DIALOG_MENU_SETTINGS,

	// information
	DIALOG_STATS = 1200,
	DIALOG_HELP,

	// economy
	DIALOG_BANK_DEPOSIT = 1300,
	DIALOG_BANK_WITHDRAW,

	// jobs
	DIALOG_JOB_CATEGORY = 1400,
	DIALOG_JOB_LIST,
	DIALOG_JOB_QUIT,

	// crew
	DIALOG_CREW_CREATE = 1500
}
