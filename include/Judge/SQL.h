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
	static sqlite3_stmt
		*userGroup_selectAll,
		*userGroup_insert,
		*userGroup_update,
		*userGroup_delete,
		*user_selectAll,
		*user_insert,
		*user_update,
		*user_delete,
		*session_selectAll,
		*session_insert,
		*session_update,
		*session_delete;


} SQL;


}

#endif
