#include <stdio.h>

// beachJudge
#include <Judge/SQL.h>

namespace judge {

#define SQL_STMT(stmt, qry, ret) \
	ret = sqlite3_prepare_v2(g_sql, qry, -1, &stmt, NULL); \
	if (ret != SQLITE_OK) { \
		printf("ERR "#stmt": %d\n", ret); \
		return false; \
	}

sqlite3 *g_sql = 0;

sqlite3_stmt *SQL::session_selectAll = 0,
	*SQL::session_insert = 0,
	*SQL::session_update = 0,
	*SQL::session_delete = 0;

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


	//-----------------------------------------
	//--------------- Tables ------------------
	
	// Session
	sqlite3_stmt *stmt = 0;
	SQL_STMT(stmt, 
		"CREATE TABLE IF NOT EXISTS jd_session(\n"
			"Key VARCHAR(20),\n"
			"User INT,\n"
			"Expire INT,\n"
			"PRIMARY KEY(Key)\n"
		")", s);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);


	//-----------------------------------------
	//--------------- Session -----------------

	SQL_STMT(session_selectAll, 
		"SELECT * FROM jd_session", s);
	SQL_STMT(session_insert,
		"INSERT INTO jd_session VALUES (?,?,?)", s);
	SQL_STMT(session_update,
		"UPDATE jd_session SET Expire=? WHERE Key=?", s);
	SQL_STMT(session_delete,
		"DELETE FROM jd_session WHERE Key=?", s);


	return true;
}

void SQL::Cleanup()
{
	//TODO: Determine if we should sqlite3_finalize all the statements
	sqlite3_close_v2(g_sql);
}


}
