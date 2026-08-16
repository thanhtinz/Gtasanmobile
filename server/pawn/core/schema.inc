/**
 * Table definitions.
 *
 * Written once against the type macros in core/db.inc so the same statements
 * work on both engines. Everything is IF NOT EXISTS, so this runs on each boot
 * and is the only place the schema is described.
 */

#if defined _gtasan_schema_included
	#endinput
#endif
#define _gtasan_schema_included

stock Schema_Create()
{
	new query[DB_QUERY_LEN];

	// --- accounts ----------------------------------------------------------
	format(query, sizeof query,
		"CREATE TABLE IF NOT EXISTS accounts ("\
		"id %s,"\
		"name %s,"\
		"password %s,"\
		"salt %s,"\
		"ip %s,"\
		"admin %s,"\
		"vip %s,"\
		"banned %s,"\
		"registered %s,"\
		"last_login %s,"\
		"playtime %s"\
		")%s;",
		SQL_PK, SQL_NAME, SQL_HASH, SQL_SALT, SQL_TEXT,
		SQL_INT, SQL_INT, SQL_INT, SQL_BIGINT, SQL_BIGINT, SQL_INT,
		SQL_TABLE_SUFFIX);
	DB_Query(query);

	DB_Query("CREATE UNIQUE INDEX IF NOT EXISTS idx_accounts_name ON accounts (name);");

	// --- per-character state -----------------------------------------------
	// Split from `accounts` so a future multi-character feature only adds rows
	// here rather than reshaping the login table.
	format(query, sizeof query,
		"CREATE TABLE IF NOT EXISTS characters ("\
		"id %s,"\
		"account_id %s,"\
		"cash %s,"\
		"bank %s,"\
		"level %s,"\
		"exp %s,"\
		"skin %s,"\
		"health %s,"\
		"armour %s,"\
		"pos_x %s,"\
		"pos_y %s,"\
		"pos_z %s,"\
		"pos_a %s,"\
		"interior %s,"\
		"world %s,"\
		"job %s,"\
		"wanted %s,"\
		"kills %s,"\
		"deaths %s,"\
		"arrests %s,"\
		"crew_id %s,"\
		"phone %s"\
		")%s;",
		SQL_PK, SQL_INT, SQL_BIGINT, SQL_BIGINT, SQL_INT, SQL_INT, SQL_INT,
		SQL_FLOAT, SQL_FLOAT, SQL_FLOAT, SQL_FLOAT, SQL_FLOAT, SQL_FLOAT,
		SQL_INT, SQL_INT, SQL_INT, SQL_INT, SQL_INT, SQL_INT, SQL_INT,
		SQL_INT, SQL_INT,
		SQL_TABLE_SUFFIX);
	DB_Query(query);

	DB_Query("CREATE INDEX IF NOT EXISTS idx_characters_account ON characters (account_id);");

	// --- per-job progression -----------------------------------------------
	// One row per (account, job). Ranking up a trade must not reset the others,
	// which a single job_rank column on `characters` could not express.
	format(query, sizeof query,
		"CREATE TABLE IF NOT EXISTS job_progress ("\
		"id %s,"\
		"account_id %s,"\
		"job_id %s,"\
		"rank %s,"\
		"exp %s,"\
		"runs %s,"\
		"earned %s"\
		")%s;",
		SQL_PK, SQL_INT, SQL_INT, SQL_INT, SQL_INT, SQL_INT, SQL_BIGINT,
		SQL_TABLE_SUFFIX);
	DB_Query(query);

	DB_Query("CREATE UNIQUE INDEX IF NOT EXISTS idx_job_progress ON job_progress (account_id, job_id);");

	// --- audit trail for money movements -----------------------------------
	format(query, sizeof query,
		"CREATE TABLE IF NOT EXISTS money_log ("\
		"id %s,"\
		"account_id %s,"\
		"amount %s,"\
		"balance %s,"\
		"reason %s,"\
		"stamp %s"\
		")%s;",
		SQL_PK, SQL_INT, SQL_BIGINT, SQL_BIGINT, SQL_TEXT, SQL_BIGINT,
		SQL_TABLE_SUFFIX);
	DB_Query(query);

	Log("schema ready");
	return 1;
}
