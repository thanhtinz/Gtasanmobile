/**
 * Database abstraction.
 *
 * Everything above this file speaks one API and never touches a driver native
 * directly, so identical gameplay sources compile against SQLite (zero setup,
 * good for a small server) or MySQL (what a busy one wants). The driver is
 * picked by tools/server-build.sh, which generates core/db_driver.inc.
 *
 * The API is callback-shaped even for SQLite — where queries are synchronous —
 * so game code is written once against the asynchronous model MySQL needs:
 *
 *     DB_QueryCB(query, "OnAccountLoaded", playerid);
 *
 *     forward OnAccountLoaded(playerid, extra);
 *     public OnAccountLoaded(playerid, extra)
 *     {
 *         if (!DB_Next()) return 0;          // no rows
 *         new money = DB_Int("cash");
 *     }
 *
 * Result accessors are valid only inside such a callback.
 */

#if defined _gtasan_db_included
	#endinput
#endif
#define _gtasan_db_included

#include "core/db_driver.inc"

#if !defined DB_USE_MYSQL && !defined DB_USE_SQLITE
	#error No database driver selected. Build through tools/server-build.sh.
#endif

#if defined DB_USE_MYSQL
	#include <a_mysql>
#endif

#define DB_QUERY_LEN            (2048)
#define DB_NO_EXTRA             (0)

// Column types differ between the engines; schema.inc builds its CREATE TABLE
// statements from these so the schema itself is written once.
#if defined DB_USE_MYSQL
	#define SQL_PK              "INT NOT NULL AUTO_INCREMENT PRIMARY KEY"
	#define SQL_TEXT            "VARCHAR(255) NOT NULL DEFAULT ''"
	#define SQL_NAME            "VARCHAR(24) NOT NULL DEFAULT ''"
	#define SQL_HASH            "VARCHAR(64) NOT NULL DEFAULT ''"
	#define SQL_SALT            "VARCHAR(16) NOT NULL DEFAULT ''"
	#define SQL_INT             "INT NOT NULL DEFAULT 0"
	#define SQL_BIGINT          "BIGINT NOT NULL DEFAULT 0"
	#define SQL_FLOAT           "FLOAT NOT NULL DEFAULT 0"
	#define SQL_TABLE_SUFFIX    " ENGINE=InnoDB DEFAULT CHARSET=utf8mb4"
	// Upsert clause. MySQL keys off whichever unique index the row collides
	// with, so it does not name the columns; SQLite requires them.
	#define SQL_UPSERT(%0)      "ON DUPLICATE KEY UPDATE "
#else
	#define SQL_PK              "INTEGER PRIMARY KEY AUTOINCREMENT"
	#define SQL_TEXT            "TEXT NOT NULL DEFAULT ''"
	#define SQL_NAME            "TEXT NOT NULL DEFAULT ''"
	#define SQL_HASH            "TEXT NOT NULL DEFAULT ''"
	#define SQL_SALT            "TEXT NOT NULL DEFAULT ''"
	#define SQL_INT             "INTEGER NOT NULL DEFAULT 0"
	#define SQL_BIGINT          "INTEGER NOT NULL DEFAULT 0"
	#define SQL_FLOAT           "REAL NOT NULL DEFAULT 0"
	#define SQL_TABLE_SUFFIX    ""
	#define SQL_UPSERT(%0)      "ON CONFLICT(" %0 ") DO UPDATE SET "
#endif

#if defined DB_USE_MYSQL
	static MySQL:g_SQL = MYSQL_INVALID_HANDLE;
#else
	static DB:g_SQL = DB:0;
	static DBResult:g_Result = DBResult:0;
	static g_ResultRows = 0;
#endif

// Row cursor shared by both drivers: -1 means "before the first row".
static g_Row = -1;

stock DB_DriverName()
{
#if defined DB_USE_MYSQL
	new name[8] = "MySQL";
#else
	new name[8] = "SQLite";
#endif
	return name;
}

// ---------------------------------------------------------------------------
// Connection lifecycle
// ---------------------------------------------------------------------------

stock bool:DB_Init()
{
#if defined DB_USE_MYSQL
	// Credentials live in server/mysql.ini so they never enter the repository.
	g_SQL = mysql_connect_file("mysql.ini");

	if (g_SQL == MYSQL_INVALID_HANDLE || mysql_errno(g_SQL) != 0)
	{
		new error[256];
		mysql_error(error, sizeof error, g_SQL);
		Log("MySQL connection failed: %s", error);
		Log("Check server/mysql.ini (copy server/mysql.ini.example).");
		return false;
	}

	mysql_set_charset("utf8mb4", g_SQL);
	Log("MySQL connected");
	return true;
#else
	g_SQL = DB_Open("gtasan.db");
	if (!g_SQL)
	{
		Log("SQLite could not open gtasan.db");
		return false;
	}

	// WAL keeps reads from blocking on the periodic save sweep; the busy
	// timeout stops a slow write from failing outright when queries collide.
	DB_Query("PRAGMA journal_mode = WAL;");
	DB_Query("PRAGMA synchronous = NORMAL;");
	DB_Query("PRAGMA busy_timeout = 3000;");
	DB_Query("PRAGMA foreign_keys = ON;");

	Log("SQLite opened (server/gtasan.db)");
	return true;
#endif
}

stock DB_Exit()
{
#if defined DB_USE_MYSQL
	if (g_SQL != MYSQL_INVALID_HANDLE)
	{
		mysql_close(g_SQL);
		g_SQL = MYSQL_INVALID_HANDLE;
	}
#else
	DB_ReleaseResult();
	if (g_SQL)
	{
		DB_Close(g_SQL);
		g_SQL = DB:0;
	}
#endif
	return 1;
}

// ---------------------------------------------------------------------------
// Escaping
// ---------------------------------------------------------------------------

