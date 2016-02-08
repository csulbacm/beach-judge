#ifndef _JUDGE_SQL_H_
#define _JUDGE_SQL_H_

#include <string>
#include <chrono>

// sqlite
#include <sqlite3.h>

// beachJudge
#include <Judge/Types.h>

namespace judge {

extern sqlite3 *g_sql;

inline std::string TimeStampToDateTime(u64 &ts)
{
	time_t t = (time_t)ts;
	std::tm *ptm = std::gmtime(&t);
	char buffer[32];
	std::strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", ptm); 
	return std::string(buffer);
}
inline u64 DateTimeEscToTimeStamp(const char *dt, i32 offset)
{
	//TODO: Verify security
	u16 Y, M, D, h, m, s;
	std::sscanf(dt, "%d-%d-%d+%d%%3A%d%%3A%d", &Y, &M, &D, &h, &m, &s);
	struct tm *ti;
	time_t _s, ls, gs, t = 0;
	std::time(&_s);
	ti = std::localtime(&_s);
	ls = std::mktime(ti);
	ti = std::gmtime(&_s);
	gs = std::mktime(ti);
	ti = std::gmtime(&t);
	ti->tm_year = Y - 1900;
	ti->tm_mon = M - 1; ti->tm_mday = D;
	ti->tm_hour = h;
	ti->tm_min = m;
	ti->tm_sec = s;
	t = std::mktime(ti) + ls - gs;
	return (u64)t - offset * 3600;
}
inline u64 DateTimeToTimeStamp(const char *dt, i32 offset = 0)
{
	//TODO: Verify security
	u16 Y, M, D, h, m, s;
	std::sscanf(dt, "%d-%d-%d %d:%d:%d", &Y, &M, &D, &h, &m, &s);
	struct tm *ti;
	time_t _s, ls, gs, t = 0;
	std::time(&_s);
	ti = std::localtime(&_s);
	ls = std::mktime(ti);
	ti = std::gmtime(&_s);
	gs = std::mktime(ti);
	ti = std::gmtime(&t);
	ti->tm_year = Y - 1900;
	ti->tm_mon = M - 1; ti->tm_mday = D;
	ti->tm_hour = h;
	ti->tm_min = m;
	ti->tm_sec = s;
	t = std::mktime(ti) + ls - gs;
	return (u64)t - offset * 3600;
}

typedef struct SQL
{

	static bool Init();

	static void Cleanup();


	// SQL Statements
	static sqlite3_stmt
		*problem_selectAll,
		*problem_insert,
		*problem_update,
		*problem_delete,
		*problemSet_selectAll,
		*problemSet_insert,
		*problemSet_update,
		*problemSet_delete,
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
