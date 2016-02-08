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

sqlite3_stmt
	*SQL::problemSet_selectAll = 0,
	*SQL::problemSet_insert = 0,
	*SQL::problemSet_update = 0,
	*SQL::problemSet_delete = 0,
	*SQL::userGroup_selectAll = 0,
	*SQL::userGroup_insert = 0,
	*SQL::userGroup_update = 0,
	*SQL::userGroup_delete = 0,
	*SQL::user_selectAll = 0,
	*SQL::user_insert = 0,
	*SQL::user_update = 0,
	*SQL::user_delete = 0,
	*SQL::session_selectAll = 0,
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


	//--------------- Tables ------------------
	
	sqlite3_stmt *stmt = 0;
	
	// UserGroup
	SQL_STMT(stmt, 
		"CREATE TABLE IF NOT EXISTS jd_problemSet(\n"
			"ID INT,\n"
			"Name VARCHAR,\n"
			"PRIMARY KEY(ID)\n"
		")", s);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	// UserGroup
	SQL_STMT(stmt, 
		"CREATE TABLE IF NOT EXISTS jd_userGroup(\n"
			"ID INT,\n"
			"Name VARCHAR,\n"
			"IsActive BOOLEAN,\n"
			"PRIMARY KEY(ID)\n"
		")", s);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	// User
	SQL_STMT(stmt, 
		"CREATE TABLE IF NOT EXISTS jd_user(\n"
			"ID INT,\n"
			"Name VARCHAR,\n"
			"Password CHAR(64),\n"
			"Display VARCHAR,\n"
			"Level INT,\n"
			"GroupID INT,\n"
			"PRIMARY KEY(ID)\n"
		")", s);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	
	// Session
	SQL_STMT(stmt, 
		"CREATE TABLE IF NOT EXISTS jd_session(\n"
			"Key CHAR(20),\n"
			"User INT,\n"
			"Expire INT,\n"
			"PRIMARY KEY(Key)\n"
		")", s);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);


	//-------------- ProblemSet ---------------
	//TODO: Split update
	
	SQL_STMT(problemSet_selectAll, 
		"SELECT * FROM jd_problemSet", s);
	SQL_STMT(problemSet_insert,
		"INSERT INTO jd_problemSet VALUES (?,?)", s);
	SQL_STMT(problemSet_update,
		"UPDATE jd_problemSet SET Name=? WHERE ID=?", s);
	SQL_STMT(problemSet_delete,
		"DELETE FROM jd_problemSet WHERE ID=?", s);


	//-------------- UserGroup ----------------
	//TODO: Split update
	
	SQL_STMT(userGroup_selectAll, 
		"SELECT * FROM jd_userGroup", s);
	SQL_STMT(userGroup_insert,
		"INSERT INTO jd_userGroup VALUES (?,?,?)", s);
	SQL_STMT(userGroup_update,
		"UPDATE jd_userGroup SET Name=?, IsActive=? WHERE ID=?", s);
	SQL_STMT(userGroup_delete,
		"DELETE FROM jd_userGroup WHERE ID=?", s);


	//---------------- User -------------------
	//TODO: Split update
	
	SQL_STMT(user_selectAll, 
		"SELECT * FROM jd_user", s);
	SQL_STMT(user_insert,
		"INSERT INTO jd_user VALUES (?,?,?,?,?,?)", s);
	SQL_STMT(user_update,
		"UPDATE jd_user SET Name=?, Password=?, Display=?, Level=?, GroupID=? WHERE ID=?", s);
	SQL_STMT(user_delete,
		"DELETE FROM jd_user WHERE ID=?", s);


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
