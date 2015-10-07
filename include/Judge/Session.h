#ifndef _JUDGE_SESSION_H_
#define _JUDGE_SESSION_H_

// beachJudge
#include <Judge/User.h>
#include <Judge/Types.h>

namespace judge {

const u64 sessionExpireTimeMS = 1000 * 60 * 60 * 1;

typedef struct Session
{

	User *user;

	u64 expireTimeMS;

	Session() :
		user(0),
		expireTimeMS(0)
	{
	}

	Session(User *user) :
		user(user)
	{
		Reset();
	}

	Session(User *user, u64 timeMS) :
		user(user),
		expireTimeMS(timeMS)
	{
	}

	inline void Reset()
	{
		expireTimeMS = getRealTimeMS() + sessionExpireTimeMS;
	}


} Session;


}

#endif
