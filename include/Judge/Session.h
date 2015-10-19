#ifndef _JUDGE_SESSION_H_
#define _JUDGE_SESSION_H_

#include <map>
#include <string>

// beachJudge
#include <Judge/Base.h>
#include <Judge/User.h>
#include <Judge/Types.h>
#include <Judge/SQL.h>

using namespace std;

namespace judge {

const u64 sessionExpireTimeMS = 1000 * 60 * 60 * 1;

typedef struct Session
{

	static std::map<std::string, Session> s_sessionMap;

	std::string key;

	User *user;

	u64 expireTimeMS;

	Session() :
		key(""),
		user(0),
		expireTimeMS(0)
	{
	}

	Session(const char *key, User *user) :
		key(key),
		user(user)
	{
		Reset();
	}

	Session(const char *key, User *user, u64 timeMS) :
		key(key),
		user(user),
		expireTimeMS(timeMS)
	{
	}

	inline void Reset()
	{
		expireTimeMS = getRealTimeMS() + sessionExpireTimeMS;
	}

	inline void SQL_Insert()
	{
		sqlite3_bind_text(SQL::session_insert,
			1, key.c_str(), key.length(), 0);
		sqlite3_bind_int(SQL::session_insert,
			2, user->id);
		sqlite3_bind_int64(SQL::session_insert,
			3, expireTimeMS);
		sqlite3_step(SQL::session_insert);
		sqlite3_reset(SQL::session_insert);
	}

	inline void SQL_Sync()
	{
		sqlite3_bind_int64(SQL::session_update,
			1, expireTimeMS);
		sqlite3_bind_text(SQL::session_update,
			2, key.c_str(), key.length(), 0);
		sqlite3_step(SQL::session_update);
		sqlite3_reset(SQL::session_update);
	}

	inline void SQL_Delete()
	{
		sqlite3_bind_text(SQL::session_delete,
			1, key.c_str(), key.length(), 0);
		sqlite3_step(SQL::session_delete);
		sqlite3_reset(SQL::session_delete);
	}

	static inline void SQL_LoadAll()
	{
		const char *key;
		std::string k;
		u16 userID;
		u64 expire;
		while (sqlite3_step(SQL::session_selectAll)
				!= SQLITE_DONE) {
			key = (const char *)sqlite3_column_text(SQL::session_selectAll, 0);
			userID = sqlite3_column_int(SQL::session_selectAll, 1);
			expire = sqlite3_column_int64(SQL::session_selectAll, 2);
			//TODO: Handle expiration

			if (User::s_usersById.count(userID) == 0) {
				//TODO: Handle invalid user
				continue;
			}
			k = std::string(key);
			s_sessionMap[k] = Session(key, User::s_usersById[userID], expire);
		}
		sqlite3_reset(SQL::session_selectAll);
	}


} Session;


}

#endif
