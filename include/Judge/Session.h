#ifndef _JUDGE_SESSION_H_
#define _JUDGE_SESSION_H_

#include <Judge/User.h>

namespace judge {

const unsigned long long sessionExpireTimeMS = 1000 * 60 * 60 * 1;

typedef struct Session
{

	User *user;

	unsigned long long expireTimeMS;

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

	Session(User *user, unsigned long long timeMS) :
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
