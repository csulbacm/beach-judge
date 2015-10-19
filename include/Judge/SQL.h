#ifndef _JUDGE_SQL_H_
#define _JUDGE_SQL_H_

// sqlite
#include <sqlite3.h>

namespace judge {

extern sqlite3 *g_sql;

typedef struct SQL
{

	static bool Init();

	static void Cleanup();


	// SQL Statements
	
	static sqlite3_stmt *session_selectAll;


} SQL;


}

#endif
