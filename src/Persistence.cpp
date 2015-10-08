#include <stdio.h>
#include <fstream>

// beachJudge
#include <Judge/Session.h>
#include <Judge/User.h>
#include <Judge/Persistence.h>

namespace judge {

void loadJudgeData()
{
	// Create default judge account
	//TODO: Turn this into an administrator account and make dummy judge accounts
	if (User::s_usersByName.count("judge") == 0) {
		printf("Creating judge account...\n");
		new User("judge", "test", true, 0);
	}

	printf("Loading user sessions...\n");
	{
		ifstream file(".sessions");
		if (file.is_open()) {
			string sessID, name;
			u64 expireTimeMS;
			while (file >> sessID >> name >> expireTimeMS) {
				if (User::s_usersByName.count(name) == 0)
					continue;
				Session::s_sessionMap[sessID] = Session(User::s_usersByName[name], expireTimeMS);
			}
			file.close();
		}
	}
}

void saveJudgeData()
{
	printf("Saving user sessions...\n");
	{
		ofstream file;
		file.open(".sessions");
		std::map<std::string, Session>::iterator it = Session::s_sessionMap.begin();
		std::map<std::string, Session>::iterator end = Session::s_sessionMap.end();
		while (it != end) {
			file << it->first
				<< ' ' << it->second.user->username.c_str() 
				<< ' ' << it->second.expireTimeMS
				<< '\n';
			++it;
		}
		file.close();
	}
}

}