/**
 * Escapes a value for interpolation into a query. Every string that reaches a
 * query goes through this — player names, chat text, crew names. `dest` should
 * be at least 2*strlen(src)+1 for the worst case.
 *
 * Percent signs are dropped rather than escaped. open.mp's DB_ExecuteQuery
 * accepts an optionally-formatted query, so a stray %s in stored text could be
 * read as a format specifier; nothing here legitimately stores one.
 */
stock DB_Escape(const src[], dest[], size = sizeof (dest))
{
#if defined DB_USE_MYSQL
	mysql_escape_string(src, dest, size, g_SQL);
#else
	new written = 0;
	for (new i = 0; src[i] != EOS && written < size - 1; ++i)
	{
		new c = src[i];
		if (c < ' ' || c == '%')
		{
			continue;
		}
		if (c == '\'')
		{
			if (written >= size - 2)
			{
				break;
			}
			dest[written++] = '\'';
		}
		dest[written++] = c;
	}
	dest[written] = EOS;
#endif
	return 1;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

/// Fire-and-forget, for writes nobody waits on.
stock DB_Query(const query[])
{
#if defined DB_USE_MYSQL
	if (!mysql_tquery(g_SQL, query))
	{
		Log("MySQL query failed: %s", query);
		return 0;
	}
#else
	new DBResult:result = DB_ExecuteQuery(g_SQL, query);
	if (!result)
	{
		Log("SQLite query failed: %s", query);
		return 0;
	}
	DB_FreeResultSet(result);
#endif
	return 1;
}

/**
 * Runs `query` and invokes `callback(playerid, extra)` with the result set
 * selected. On MySQL that happens on a later tick; on SQLite it happens before
 * this returns. Game code must not assume either.
 */
stock DB_QueryCB(const query[], const callback[], playerid = INVALID_PLAYER_ID, extra = DB_NO_EXTRA)
{
#if defined DB_USE_MYSQL
	if (!mysql_tquery(g_SQL, query, callback, "ii", playerid, extra))
	{
		Log("MySQL query failed: %s", query);
		return 0;
	}
	return 1;
#else
	// A nested query would clobber the single active result, so the previous
	// one is released first and callbacks must read before querying again.
	DB_ReleaseResult();

	g_Result = DB_ExecuteQuery(g_SQL, query);
	if (!g_Result)
	{
		Log("SQLite query failed: %s", query);
		return 0;
	}

	g_ResultRows = DB_GetRowCount(g_Result);
	g_Row = -1;

	if (callback[0] != EOS)
	{
		if (funcidx(callback) == -1)
		{
			Log("DB_QueryCB: callback '%s' does not exist", callback);
		}
		else
		{
			CallLocalFunction(callback, "ii", playerid, extra);
		}
	}

	DB_ReleaseResult();
	return 1;
#endif
}

#if !defined DB_USE_MYSQL
stock DB_ReleaseResult()
{
	if (g_Result)
	{
		DB_FreeResultSet(g_Result);
		g_Result = DBResult:0;
	}
	g_ResultRows = 0;
	g_Row = -1;
	return 1;
}
#endif

// ---------------------------------------------------------------------------
// Result access — valid only inside a DB_QueryCB callback
// ---------------------------------------------------------------------------

stock DB_Rows()
{
#if defined DB_USE_MYSQL
	new rows = 0;
	cache_get_row_count(rows);
	return rows;
#else
	return g_ResultRows;
#endif
}

/// Advances to the next row; false when the set is exhausted.
stock bool:DB_Next()
{
#if defined DB_USE_MYSQL
	return ++g_Row < DB_Rows();
#else
	if (!g_Result)
	{
		return false;
	}
	// SQLite results arrive already positioned on the first row, so the first
	// call must not advance the cursor.
	if (g_Row == -1)
	{
		g_Row = 0;
		return g_ResultRows > 0;
	}
	if (++g_Row >= g_ResultRows)
	{
		return false;
	}
	return !!DB_SelectNextRow(g_Result);
#endif
}

stock DB_Int(const field[])
{
#if defined DB_USE_MYSQL
	new value = 0;
	cache_get_value_name_int(g_Row, field, value);
	return value;
#else
	return g_Result ? DB_GetFieldIntByName(g_Result, field) : 0;
#endif
}

stock Float:DB_Float(const field[])
{
#if defined DB_USE_MYSQL
	new Float:value = 0.0;
	cache_get_value_name_float(g_Row, field, value);
	return value;
#else
	return g_Result ? DB_GetFieldFloatByName(g_Result, field) : 0.0;
#endif
}

stock DB_Str(const field[], dest[], size = sizeof (dest))
{
	dest[0] = EOS;
#if defined DB_USE_MYSQL
	cache_get_value_name(g_Row, field, dest, size);
#else
	if (g_Result)
	{
		DB_GetFieldStringByName(g_Result, field, dest, size);
	}
#endif
	return 1;
}

stock bool:DB_Bool(const field[])
{
	return DB_Int(field) != 0;
}

/**
 * Primary key of the row the last INSERT created.
 *
 * On MySQL this lives on the cache of the INSERT itself, so it is meaningful
 * only inside that query's callback. On SQLite it is per connection, which is
 * safe here because SQLite queries are synchronous — nothing can interleave.
 */
stock DB_InsertId()
{
#if defined DB_USE_MYSQL
	return cache_insert_id();
#else
	new id = 0;
	new DBResult:result = DB_ExecuteQuery(g_SQL, "SELECT last_insert_rowid() AS id;");
	if (result)
	{
		if (DB_GetRowCount(result) > 0)
		{
			id = DB_GetFieldIntByName(result, "id");
		}
		DB_FreeResultSet(result);
	}
	return id;
#endif
}
