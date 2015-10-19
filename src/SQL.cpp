#include <stdio.h>

// beachJudge
#include <Judge/SQL.h>

namespace judge {

sqlite3 *g_sql = 0;

sqlite3_stmt *SQL::session_selectAll = 0;

bool SQL::Init()
{
	int f = SQLITE_OPEN_READWRITE
		| SQLITE_OPEN_CREATE;
	//TODO: Expand this error check
	int s = 0;
	//s = sqlite3_open_v2("judge.db", &g_sql, f, "");
	s = sqlite3_open("judge.db", &g_sql);
	if (s != SQLITE_OK) {
		printf("SQLite Error: %d\n", s);
		sqlite3_close_v2(g_sql);
		return false;
	}

	// Table Setup
	sqlite3_stmt *stmt = 0;
	s = sqlite3_prepare_v2(g_sql,
		"CREATE TABLE IF NOT EXISTS jd_session(\n"
			"Key VARCHAR(20),\n"
			"User INT,\n"
			"PRIMARY KEY(Key)\n"
		")", -1, &stmt, NULL);
	if (s != SQLITE_OK) {
		printf("ERR session_createTable: %d\n", s);
		return false;
	}
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	// Prepared Statements
	s = sqlite3_prepare_v2(g_sql,
		"SELECT * FROM jd_session",
		-1, &session_selectAll, NULL);
	if (s != SQLITE_OK) {
		printf("ERR session_selectAll: %d\n", s);
		return false;
	}

	return true;
}

void SQL::Cleanup()
{
	//TODO: Determine if we should sqlite3_finalize all the statements
	sqlite3_close_v2(g_sql);
}


}
